import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { computeDamage } from "../../server/src/core/formulas/combat.js";
import {
  cumulativeExp,
  expToNext,
} from "../../server/src/core/formulas/level.js";
import {
  computeOfflineRewards,
  OFFLINE_EFFICIENCY,
} from "../../server/src/core/formulas/offline.js";
import {
  BOSS_REWARD_BONUS,
  computeKillExp,
  computeKillGold,
} from "../../server/src/core/formulas/reward.js";
import {
  computeMonsterStatMultiplier,
  computeRewardMultiplier,
  DEFAULT_STAGES_PER_CHAPTER,
} from "../../server/src/core/formulas/stage.js";
import {
  type ClassId,
  defaultPrimaryStats,
  deriveStats,
} from "../../server/src/core/formulas/stats.js";

export type SimulationOptions = {
  runs?: number;
  seed?: number;
  targetLevel?: number;
  includeBoss?: boolean;
};

export type SimulationSample = {
  run: number;
  classId: ClassId;
  activeShare: number;
  equipmentMultiplier: number;
  questMultiplier: number;
  offlineEfficiency: number;
  hoursToTarget: number;
  bossSeconds: number;
  expPerHourAtLevel50: number;
  goldPerHourAtLevel50: number;
  level50StageIndex: number;
  normalKillExpAtLevel50: number;
  normalKillGoldAtLevel50: number;
  bossKillExpAtLevel50: number;
  bossKillGoldAtLevel50: number;
};

export type DistributionSummary = {
  runs: number;
  targetLevel: number;
  p10Hours: number;
  medianHours: number;
  p90Hours: number;
  minHours: number;
  maxHours: number;
};

export type SimulationDistribution = {
  runs: number;
  seed: number;
  samples: SimulationSample[];
  summary: DistributionSummary;
};

export type BalanceEvaluation = {
  status: "inside-target" | "too-fast" | "too-slow";
  targetHours: {
    min: number;
    max: number;
  };
  acceptableHours: {
    min: number;
    max: number;
  };
  recommendations: string[];
};

export type BalanceReport = {
  markdown: string;
  json: {
    generatedAt: string;
    model: {
      targetLevel: number;
      bossIncluded: boolean;
      formulas: string[];
      rewardScaling: StageRewardComparison[];
    };
    distribution: SimulationDistribution;
    evaluation: BalanceEvaluation;
  };
};

export type StageRewardComparison = {
  stage: string;
  globalStageIndex: number;
  monsterHpMultiplier: number;
  rewardMultiplier: number;
  normalExp: number;
  normalGold: string;
  bossExp: number;
  bossGold: string;
};

const DEFAULT_RUNS = 1000;
const DEFAULT_SEED = 23;
const TARGET_LEVEL = 100;
const TARGET_MIN_HOURS = 5;
const TARGET_MAX_HOURS = 10;
const ACCEPTABLE_MIN_HOURS = 3;
const ACCEPTABLE_MAX_HOURS = 20;
const ACTIVE_EXP_TUNING = 5.5;
const ACTIVE_GOLD_TUNING = 7.4;
const SKILL_DPS_MULTIPLIER = 1.35;
const BOSS_HP_MULTIPLIER = 0.012;

export function simulateRebirthDistribution(
  options: SimulationOptions = {},
): SimulationDistribution {
  const runs = options.runs ?? DEFAULT_RUNS;
  const seed = options.seed ?? DEFAULT_SEED;
  const targetLevel = options.targetLevel ?? TARGET_LEVEL;
  const includeBoss = options.includeBoss ?? true;
  assertPositiveInteger(runs, "runs");
  assertPositiveInteger(seed, "seed");
  assertPositiveInteger(targetLevel, "targetLevel");

  const random = mulberry32(seed);
  const samples: SimulationSample[] = [];
  for (let run = 1; run <= runs; run += 1) {
    samples.push(simulateRun(run, random, targetLevel, includeBoss));
  }

  return {
    runs,
    seed,
    samples,
    summary: summarizeSamples(samples, targetLevel),
  };
}

export function evaluateBalance(
  summary: DistributionSummary,
): BalanceEvaluation {
  const status =
    summary.medianHours < TARGET_MIN_HOURS
      ? "too-fast"
      : summary.medianHours > TARGET_MAX_HOURS
        ? "too-slow"
        : "inside-target";

  return {
    status,
    targetHours: { min: TARGET_MIN_HOURS, max: TARGET_MAX_HOURS },
    acceptableHours: {
      min: ACCEPTABLE_MIN_HOURS,
      max: ACCEPTABLE_MAX_HOURS,
    },
    recommendations: buildRecommendations(status),
  };
}

export function buildBalanceReport(
  distribution: SimulationDistribution,
): BalanceReport {
  const evaluation = evaluateBalance(distribution.summary);
  const json = {
    generatedAt: new Date().toISOString(),
    model: {
      targetLevel: distribution.summary.targetLevel,
      bossIncluded: true,
      formulas: [
        "server/src/core/formulas/level.ts",
        "server/src/core/formulas/combat.ts",
        "server/src/core/formulas/stats.ts",
        "server/src/core/formulas/offline.ts",
        "server/src/core/formulas/reward.ts",
        "server/src/core/formulas/stage.ts",
      ],
      rewardScaling: buildStageRewardComparison(1),
    },
    distribution,
    evaluation,
  };

  return {
    json,
    markdown: renderMarkdown(json),
  };
}

export function writeBalanceReport(
  report: BalanceReport,
  outputDir = defaultOutputDir(),
): {
  markdownPath: string;
  jsonPath: string;
} {
  mkdirSync(outputDir, { recursive: true });
  const markdownPath = resolve(outputDir, "balance-sim-report.md");
  const jsonPath = resolve(outputDir, "balance-sim-report.json");
  writeFileSync(markdownPath, report.markdown, "utf8");
  writeFileSync(
    `${jsonPath}`,
    `${JSON.stringify(report.json, null, 2)}\n`,
    "utf8",
  );
  return { markdownPath, jsonPath };
}

function simulateRun(
  run: number,
  random: () => number,
  targetLevel: number,
  includeBoss: boolean,
): SimulationSample {
  const classId = randomClass(random);
  const activeShare = randomRange(random, 0.58, 0.9);
  const equipmentMultiplier = randomRange(random, 0.9, 1.22);
  const questMultiplier = randomRange(random, 0.94, 1.12);
  const offlineEfficiency = randomRange(random, 0.68, 0.8);
  let seconds = 0;

  for (let level = 1; level < targetLevel; level += 1) {
    const requiredExp = expToNext(level);
    const stageIndex = representativeStageIndex(level, targetLevel);
    const expPerSecond =
      blendedExpPerSecond(level, stageIndex, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * rebirthRamp(level);
    seconds += requiredExp / expPerSecond;
  }

  const bossSeconds = includeBoss
    ? bossClearSeconds(
        targetLevel,
        representativeStageIndex(targetLevel, targetLevel),
        classId,
        equipmentMultiplier,
      )
    : 0;
  seconds += bossSeconds;
  const level50StageIndex = representativeStageIndex(50, targetLevel);

  return {
    run,
    classId,
    activeShare: round(activeShare, 3),
    equipmentMultiplier: round(equipmentMultiplier, 3),
    questMultiplier: round(questMultiplier, 3),
    offlineEfficiency: round(offlineEfficiency, 3),
    hoursToTarget: round(seconds / 3_600, 3),
    bossSeconds: round(bossSeconds, 1),
    expPerHourAtLevel50: Math.round(
      blendedExpPerSecond(50, level50StageIndex, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * 3_600,
    ),
    goldPerHourAtLevel50: Math.round(
      blendedGoldPerSecond(50, level50StageIndex, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * 3_600,
    ),
    level50StageIndex,
    normalKillExpAtLevel50: computeKillExp(
      baseKillExp(50),
      level50StageIndex,
      false,
    ),
    normalKillGoldAtLevel50: computeKillGold(
      baseKillGold(50),
      level50StageIndex,
      false,
    ),
    bossKillExpAtLevel50: computeKillExp(
      baseKillExp(50),
      level50StageIndex,
      true,
    ),
    bossKillGoldAtLevel50: computeKillGold(
      baseKillGold(50),
      level50StageIndex,
      true,
    ),
  };
}

function blendedExpPerSecond(
  level: number,
  stageIndex: number,
  classId: ClassId,
  modifiers: {
    activeShare: number;
    equipmentMultiplier: number;
    questMultiplier: number;
    offlineEfficiency: number;
  },
): number {
  const active = activeExpPerSecond(
    level,
    stageIndex,
    classId,
    modifiers.equipmentMultiplier,
  );
  const offline =
    (computeOfflineRewards({
      level,
      lastSeenUnixSec: 0,
      nowUnixSec: 3_600,
    }).exp /
      3_600) *
    (modifiers.offlineEfficiency / OFFLINE_EFFICIENCY);

  return (
    (active * modifiers.activeShare + offline * (1 - modifiers.activeShare)) *
    modifiers.questMultiplier
  );
}

function blendedGoldPerSecond(
  level: number,
  stageIndex: number,
  classId: ClassId,
  modifiers: {
    activeShare: number;
    equipmentMultiplier: number;
    questMultiplier: number;
    offlineEfficiency: number;
  },
): number {
  const active = activeGoldPerSecond(
    level,
    stageIndex,
    classId,
    modifiers.equipmentMultiplier,
  );
  const offline =
    (computeOfflineRewards({
      level,
      lastSeenUnixSec: 0,
      nowUnixSec: 3_600,
    }).gold /
      3_600) *
    (modifiers.offlineEfficiency / OFFLINE_EFFICIENCY);

  return active * modifiers.activeShare + offline * (1 - modifiers.activeShare);
}

function activeExpPerSecond(
  level: number,
  stageIndex: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const killsPerSecond = estimatedKillsPerSecond(
    level,
    stageIndex,
    classId,
    equipmentMultiplier,
  );
  return (
    computeKillExp(baseKillExp(level), stageIndex, false) *
    killsPerSecond *
    ACTIVE_EXP_TUNING
  );
}

function activeGoldPerSecond(
  level: number,
  stageIndex: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const killsPerSecond = estimatedKillsPerSecond(
    level,
    stageIndex,
    classId,
    equipmentMultiplier,
  );
  return (
    computeKillGold(baseKillGold(level), stageIndex, false) *
    killsPerSecond *
    ACTIVE_GOLD_TUNING
  );
}

function estimatedKillsPerSecond(
  level: number,
  stageIndex: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const dps = playerDps(level, classId, equipmentMultiplier);
  return Math.max(0.05, dps / monsterHp(level, stageIndex));
}

function playerDps(
  level: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const stats = deriveStats(defaultPrimaryStats(classId, level), level, {
    physAtk: level * 16 * equipmentMultiplier,
    magicAtk: level * 16 * equipmentMultiplier,
  });
  const attack =
    classId === 2 || classId === 5 ? stats.magicAtk : stats.physAtk;
  const baseHit = computeDamage(attack, monsterDef(level));
  const critMultiplier = 1 + stats.critRate * (stats.critDmg - 1);
  return baseHit * stats.atkSpeed * critMultiplier * SKILL_DPS_MULTIPLIER;
}

function bossClearSeconds(
  level: number,
  stageIndex: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const bossHp = monsterHp(level, stageIndex) * BOSS_HP_MULTIPLIER;
  return bossHp / playerDps(level, classId, equipmentMultiplier);
}

function monsterHp(level: number, stageIndex: number): number {
  return Math.round(
    level * 80 * 1.08 ** level * computeMonsterStatMultiplier(stageIndex),
  );
}

function monsterDef(level: number): number {
  return Math.round(level * 5 * 1.035 ** level);
}

function rebirthRamp(level: number): number {
  const progress =
    cumulativeExp(level) / Math.max(1, cumulativeExp(TARGET_LEVEL));
  return 1 + progress * 0.18;
}

function representativeStageIndex(level: number, targetLevel: number): number {
  const normalizedProgress = Math.max(0, Math.min(level - 1, targetLevel - 1));
  return Math.min(
    DEFAULT_STAGES_PER_CHAPTER - 1,
    Math.floor((normalizedProgress / targetLevel) * DEFAULT_STAGES_PER_CHAPTER),
  );
}

function baseKillExp(level: number): number {
  return level * 12;
}

function baseKillGold(level: number): number {
  return level * 8;
}

function buildStageRewardComparison(chapter: number): StageRewardComparison[] {
  const baseGoldMin = 10;
  const baseGoldMax = 15;
  return Array.from(
    { length: DEFAULT_STAGES_PER_CHAPTER },
    (_, stageOffset) => {
      const globalStageIndex =
        (chapter - 1) * DEFAULT_STAGES_PER_CHAPTER + stageOffset;
      const baseExp = baseKillExp(1);
      const normalGoldMin = computeKillGold(
        baseGoldMin,
        globalStageIndex,
        false,
      );
      const normalGoldMax = computeKillGold(
        baseGoldMax,
        globalStageIndex,
        false,
      );
      const bossGoldMin = computeKillGold(baseGoldMin, globalStageIndex, true);
      const bossGoldMax = computeKillGold(baseGoldMax, globalStageIndex, true);
      return {
        stage: `${chapter}-${stageOffset + 1}`,
        globalStageIndex,
        monsterHpMultiplier: round(
          computeMonsterStatMultiplier(globalStageIndex),
          3,
        ),
        rewardMultiplier: round(computeRewardMultiplier(globalStageIndex), 3),
        normalExp: computeKillExp(baseExp, globalStageIndex, false),
        normalGold: `${normalGoldMin}-${normalGoldMax}`,
        bossExp: computeKillExp(baseExp, globalStageIndex, true),
        bossGold: `${bossGoldMin}-${bossGoldMax}`,
      };
    },
  );
}

function summarizeSamples(
  samples: SimulationSample[],
  targetLevel: number,
): DistributionSummary {
  const hours = samples
    .map((sample) => sample.hoursToTarget)
    .sort((left, right) => left - right);

  return {
    runs: samples.length,
    targetLevel,
    p10Hours: percentile(hours, 0.1),
    medianHours: percentile(hours, 0.5),
    p90Hours: percentile(hours, 0.9),
    minHours: round(hours[0] ?? 0, 3),
    maxHours: round(hours[hours.length - 1] ?? 0, 3),
  };
}

function percentile(sortedValues: number[], rank: number): number {
  if (sortedValues.length === 0) {
    return 0;
  }
  const index = Math.min(
    sortedValues.length - 1,
    Math.max(0, Math.floor((sortedValues.length - 1) * rank)),
  );
  return round(sortedValues[index], 3);
}

function buildRecommendations(status: BalanceEvaluation["status"]): string[] {
  if (status === "too-fast") {
    return [
      "EXP curve: consider exponent 1.85-2.0 before combat tuning.",
      "gold per hour: reduce active gold tuning by 5-10% if spend is light.",
      "offline efficiency: test 0.70-0.75 if p10 drops under 3h.",
    ];
  }
  if (status === "too-slow") {
    return [
      "EXP curve: keep exponent 1.7 until median returns below 10h.",
      "gold per hour: raise early tuning 5-10% before lowering costs.",
      "offline efficiency: allow 0.75-0.80 only if active stays faster.",
    ];
  }
  return [
    "EXP curve: keep current level.ts curve; do not raise exponent yet.",
    "gold per hour: keep active tuning stable; watch enhancement spend.",
    "offline efficiency: 70-80% keeps idle below active progress.",
  ];
}

function renderMarkdown(report: BalanceReport["json"]): string {
  const { summary } = report.distribution;
  const { evaluation } = report;
  const lines = [
    "# Balance Simulator V1",
    "",
    "## Distribution",
    "",
    `- Runs: ${summary.runs}`,
    `- Target: Lv${summary.targetLevel} + boss clear`,
    `- p10: ${summary.p10Hours}h`,
    `- median: ${summary.medianHours}h`,
    `- p90: ${summary.p90Hours}h`,
    `- min/max: ${summary.minHours}h / ${summary.maxHours}h`,
    `- status: ${evaluation.status} (target ${evaluation.targetHours.min}-${evaluation.targetHours.max}h, acceptable ${evaluation.acceptableHours.min}-${evaluation.acceptableHours.max}h)`,
    "",
    "## Sensitivity",
    "",
    ...evaluation.recommendations.map(
      (recommendation) => `- ${recommendation}`,
    ),
    "",
    "## Reward Scaling",
    "",
    `- Boss bonus: ${BOSS_REWARD_BONUS}x`,
    "- Normal kill rewards use `computeKillExp` / `computeKillGold`.",
    "- Source: `server/src/core/formulas/reward.ts`.",
    "- Monster HP and reward multipliers both reuse the stage index ramp.",
    "- Result: 1-1 through 1-5 keep reward-per-HP pressure stable before boss bonuses.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Stage | idx | HP x | Reward x | Normal EXP | Normal Gold | Boss EXP | Boss Gold |",
    "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.rewardScaling.map(
      (row) =>
        `| ${row.stage} | ${row.globalStageIndex} | ${row.monsterHpMultiplier} | ${row.rewardMultiplier} | ${row.normalExp} | ${row.normalGold} | ${row.bossExp} | ${row.bossGold} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Formula Sources",
    "",
    ...report.model.formulas.map((source) => `- ${source}`),
  ];
  return `${lines.join("\n")}\n`;
}

function randomClass(random: () => number): ClassId {
  const classes: ClassId[] = [1, 2, 3];
  return classes[Math.floor(random() * classes.length)] ?? 1;
}

function randomRange(random: () => number, min: number, max: number): number {
  return min + (max - min) * random();
}

function round(value: number, digits: number): number {
  const factor = 10 ** digits;
  return Math.round(value * factor) / factor;
}

function mulberry32(seed: number): () => number {
  let state = seed;
  return () => {
    state += 0x6d2b79f5;
    let value = state;
    value = Math.imul(value ^ (value >>> 15), value | 1);
    value ^= value + Math.imul(value ^ (value >>> 7), value | 61);
    return ((value ^ (value >>> 14)) >>> 0) / 4294967296;
  };
}

function assertPositiveInteger(value: number, name: string) {
  if (!Number.isInteger(value) || value < 1) {
    throw new Error(`${name} must be a positive integer`);
  }
}

function defaultOutputDir(): string {
  return resolve(dirname(fileURLToPath(import.meta.url)), "reports");
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const distribution = simulateRebirthDistribution();
  const report = buildBalanceReport(distribution);
  const paths = writeBalanceReport(report);
  console.log(report.markdown);
  console.log(`Wrote ${paths.markdownPath}`);
  console.log(`Wrote ${paths.jsonPath}`);
}
