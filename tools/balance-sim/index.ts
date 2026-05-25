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
    };
    distribution: SimulationDistribution;
    evaluation: BalanceEvaluation;
  };
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
      ],
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
    const expPerSecond =
      blendedExpPerSecond(level, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * rebirthRamp(level);
    seconds += requiredExp / expPerSecond;
  }

  const bossSeconds = includeBoss
    ? bossClearSeconds(targetLevel, classId, equipmentMultiplier)
    : 0;
  seconds += bossSeconds;

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
      blendedExpPerSecond(50, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * 3_600,
    ),
    goldPerHourAtLevel50: Math.round(
      blendedGoldPerSecond(50, classId, {
        activeShare,
        equipmentMultiplier,
        questMultiplier,
        offlineEfficiency,
      }) * 3_600,
    ),
  };
}

function blendedExpPerSecond(
  level: number,
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
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const killsPerSecond = estimatedKillsPerSecond(
    level,
    classId,
    equipmentMultiplier,
  );
  return level * 12 * killsPerSecond * ACTIVE_EXP_TUNING;
}

function activeGoldPerSecond(
  level: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const killsPerSecond = estimatedKillsPerSecond(
    level,
    classId,
    equipmentMultiplier,
  );
  return level * 8 * killsPerSecond * ACTIVE_GOLD_TUNING;
}

function estimatedKillsPerSecond(
  level: number,
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const dps = playerDps(level, classId, equipmentMultiplier);
  return Math.max(0.05, dps / monsterHp(level));
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
  classId: ClassId,
  equipmentMultiplier: number,
): number {
  const bossHp = monsterHp(level) * BOSS_HP_MULTIPLIER;
  return bossHp / playerDps(level, classId, equipmentMultiplier);
}

function monsterHp(level: number): number {
  return Math.round(level * 80 * 1.08 ** level);
}

function monsterDef(level: number): number {
  return Math.round(level * 5 * 1.035 ** level);
}

function rebirthRamp(level: number): number {
  const progress =
    cumulativeExp(level) / Math.max(1, cumulativeExp(TARGET_LEVEL));
  return 1 + progress * 0.18;
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
      "EXP curve: raise expToNext exponent toward 1.85-2.0 before touching combat coefficients.",
      "gold per hour: reduce active gold tuning by 5-10% if enhancement pressure is too light.",
      "offline efficiency: keep below active play and test 0.70-0.75 if p10 remains under 3h.",
    ];
  }
  if (status === "too-slow") {
    return [
      "EXP curve: keep exponent at 1.7 and avoid moving toward 1.85-2.0 until median returns below 10h.",
      "gold per hour: increase early gold tuning by 5-10% before reducing enhancement costs.",
      "offline efficiency: allow 0.75-0.80 only if active play remains clearly faster.",
    ];
  }
  return [
    "EXP curve: keep the current level.ts curve; do not apply the 1.85-2.0 exponent increase yet.",
    "gold per hour: keep active gold tuning stable and watch enhancement spend in the next slice.",
    "offline efficiency: current 70-80% sampling keeps idle rewards below active progress.",
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
    "## Formula Sources",
    "",
    ...report.model.formulas.map((source) => `- ${source}`),
    "",
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
