import { mkdirSync, writeFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { getSkillDefinitionsForClass } from "../../server/src/core/data/skills.js";
import {
  ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS,
  ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS,
  ACHIEVEMENT_POINTS_MULTIPLIER,
  getAchievementStatMultiplier,
} from "../../server/src/core/formulas/achievement.js";
import { getClassMasteryMultipliers } from "../../server/src/core/formulas/classRune.js";
import {
  computeClassDamage,
  computeDamage,
  computeElementMultiplier,
  type SkillElement,
} from "../../server/src/core/formulas/combat.js";
import { computeCombatPower } from "../../server/src/core/formulas/combatPower.js";
import {
  getAffixCount,
  getRarityDropChances,
  getRarityStatMultiplier,
  type ItemRarity,
} from "../../server/src/core/formulas/drop.js";
import {
  type EnhanceItemRarity,
  getEnhanceCost,
  getEnhanceSuccessRate,
  getRarityCostMultiplier,
  MAX_ENHANCE_LEVEL,
} from "../../server/src/core/formulas/enhance.js";
import {
  cumulativeExp,
  expToNext,
} from "../../server/src/core/formulas/level.js";
import {
  computeOfflineRewards,
  OFFLINE_EFFICIENCY,
} from "../../server/src/core/formulas/offline.js";
import {
  getEffectiveBonusPercent,
  getFeedCost,
  MAX_PET_LEVEL,
} from "../../server/src/core/formulas/petLevel.js";
import {
  BOSS_REWARD_BONUS,
  computeKillExp,
  computeKillGold,
  ELITE_REWARD_BONUS,
} from "../../server/src/core/formulas/reward.js";
import {
  getCoreRuneMultiplier,
  getUtilCap,
  getUtilRuneValue,
  RUNE_SLOT_COUNT,
} from "../../server/src/core/formulas/rune.js";
import {
  CORE_CATEGORY_BONUS,
  computeRuneCodexBonus,
  getRowCompletionBonus,
  PER_CELL_CORE_BONUS,
  RUNE_CODEX_TOTAL_CELLS,
  UTIL_CATEGORY_CAP_EXTENSION,
} from "../../server/src/core/formulas/runeCodex.js";
import {
  computeRuneSetBonus,
  getSetTierBonus,
  RUNE_SET_BASTION,
  RUNE_SET_FORTUNE,
  RUNE_SET_OFFENSE,
  RUNE_SET_TIER1_COUNT,
  RUNE_SET_TIER2_COUNT,
  RUNE_SET_TIER3_COUNT,
  RUNE_SET_VITALITY,
} from "../../server/src/core/formulas/runeSet.js";
import {
  computeGlobalStageIndex,
  computeMonsterStatMultiplier,
  computeRewardMultiplier,
  DEFAULT_STAGES_PER_CHAPTER,
  getStageWeakElement,
  isBossStage,
  isEliteStage,
} from "../../server/src/core/formulas/stage.js";
import {
  type ClassId,
  type DerivedStats,
  defaultPrimaryStats,
  deriveStats,
} from "../../server/src/core/formulas/stats.js";
import { getTowerMilestoneMultiplier } from "../../server/src/core/formulas/towerMilestone.js";
import { getTranscendStatMultiplier } from "../../server/src/core/formulas/transcend.js";

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
      darkElementPressure: ElementPressure;
      dropRarityPressure: DropRarityPressure;
      enhancementPressure: EnhancementPressure;
      petFeedPressure: PetFeedPressure;
      achievementPressure: AchievementPressure;
      runePressure: RunePressure;
      runeCodexPressure: RuneCodexPressure;
      classBalance: ClassBalanceSnapshot;
    };
    distribution: SimulationDistribution;
    evaluation: BalanceEvaluation;
  };
};

export type ClassRole = "dps" | "tank" | "healer";

export type ClassBalanceRow = {
  classId: ClassId;
  className: string;
  role: ClassRole;
  level: number;
  hp: number;
  physAtk: number;
  magicAtk: number;
  physDef: number;
  magicDef: number;
  critRate: number;
  atkSpeed: number;
  effectiveAttack: number;
  skillDpsRate: number;
  effectiveDps: number;
  dpsDeltaFromMedian: number;
  combatPower: number;
};

export type ClassBalanceSnapshot = {
  levels: number[];
  rowsByLevel: Record<number, ClassBalanceRow[]>;
};

export type StageRewardComparison = {
  stage: string;
  globalStageIndex: number;
  encounterType: "normal" | "elite" | "boss";
  weakElement: SkillElement;
  monsterHpMultiplier: number;
  rewardMultiplier: number;
  normalExp: number;
  normalGold: string;
  eliteExp: number;
  eliteGold: string;
  bossExp: number;
  bossGold: string;
};

export type ElementPressureRow = {
  skillElement: SkillElement;
  targetWeakElement: SkillElement;
  multiplier: number;
  note: string;
};

export type ElementPressure = {
  darkWeakStageCount: number;
  totalStageCount: number;
  rows: ElementPressureRow[];
};

export type DropRarityPressureRow = {
  rarity: Exclude<ItemRarity, "None">;
  statMultiplier: number;
  chanceAtLevel1Percent: number;
  chanceAtLevel100Percent: number;
  affixCount: string;
};

export type DropRarityPressure = {
  levels: number[];
  totalChanceAtLevel1: number;
  totalChanceAtLevel100: number;
  rows: DropRarityPressureRow[];
};

export type EnhancementPressureRow = {
  currentLevel: number;
  nextLevel: number;
  cost: number;
  successRate: number;
  expectedAttempts: number;
  expectedGoldCost: number;
  cumulativeExpectedGoldCost: number;
};

export type EnhancementPressure = {
  maxLevel: number;
  goldCostFloorToMax: number;
  expectedGoldCostToMax: number;
  medianGoldPerHourAtLevel50: number;
  expectedHoursAtMedianGoldPerHour: number;
  rarityScenarios: EnhancementRarityScenario[];
  legendaryEightSlotExpectedGoldCost: number;
  legendaryEightSlotExpectedHoursAtMedianGoldPerHour: number;
  mythicEightSlotExpectedGoldCost: number;
  mythicEightSlotExpectedHoursAtMedianGoldPerHour: number;
  rows: EnhancementPressureRow[];
};

export type PetFeedPressureRow = {
  currentLevel: number;
  nextLevel: number;
  feedCost: number;
  cumulativeFeedCost: number;
  dogGoldBonusPercentAfterFeed: number;
  birdDropBonusPercentAfterFeed: number;
};

export type PetFeedPressure = {
  maxLevel: number;
  totalFeedCostToMax: number;
  medianGoldPerHourAtLevel50: number;
  dogBaseGoldBonusPercent: number;
  dogMaxGoldBonusPercent: number;
  dogGoldBonusDeltaPercentAtMax: number;
  birdBaseDropBonusPercent: number;
  birdMaxDropBonusPercent: number;
  birdDropBonusDeltaPercentAtMax: number;
  incrementalGoldPerHourAtMedian: number;
  paybackHoursAtMedianGoldPerHour: number;
  rows: PetFeedPressureRow[];
};

export type AchievementPressureRow = {
  totalPoints: number;
  legacyMultiplier: number;
  softCappedMultiplier: number;
  compositeWithTranscendAndTower: number;
};

export type AchievementPressure = {
  softCapStartPoints: number;
  softCapBonusPoints: number;
  referenceTranscendCount: number;
  referenceTowerFloor: number;
  referenceTranscendMultiplier: number;
  referenceTowerMultiplier: number;
  rows: AchievementPressureRow[];
};

export type EnhancementRarityScenario = {
  rarity: EnhanceItemRarity;
  multiplier: number;
  goldCostFloorToMax: number;
  expectedGoldCostToMax: number;
  expectedHoursAtMedianGoldPerHour: number;
};

export type RuneRarityName =
  | "Common"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent"
  | "Mythic";

export type RuneTypeName =
  | "CritDamage"
  | "GoldFind"
  | "ExpBoost"
  | "OfflineEff";

export type RuneCorePressureRow = {
  rarity: RuneRarityName;
  enhanceLevel: number;
  singleRuneBonusPercent: number;
  sixSlotMultiplier: number;
};

export type RuneCombatPressureRow = {
  runeSet: string;
  className: string;
  level: number;
  baseCombatPower: number;
  runeCombatPower: number;
  cpMultiplier: number;
  baseDps: number;
  runeDps: number;
  dpsMultiplier: number;
};

export type ClassMasteryPressureRow = {
  classId: ClassId;
  className: string;
  role: ClassRole;
  level: number;
  rarity: RuneRarityName;
  enhanceLevel: number;
  masteryStats: string[];
  baseCombatPower: number;
  classRuneCombatPower: number;
  cpMultiplier: number;
  baseDps: number;
  classRuneDps: number;
  dpsMultiplier: number;
};

export type RuneSetPressureRow = {
  runeSet: string;
  count: number;
  tierBonusPercent: number;
  bonusLanes: string[];
  className: string;
  level: number;
  baseCombatPower: number;
  setCombatPower: number;
  cpMultiplier: number;
  baseDps: number;
  setDps: number;
  dpsMultiplier: number;
  firstRebirthInjected: boolean;
};

export type RuneUtilPressureRow = {
  runeType: RuneTypeName;
  rarity: RuneRarityName;
  enhanceLevel: number;
  singleRuneValuePercent: number;
  sixSlotUncappedTotalPercent: number;
  effectiveEconomicMultiplier: number;
};

export type RunePressure = {
  slotCount: number;
  coreRows: RuneCorePressureRow[];
  combatRows: RuneCombatPressureRow[];
  classMasteryRows: ClassMasteryPressureRow[];
  setRows: RuneSetPressureRow[];
  utilRows: RuneUtilPressureRow[];
};

export type RuneCodexPressure = {
  totalCells: number;
  perCellCoreBonusPercent: number;
  allCellsCoreBonusPercent: number;
  allRowsCoreBonusPercent: number;
  coreCategoryBonusPercent: number;
  utilCategoryCapExtensionPercent: number;
  fullCodexCoreStatAddPercent: number;
  baseMedianRebirthHours: number;
  projectedFullCodexMedianHours: number;
  projectedMedianDeltaPercent: number;
  injectedIntoSampledRun: boolean;
};

const DEFAULT_RUNS = 1000;
const DEFAULT_SEED = 23;
const TARGET_LEVEL = 100;
const TOTAL_CHAPTERS = 3;
const TARGET_MIN_HOURS = 5;
const TARGET_MAX_HOURS = 10;
const ACCEPTABLE_MIN_HOURS = 3;
const ACCEPTABLE_MAX_HOURS = 20;
const ACTIVE_EXP_TUNING = 5.5;
const ACTIVE_GOLD_TUNING = 7.4;
const SKILL_DPS_MULTIPLIER = 1.35;
const FIRST_REBIRTH_REVIEW_STAGE_COUNT = 5;
const CLASS_BALANCE_REVIEW_DEF_PER_LEVEL = 5;
const BOSS_HP_MULTIPLIER = 0.012;
const ENHANCEMENT_RARITY_SCENARIOS: EnhanceItemRarity[] = [
  "Common",
  "Rare",
  "Epic",
  "Unique",
  "Legendary",
  "Transcendent",
  "Mythic",
];
const ITEM_DROP_RARITIES: Exclude<ItemRarity, "None">[] = [
  "Common",
  "Rare",
  "Epic",
  "Unique",
  "Legendary",
  "Transcendent",
  "Mythic",
];
const EQUIPMENT_SLOT_COUNT = 8;
const DOG_GOLD_BONUS_PERCENT = 20;
const BIRD_DROP_BONUS_PERCENT = 15;
const RUNE_CORE_REVIEW_LEVELS = [0, 10, 50, 100];
const RUNE_CORE_REVIEW_RARITIES = [1, 3, 5, 7] as const;
const RUNE_UTIL_REVIEW_TYPES = [6, 7, 8, 9] as const;
const MYTHIC_RUNE_RARITY = 7;
const CLASS_MASTERY_REVIEW_LEVEL = 100;
const CLASS_MASTERY_REVIEW_ENHANCE_LEVEL = 50;
const REGULAR_RUNE_SLOT_COUNT = RUNE_SLOT_COUNT - 1;
const RUNE_SET_REVIEW_CLASS: ClassId = 1;
const RUNE_SET_REVIEW_LEVEL = 100;
const RUNE_SET_REVIEW_COUNTS = [
  RUNE_SET_TIER1_COUNT,
  RUNE_SET_TIER2_COUNT,
  RUNE_SET_TIER3_COUNT,
];
const RUNE_SET_REVIEW_SETS = [
  RUNE_SET_OFFENSE,
  RUNE_SET_BASTION,
  RUNE_SET_VITALITY,
  RUNE_SET_FORTUNE,
] as const;

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
        "server/src/core/formulas/drop.ts",
        "server/src/core/formulas/enhance.ts",
        "server/src/core/formulas/petLevel.ts",
        "server/src/core/formulas/achievement.ts",
        "server/src/core/formulas/rune.ts",
        "server/src/core/formulas/classRune.ts",
        "server/src/core/formulas/runeSet.ts",
        "server/src/core/formulas/runeCodex.ts",
      ],
      rewardScaling: buildStageRewardComparison(),
      darkElementPressure: buildDarkElementPressure(),
      dropRarityPressure: buildDropRarityPressure(),
      enhancementPressure: buildEnhancementPressure(distribution.samples),
      petFeedPressure: buildPetFeedPressure(distribution.samples),
      achievementPressure: buildAchievementPressure(),
      runePressure: buildRunePressure(),
      runeCodexPressure: buildRuneCodexPressure(
        distribution.summary.medianHours,
      ),
      classBalance: buildClassBalanceSnapshot([50, 100]),
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
  const attack = isMagicClass(classId) ? stats.magicAtk : stats.physAtk;
  const baseHit = computeDamage(attack, monsterDef(level));
  const critMultiplier = 1 + stats.critRate * (stats.critDmg - 1);
  return baseHit * stats.atkSpeed * critMultiplier * SKILL_DPS_MULTIPLIER;
}

export function buildClassBalanceSnapshot(
  levels: number[] = [50, 100],
): ClassBalanceSnapshot {
  const rowsByLevel: Record<number, ClassBalanceRow[]> = {};
  for (const level of levels) {
    const rows = ALL_CLASSES.map((classId) =>
      buildClassBalanceRow(classId, level),
    );
    const dpsValues = rows
      .filter((row) => row.role === "dps")
      .map((row) => row.effectiveDps)
      .sort((left, right) => left - right);
    const medianDps = dpsValues[Math.floor(dpsValues.length / 2)] ?? 0;

    rowsByLevel[level] = rows.map((row) => ({
      ...row,
      dpsDeltaFromMedian:
        row.role === "dps" && medianDps > 0
          ? round((row.effectiveDps - medianDps) / medianDps, 3)
          : 0,
    }));
  }

  return { levels, rowsByLevel };
}

function buildClassBalanceRow(
  classId: ClassId,
  level: number,
): ClassBalanceRow {
  const stats = deriveStats(defaultPrimaryStats(classId, level), level, {
    physAtk: level * 16,
    magicAtk: level * 16,
  });
  const effectiveAttack = isMagicClass(classId)
    ? stats.magicAtk
    : stats.physAtk;
  const damageSkills = getSkillDefinitionsForClass(classId).filter(
    (skill) => skill.cooldown > 0 && skill.damageCoeff > 0,
  );
  const skillDpsRate = damageSkills.reduce(
    (sum, skill) => sum + skill.damageCoeff / skill.cooldown,
    0,
  );
  const effectiveAtkSpeed = 1 + (stats.atkSpeed - 1) * 0.6;
  const critMultiplier = 1 + stats.critRate * (stats.critDmg - 1) * 0.6;
  const reviewDef = level * CLASS_BALANCE_REVIEW_DEF_PER_LEVEL;
  const baseHit = computeClassDamage(stats, classId, reviewDef, reviewDef);
  const skillDps = damageSkills.reduce(
    (sum, skill) =>
      sum +
      computeClassDamage(
        {
          physAtk: stats.physAtk * skill.damageCoeff,
          magicAtk: stats.magicAtk * skill.damageCoeff,
        },
        classId,
        reviewDef,
        reviewDef,
      ) /
        skill.cooldown,
    0,
  );

  return {
    classId,
    className: CLASS_NAMES[classId],
    role: CLASS_ROLES[classId],
    level,
    hp: stats.hp,
    physAtk: stats.physAtk,
    magicAtk: stats.magicAtk,
    physDef: stats.physDef,
    magicDef: stats.magicDef,
    critRate: stats.critRate,
    atkSpeed: stats.atkSpeed,
    effectiveAttack,
    skillDpsRate: round(skillDpsRate, 3),
    effectiveDps: Math.round(
      (baseHit * effectiveAtkSpeed + skillDps) * critMultiplier,
    ),
    dpsDeltaFromMedian: 0,
    combatPower: computeCombatPower(stats),
  };
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
  return (
    1 +
    Math.min(
      FIRST_REBIRTH_REVIEW_STAGE_COUNT - 1,
      Math.floor(
        (normalizedProgress / targetLevel) * FIRST_REBIRTH_REVIEW_STAGE_COUNT,
      ),
    )
  );
}

function baseKillExp(level: number): number {
  return level * 12;
}

function baseKillGold(level: number): number {
  return level * 8;
}

function buildStageRewardComparison(): StageRewardComparison[] {
  const baseGoldMin = 10;
  const baseGoldMax = 15;
  return Array.from({ length: TOTAL_CHAPTERS }, (_, chapterOffset) =>
    Array.from(
      { length: DEFAULT_STAGES_PER_CHAPTER },
      (_, stageOffset): StageRewardComparison => {
        const chapter = chapterOffset + 1;
        const stage = stageOffset + 1;
        const globalStageIndex = computeGlobalStageIndex(
          chapter,
          stage,
          DEFAULT_STAGES_PER_CHAPTER,
        );
        const encounterType = isBossStage(
          chapter,
          stage,
          DEFAULT_STAGES_PER_CHAPTER,
        )
          ? "boss"
          : isEliteStage(stage)
            ? "elite"
            : "normal";
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
        const eliteGoldMin = computeKillGold(
          baseGoldMin,
          globalStageIndex,
          false,
          true,
        );
        const eliteGoldMax = computeKillGold(
          baseGoldMax,
          globalStageIndex,
          false,
          true,
        );
        const bossGoldMin = computeKillGold(
          baseGoldMin,
          globalStageIndex,
          true,
        );
        const bossGoldMax = computeKillGold(
          baseGoldMax,
          globalStageIndex,
          true,
        );
        return {
          stage: `${chapter}-${stage}`,
          globalStageIndex,
          encounterType,
          weakElement: getStageWeakElement(globalStageIndex),
          monsterHpMultiplier: round(
            computeMonsterStatMultiplier(globalStageIndex),
            3,
          ),
          rewardMultiplier: round(computeRewardMultiplier(globalStageIndex), 3),
          normalExp: computeKillExp(baseExp, globalStageIndex, false),
          normalGold: `${normalGoldMin}-${normalGoldMax}`,
          eliteExp: computeKillExp(baseExp, globalStageIndex, false, true),
          eliteGold: `${eliteGoldMin}-${eliteGoldMax}`,
          bossExp: computeKillExp(baseExp, globalStageIndex, true),
          bossGold: `${bossGoldMin}-${bossGoldMax}`,
        };
      },
    ),
  ).flat();
}

function buildDarkElementPressure(): ElementPressure {
  const totalStageCount = TOTAL_CHAPTERS * DEFAULT_STAGES_PER_CHAPTER;
  const darkWeakStageCount = Array.from(
    { length: totalStageCount },
    (_, index) => getStageWeakElement(index + 1),
  ).filter((element) => element === "Dark").length;

  return {
    darkWeakStageCount,
    totalStageCount,
    rows: [
      {
        skillElement: "Holy",
        targetWeakElement: "Dark",
        multiplier: computeElementMultiplier("Holy", "Dark"),
        note: "Holy counter into Dark-heavy stages",
      },
      {
        skillElement: "Dark",
        targetWeakElement: "Holy",
        multiplier: computeElementMultiplier("Dark", "Holy"),
        note: "Dark counter into Holy weakness stages",
      },
      {
        skillElement: "Dark",
        targetWeakElement: "Dark",
        multiplier: computeElementMultiplier("Dark", "Dark"),
        note: "Direct Dark weakness match",
      },
      {
        skillElement: "Dark",
        targetWeakElement: "Fire",
        multiplier: computeElementMultiplier("Dark", "Fire"),
        note: "Neutral non-Holy/Dark matchup",
      },
    ],
  };
}

function buildDropRarityPressure(): DropRarityPressure {
  const level1Chances = getRarityDropChances(1);
  const level100Chances = getRarityDropChances(100);

  return {
    levels: [1, 100],
    totalChanceAtLevel1: round(
      Object.values(level1Chances).reduce((sum, chance) => sum + chance, 0),
      6,
    ),
    totalChanceAtLevel100: round(
      Object.values(level100Chances).reduce((sum, chance) => sum + chance, 0),
      6,
    ),
    rows: ITEM_DROP_RARITIES.map((rarity) => ({
      rarity,
      statMultiplier: round(getRarityStatMultiplier(rarity), 3),
      chanceAtLevel1Percent: round(level1Chances[rarity] * 100, 3),
      chanceAtLevel100Percent: round(level100Chances[rarity] * 100, 3),
      affixCount: describeAffixCount(rarity),
    })),
  };
}

function describeAffixCount(rarity: ItemRarity): string {
  const lowRoll = getAffixCount(rarity, () => 0.49);
  const highRoll = getAffixCount(rarity, () => 0.5);
  return lowRoll === highRoll ? String(lowRoll) : `${lowRoll}-${highRoll}`;
}

function buildEnhancementPressure(
  samples: SimulationSample[],
): EnhancementPressure {
  let cumulativeExpectedGoldCost = 0;
  const rows = Array.from({ length: MAX_ENHANCE_LEVEL }, (_, currentLevel) => {
    const cost = getEnhanceCost(currentLevel);
    const successRate = getEnhanceSuccessRate(currentLevel);
    const expectedAttempts = successRate > 0 ? 1 / successRate : 0;
    const expectedGoldCost = cost * expectedAttempts;
    cumulativeExpectedGoldCost += expectedGoldCost;
    return {
      currentLevel,
      nextLevel: currentLevel + 1,
      cost,
      successRate,
      expectedAttempts: round(expectedAttempts, 3),
      expectedGoldCost: round(expectedGoldCost, 2),
      cumulativeExpectedGoldCost: round(cumulativeExpectedGoldCost, 2),
    };
  });
  const goldCostFloorToMax = rows.reduce((sum, row) => sum + row.cost, 0);
  const medianGoldPerHourAtLevel50 = percentile(
    samples
      .map((sample) => sample.goldPerHourAtLevel50)
      .sort((left, right) => left - right),
    0.5,
  );
  const expectedGoldCostToMax = round(cumulativeExpectedGoldCost, 2);
  const rarityScenarios = ENHANCEMENT_RARITY_SCENARIOS.map((rarity) =>
    buildEnhancementRarityScenario(rarity, medianGoldPerHourAtLevel50),
  );
  const legendaryScenario = rarityScenarios.find(
    (scenario) => scenario.rarity === "Legendary",
  );
  const legendaryEightSlotExpectedGoldCost = round(
    (legendaryScenario?.expectedGoldCostToMax ?? 0) * EQUIPMENT_SLOT_COUNT,
    2,
  );
  const mythicScenario = rarityScenarios.find(
    (scenario) => scenario.rarity === "Mythic",
  );
  const mythicEightSlotExpectedGoldCost = round(
    (mythicScenario?.expectedGoldCostToMax ?? 0) * EQUIPMENT_SLOT_COUNT,
    2,
  );

  return {
    maxLevel: MAX_ENHANCE_LEVEL,
    goldCostFloorToMax,
    expectedGoldCostToMax,
    medianGoldPerHourAtLevel50,
    expectedHoursAtMedianGoldPerHour: round(
      expectedGoldCostToMax / Math.max(1, medianGoldPerHourAtLevel50),
      3,
    ),
    rarityScenarios,
    legendaryEightSlotExpectedGoldCost,
    legendaryEightSlotExpectedHoursAtMedianGoldPerHour: round(
      legendaryEightSlotExpectedGoldCost /
        Math.max(1, medianGoldPerHourAtLevel50),
      3,
    ),
    mythicEightSlotExpectedGoldCost,
    mythicEightSlotExpectedHoursAtMedianGoldPerHour: round(
      mythicEightSlotExpectedGoldCost / Math.max(1, medianGoldPerHourAtLevel50),
      3,
    ),
    rows,
  };
}

function buildEnhancementRarityScenario(
  rarity: EnhanceItemRarity,
  medianGoldPerHourAtLevel50: number,
): EnhancementRarityScenario {
  let expectedGoldCostToMax = 0;
  let goldCostFloorToMax = 0;
  for (
    let currentLevel = 0;
    currentLevel < MAX_ENHANCE_LEVEL;
    currentLevel += 1
  ) {
    const cost = getEnhanceCost(currentLevel, rarity);
    const successRate = getEnhanceSuccessRate(currentLevel);
    goldCostFloorToMax += cost;
    expectedGoldCostToMax += successRate > 0 ? cost / successRate : 0;
  }

  const roundedExpectedGoldCostToMax = round(expectedGoldCostToMax, 2);
  return {
    rarity,
    multiplier: getRarityCostMultiplier(rarity),
    goldCostFloorToMax,
    expectedGoldCostToMax: roundedExpectedGoldCostToMax,
    expectedHoursAtMedianGoldPerHour: round(
      roundedExpectedGoldCostToMax / Math.max(1, medianGoldPerHourAtLevel50),
      3,
    ),
  };
}

function buildPetFeedPressure(samples: SimulationSample[]): PetFeedPressure {
  let cumulativeFeedCost = 0;
  const rows = Array.from({ length: MAX_PET_LEVEL }, (_, currentLevel) => {
    const feedCost = getFeedCost(currentLevel);
    cumulativeFeedCost += feedCost;
    const nextLevel = currentLevel + 1;
    return {
      currentLevel,
      nextLevel,
      feedCost,
      cumulativeFeedCost,
      dogGoldBonusPercentAfterFeed: round(
        getEffectiveBonusPercent(DOG_GOLD_BONUS_PERCENT, nextLevel),
        3,
      ),
      birdDropBonusPercentAfterFeed: round(
        getEffectiveBonusPercent(BIRD_DROP_BONUS_PERCENT, nextLevel),
        3,
      ),
    };
  });
  const medianGoldPerHourAtLevel50 = percentile(
    samples
      .map((sample) => sample.goldPerHourAtLevel50)
      .sort((left, right) => left - right),
    0.5,
  );
  const dogBaseGoldBonusPercent = getEffectiveBonusPercent(
    DOG_GOLD_BONUS_PERCENT,
    0,
  );
  const dogMaxGoldBonusPercent = getEffectiveBonusPercent(
    DOG_GOLD_BONUS_PERCENT,
    MAX_PET_LEVEL,
  );
  const birdBaseDropBonusPercent = getEffectiveBonusPercent(
    BIRD_DROP_BONUS_PERCENT,
    0,
  );
  const birdMaxDropBonusPercent = getEffectiveBonusPercent(
    BIRD_DROP_BONUS_PERCENT,
    MAX_PET_LEVEL,
  );
  const dogGoldBonusDeltaPercentAtMax = round(
    dogMaxGoldBonusPercent - dogBaseGoldBonusPercent,
    3,
  );
  const incrementalGoldPerHourAtMedian = round(
    medianGoldPerHourAtLevel50 * (dogGoldBonusDeltaPercentAtMax / 100),
    3,
  );

  return {
    maxLevel: MAX_PET_LEVEL,
    totalFeedCostToMax: cumulativeFeedCost,
    medianGoldPerHourAtLevel50,
    dogBaseGoldBonusPercent,
    dogMaxGoldBonusPercent,
    dogGoldBonusDeltaPercentAtMax,
    birdBaseDropBonusPercent,
    birdMaxDropBonusPercent,
    birdDropBonusDeltaPercentAtMax: round(
      birdMaxDropBonusPercent - birdBaseDropBonusPercent,
      3,
    ),
    incrementalGoldPerHourAtMedian,
    paybackHoursAtMedianGoldPerHour: round(
      cumulativeFeedCost / Math.max(1, incrementalGoldPerHourAtMedian),
      3,
    ),
    rows,
  };
}

function buildAchievementPressure(): AchievementPressure {
  const referenceTranscendCount = 10;
  const referenceTowerFloor = 100;
  const referenceTranscendMultiplier = round(
    getTranscendStatMultiplier(referenceTranscendCount),
    3,
  );
  const referenceTowerMultiplier = round(
    getTowerMilestoneMultiplier(referenceTowerFloor),
    3,
  );
  const referenceBaseMultiplier =
    referenceTranscendMultiplier * referenceTowerMultiplier;
  const pointAnchors = [0, 3, 100, 125, 250, 500];

  return {
    softCapStartPoints: ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS,
    softCapBonusPoints: ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS,
    referenceTranscendCount,
    referenceTowerFloor,
    referenceTranscendMultiplier,
    referenceTowerMultiplier,
    rows: pointAnchors.map((totalPoints) => {
      const softCappedMultiplier = getAchievementStatMultiplier(totalPoints);
      return {
        totalPoints,
        legacyMultiplier: round(
          1 + Math.max(0, totalPoints) * ACHIEVEMENT_POINTS_MULTIPLIER,
          3,
        ),
        softCappedMultiplier: round(softCappedMultiplier, 3),
        compositeWithTranscendAndTower: round(
          referenceBaseMultiplier * softCappedMultiplier,
          3,
        ),
      };
    }),
  };
}

function buildRunePressure(): RunePressure {
  const coreRows = RUNE_CORE_REVIEW_RARITIES.flatMap((rarity) =>
    RUNE_CORE_REVIEW_LEVELS.map((enhanceLevel) =>
      buildRuneCorePressureRow(rarity, enhanceLevel),
    ),
  );

  return {
    slotCount: REGULAR_RUNE_SLOT_COUNT,
    coreRows,
    combatRows: [
      buildCoreCombatPressureRow(1, 100, MYTHIC_RUNE_RARITY, 50, "PhysAtk"),
      buildCoreCombatPressureRow(2, 100, MYTHIC_RUNE_RARITY, 50, "MagicAtk"),
    ],
    classMasteryRows: ALL_CLASSES.map((classId) =>
      buildClassMasteryPressureRow(
        classId,
        CLASS_MASTERY_REVIEW_LEVEL,
        MYTHIC_RUNE_RARITY,
        CLASS_MASTERY_REVIEW_ENHANCE_LEVEL,
      ),
    ),
    setRows: RUNE_SET_REVIEW_SETS.flatMap((runeSet) =>
      RUNE_SET_REVIEW_COUNTS.map((count) =>
        buildRuneSetPressureRow(runeSet, count),
      ),
    ),
    utilRows: RUNE_UTIL_REVIEW_TYPES.map((runeType) =>
      buildRuneUtilPressureRow(runeType, MYTHIC_RUNE_RARITY),
    ),
  };
}

function buildRuneCodexPressure(
  baseMedianRebirthHours: number,
): RuneCodexPressure {
  const rowBonus = [1, 2, 3, 4, 5, 6, 7].reduce(
    (sum, rarity) => sum + getRowCompletionBonus(rarity),
    0,
  );
  const fullCodexBonus = computeRuneCodexBonus({
    unlockedCells: RUNE_CODEX_TOTAL_CELLS,
    rowComplete: [true, true, true, true, true, true, true],
    coreCategoryComplete: true,
    utilCategoryComplete: true,
  });
  const projectedFullCodexMedianHours =
    baseMedianRebirthHours / (1 + fullCodexBonus.coreStatAdd);

  return {
    totalCells: RUNE_CODEX_TOTAL_CELLS,
    perCellCoreBonusPercent: round(PER_CELL_CORE_BONUS * 100, 3),
    allCellsCoreBonusPercent: round(
      RUNE_CODEX_TOTAL_CELLS * PER_CELL_CORE_BONUS * 100,
      3,
    ),
    allRowsCoreBonusPercent: round(rowBonus * 100, 3),
    coreCategoryBonusPercent: round(CORE_CATEGORY_BONUS * 100, 3),
    utilCategoryCapExtensionPercent: round(
      UTIL_CATEGORY_CAP_EXTENSION * 100,
      3,
    ),
    fullCodexCoreStatAddPercent: round(fullCodexBonus.coreStatAdd * 100, 3),
    baseMedianRebirthHours,
    projectedFullCodexMedianHours: round(projectedFullCodexMedianHours, 3),
    projectedMedianDeltaPercent: round(
      ((projectedFullCodexMedianHours - baseMedianRebirthHours) /
        baseMedianRebirthHours) *
        100,
      1,
    ),
    injectedIntoSampledRun: false,
  };
}

function buildRuneCorePressureRow(
  rarity: number,
  enhanceLevel: number,
): RuneCorePressureRow {
  const singleRuneBonus = getCoreRuneMultiplier(rarity, enhanceLevel);
  return {
    rarity: runeRarityName(rarity),
    enhanceLevel,
    singleRuneBonusPercent: round(singleRuneBonus * 100, 3),
    sixSlotMultiplier: round(1 + singleRuneBonus * REGULAR_RUNE_SLOT_COUNT, 3),
  };
}

function buildCoreCombatPressureRow(
  classId: ClassId,
  level: number,
  rarity: number,
  enhanceLevel: number,
  lane: "PhysAtk" | "MagicAtk",
): RuneCombatPressureRow {
  const baseStats = deriveStats(defaultPrimaryStats(classId, level), level, {
    physAtk: level * 16,
    magicAtk: level * 16,
  });
  const runeMultiplier =
    1 + getCoreRuneMultiplier(rarity, enhanceLevel) * REGULAR_RUNE_SLOT_COUNT;
  const runeStats = {
    ...baseStats,
    physAtk:
      lane === "PhysAtk"
        ? Math.round(baseStats.physAtk * runeMultiplier)
        : baseStats.physAtk,
    magicAtk:
      lane === "MagicAtk"
        ? Math.round(baseStats.magicAtk * runeMultiplier)
        : baseStats.magicAtk,
  };
  const baseCombatPower = computeCombatPower(baseStats);
  const runeCombatPower = computeCombatPower(runeStats);
  const baseDps = effectiveReviewDps(baseStats, classId, level);
  const runeDps = effectiveReviewDps(runeStats, classId, level);

  return {
    runeSet: `${REGULAR_RUNE_SLOT_COUNT}x ${runeRarityName(rarity)} +${enhanceLevel} ${lane}`,
    className: CLASS_NAMES[classId],
    level,
    baseCombatPower,
    runeCombatPower,
    cpMultiplier: round(runeCombatPower / Math.max(1, baseCombatPower), 3),
    baseDps,
    runeDps,
    dpsMultiplier: round(runeDps / Math.max(1, baseDps), 3),
  };
}

function buildClassMasteryPressureRow(
  classId: ClassId,
  level: number,
  rarity: number,
  enhanceLevel: number,
): ClassMasteryPressureRow {
  const baseStats = deriveStats(defaultPrimaryStats(classId, level), level, {
    physAtk: level * 16,
    magicAtk: level * 16,
  });
  const mastery = getClassMasteryMultipliers(classId, rarity, enhanceLevel);
  const classRuneStats = {
    ...baseStats,
    hp: Math.round(baseStats.hp * (1 + mastery.hp)),
    physAtk: Math.round(baseStats.physAtk * (1 + mastery.physAtk)),
    magicAtk: Math.round(baseStats.magicAtk * (1 + mastery.magicAtk)),
    physDef: Math.round(baseStats.physDef * (1 + mastery.physDef)),
    magicDef: Math.round(baseStats.magicDef * (1 + mastery.magicDef)),
  };
  const baseCombatPower = computeCombatPower(baseStats);
  const classRuneCombatPower = computeCombatPower(classRuneStats);
  const baseDps = effectiveReviewDps(baseStats, classId, level);
  const classRuneDps = effectiveReviewDps(classRuneStats, classId, level);

  return {
    classId,
    className: CLASS_NAMES[classId],
    role: CLASS_ROLES[classId],
    level,
    rarity: runeRarityName(rarity),
    enhanceLevel,
    masteryStats: masteryStatNames(mastery),
    baseCombatPower,
    classRuneCombatPower,
    cpMultiplier: round(classRuneCombatPower / Math.max(1, baseCombatPower), 3),
    baseDps,
    classRuneDps,
    dpsMultiplier: round(classRuneDps / Math.max(1, baseDps), 3),
  };
}

function buildRuneSetPressureRow(
  runeSet: number,
  count: number,
): RuneSetPressureRow {
  const classId = RUNE_SET_REVIEW_CLASS;
  const level = RUNE_SET_REVIEW_LEVEL;
  const baseStats = deriveStats(defaultPrimaryStats(classId, level), level, {
    physAtk: level * 16,
    magicAtk: level * 16,
  });
  const bonus = computeRuneSetBonus({ [runeSet]: count });
  const setStats = {
    ...baseStats,
    hp: Math.round(baseStats.hp * (1 + bonus.core.hp)),
    physAtk: Math.round(baseStats.physAtk * (1 + bonus.core.physAtk)),
    magicAtk: Math.round(baseStats.magicAtk * (1 + bonus.core.magicAtk)),
    physDef: Math.round(baseStats.physDef * (1 + bonus.core.physDef)),
    magicDef: Math.round(baseStats.magicDef * (1 + bonus.core.magicDef)),
    critDmg: baseStats.critDmg + bonus.util.critDamage,
  };
  const baseCombatPower = computeCombatPower(baseStats);
  const setCombatPower = computeCombatPower(setStats);
  const baseDps = effectiveReviewDps(baseStats, classId, level);
  const setDps = effectiveReviewDps(setStats, classId, level);

  return {
    runeSet: runeSetName(runeSet),
    count,
    tierBonusPercent: round(getSetTierBonus(count) * 100, 3),
    bonusLanes: runeSetBonusLanes(runeSet),
    className: CLASS_NAMES[classId],
    level,
    baseCombatPower,
    setCombatPower,
    cpMultiplier: round(setCombatPower / Math.max(1, baseCombatPower), 3),
    baseDps,
    setDps,
    dpsMultiplier: round(setDps / Math.max(1, baseDps), 3),
    firstRebirthInjected: false,
  };
}

function buildRuneUtilPressureRow(
  runeType: number,
  rarity: number,
): RuneUtilPressureRow {
  const cap = getUtilCap(runeType);
  const enhanceLevel = firstUtilCapLevel(runeType, rarity);
  const singleRuneValue = getUtilRuneValue(runeType, rarity, enhanceLevel);
  const sixSlotTotal = singleRuneValue * REGULAR_RUNE_SLOT_COUNT;

  return {
    runeType: runeTypeName(runeType),
    rarity: runeRarityName(rarity),
    enhanceLevel,
    singleRuneValuePercent: round(singleRuneValue * 100, 3),
    sixSlotUncappedTotalPercent: round(sixSlotTotal * 100, 3),
    effectiveEconomicMultiplier: round(1 + Math.min(sixSlotTotal, cap), 3),
  };
}

function firstUtilCapLevel(runeType: number, rarity: number): number {
  const baseValue = getUtilRuneValue(runeType, rarity, 0);
  const cap = getUtilCap(runeType);
  const step = runeType === 9 ? 0.003 : 0.005;
  return Math.max(0, Math.ceil((cap - baseValue) / step));
}

function effectiveReviewDps(
  stats: DerivedStats,
  classId: ClassId,
  level: number,
): number {
  const reviewDef = level * CLASS_BALANCE_REVIEW_DEF_PER_LEVEL;
  const baseHit = computeClassDamage(stats, classId, reviewDef, reviewDef);
  const damageSkills = getSkillDefinitionsForClass(classId).filter(
    (skill) => skill.cooldown > 0 && skill.damageCoeff > 0,
  );
  const skillDps = damageSkills.reduce(
    (sum, skill) =>
      sum +
      computeClassDamage(
        {
          physAtk: stats.physAtk * skill.damageCoeff,
          magicAtk: stats.magicAtk * skill.damageCoeff,
        },
        classId,
        reviewDef,
        reviewDef,
      ) /
        skill.cooldown,
    0,
  );
  const effectiveAtkSpeed = 1 + (stats.atkSpeed - 1) * 0.6;
  const critMultiplier = 1 + stats.critRate * (stats.critDmg - 1) * 0.6;
  return Math.round((baseHit * effectiveAtkSpeed + skillDps) * critMultiplier);
}

function masteryStatNames(
  mastery: ReturnType<typeof getClassMasteryMultipliers>,
): string[] {
  return [
    ["PhysAtk", mastery.physAtk],
    ["MagicAtk", mastery.magicAtk],
    ["PhysDef", mastery.physDef],
    ["MagicDef", mastery.magicDef],
    ["Hp", mastery.hp],
  ]
    .filter(([, value]) => value > 0)
    .map(([name]) => String(name));
}

function runeSetName(runeSet: number): string {
  switch (runeSet) {
    case RUNE_SET_OFFENSE:
      return "Offense";
    case RUNE_SET_BASTION:
      return "Bastion";
    case RUNE_SET_VITALITY:
      return "Vitality";
    case RUNE_SET_FORTUNE:
      return "Fortune";
    default:
      throw new Error(`Unsupported rune set ${runeSet}`);
  }
}

function runeSetBonusLanes(runeSet: number): string[] {
  switch (runeSet) {
    case RUNE_SET_OFFENSE:
      return ["PhysAtk", "MagicAtk"];
    case RUNE_SET_BASTION:
      return ["PhysDef", "MagicDef"];
    case RUNE_SET_VITALITY:
      return ["Hp", "OfflineEff"];
    case RUNE_SET_FORTUNE:
      return ["GoldFind", "ExpBoost", "CritDamage"];
    default:
      throw new Error(`Unsupported rune set ${runeSet}`);
  }
}

function runeRarityName(rarity: number): RuneRarityName {
  switch (rarity) {
    case 1:
      return "Common";
    case 2:
      return "Rare";
    case 3:
      return "Epic";
    case 4:
      return "Unique";
    case 5:
      return "Legendary";
    case 6:
      return "Transcendent";
    case 7:
      return "Mythic";
    default:
      throw new Error(`Unsupported rune rarity ${rarity}`);
  }
}

function runeTypeName(type: number): RuneTypeName {
  switch (type) {
    case 6:
      return "CritDamage";
    case 7:
      return "GoldFind";
    case 8:
      return "ExpBoost";
    case 9:
      return "OfflineEff";
    default:
      throw new Error(`Unsupported rune type ${type}`);
  }
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
    `- Elite bonus: ${ELITE_REWARD_BONUS}x`,
    "- Normal kill rewards use `computeKillExp` / `computeKillGold`.",
    "- Source: `server/src/core/formulas/reward.ts`.",
    "- Monster HP and reward multipliers both reuse the stage index ramp.",
    "- Result: 30-stage Chapter 1-3 comparison keeps reward-per-HP pressure stable before elite and boss bonuses.",
    `- Dark stage share: ${report.model.darkElementPressure.darkWeakStageCount}/${report.model.darkElementPressure.totalStageCount}.`,
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Stage | idx | Type | Weak | HP x | Reward x | Normal EXP | Normal Gold | Elite EXP | Elite Gold | Boss EXP | Boss Gold |",
    "| --- | ---: | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.rewardScaling.map(
      (row) =>
        `| ${row.stage} | ${row.globalStageIndex} | ${row.encounterType} | ${row.weakElement} | ${row.monsterHpMultiplier} | ${row.rewardMultiplier} | ${row.normalExp} | ${row.normalGold} | ${row.eliteExp} | ${row.eliteGold} | ${row.bossExp} | ${row.bossGold} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Dark Element Pressure",
    "",
    "- Holy and Dark counter each other at the same 1.5x weakness boundary.",
    "- Dark-heavy chapter 3 increases matchup coverage without changing the",
    "  sampled first-rebirth timing model.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Skill element | Target weakness | Multiplier | Note |",
    "| --- | --- | ---: | --- |",
    ...report.model.darkElementPressure.rows.map(
      (row) =>
        `| ${row.skillElement} | ${row.targetWeakElement} | x${row.multiplier} | ${row.note} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Item Drop Rarity Pressure",
    "",
    "- Source: `server/src/core/formulas/drop.ts`.",
    `- Level 1 total probability: ${round(report.model.dropRarityPressure.totalChanceAtLevel1 * 100, 3)}%`,
    `- Level 100 total probability: ${round(report.model.dropRarityPressure.totalChanceAtLevel100 * 100, 3)}%`,
    "- Unique and Transcendent are interpolation tiers: Unique stays below",
    "  Epic drop pressure, and Transcendent stays below Legendary drop pressure.",
    "- This table reports rarity pressure only; the sampled first-rebirth",
    "  distribution still uses the existing equipmentMultiplier abstraction.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Rarity | Stat x | Lv1 chance | Lv100 chance | Affixes |",
    "| --- | ---: | ---: | ---: | ---: |",
    ...report.model.dropRarityPressure.rows.map(
      (row) =>
        `| ${row.rarity} | ${row.statMultiplier} | ${row.chanceAtLevel1Percent}% | ${row.chanceAtLevel100Percent}% | ${row.affixCount} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Enhancement Spend Pressure",
    "",
    `- Max level: +${report.model.enhancementPressure.maxLevel}`,
    `- Minimum +0 to +50 gold cost: ${report.model.enhancementPressure.goldCostFloorToMax}`,
    `- Expected +0 to +50 gold cost: ${report.model.enhancementPressure.expectedGoldCostToMax}`,
    `- Median sampled Lv50 gold/hour: ${report.model.enhancementPressure.medianGoldPerHourAtLevel50}`,
    `- Expected +0 to +50 cost at median Lv50 gold/hour: ${report.model.enhancementPressure.expectedHoursAtMedianGoldPerHour}h`,
    "- Failure consumes gold only; no downgrade or destruction is modeled in V1.",
    "- V1 enhancement is a long-tail gold sink for infinite growth;",
    "  high-level attempts are expected to outpace sampled Lv50 income and",
    "  keep legendary multi-slot investment open-ended.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Current | Next | Cost | Success | Expected attempts | Expected gold | Cumulative expected gold |",
    "| ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.enhancementPressure.rows.map(
      (row) =>
        `| +${row.currentLevel} | +${row.nextLevel} | ${row.cost} | ${Math.round(row.successRate * 100)}% | ${row.expectedAttempts} | ${row.expectedGoldCost} | ${row.cumulativeExpectedGoldCost} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Rarity Enhancement Pressure",
    "",
    "- Rarity scales cost only; success rates, failure behavior, and stat payoff",
    "  stay unchanged.",
    `- Eight Legendary slots: ${formatNumber(report.model.enhancementPressure.legendaryEightSlotExpectedGoldCost)} expected gold,`,
    `  ${report.model.enhancementPressure.legendaryEightSlotExpectedHoursAtMedianGoldPerHour}h at sampled median Lv50 gold/hour.`,
    `- Eight Mythic slots: ${formatNumber(report.model.enhancementPressure.mythicEightSlotExpectedGoldCost)} expected gold,`,
    `  ${report.model.enhancementPressure.mythicEightSlotExpectedHoursAtMedianGoldPerHour}h at sampled median Lv50 gold/hour.`,
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Rarity | Multiplier | Minimum +0 to +50 gold | Expected +0 to +50 gold | Hours at median Lv50 gold/hour |",
    "| --- | ---: | ---: | ---: | ---: |",
    ...report.model.enhancementPressure.rarityScenarios.map(
      (row) =>
        `| ${row.rarity} | ${row.multiplier} | ${row.goldCostFloorToMax} | ${row.expectedGoldCostToMax} | ${row.expectedHoursAtMedianGoldPerHour}h |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Pet Feed Gold Pressure",
    "",
    `- Max pet level: ${report.model.petFeedPressure.maxLevel}`,
    `- Total Lv0 to Lv10 feed cost: ${report.model.petFeedPressure.totalFeedCostToMax}`,
    `- Dog gold bonus: ${report.model.petFeedPressure.dogBaseGoldBonusPercent}% to ${report.model.petFeedPressure.dogMaxGoldBonusPercent}% (+${report.model.petFeedPressure.dogGoldBonusDeltaPercentAtMax} percentage points).`,
    `- Bird drop bonus: ${report.model.petFeedPressure.birdBaseDropBonusPercent}% to ${report.model.petFeedPressure.birdMaxDropBonusPercent}% (+${report.model.petFeedPressure.birdDropBonusDeltaPercentAtMax} percentage points).`,
    `- Median sampled Lv50 gold/hour: ${report.model.petFeedPressure.medianGoldPerHourAtLevel50}`,
    `- Incremental dog gold at median: ${report.model.petFeedPressure.incrementalGoldPerHourAtMedian}/h.`,
    `- Dog payback at median Lv50 gold/hour: ${report.model.petFeedPressure.paybackHoursAtMedianGoldPerHour}h.`,
    "- This treats the Lv10 dog upgrade as an income investment against the",
    "  existing PR #32 sampled blended gold rate. It does not grant combat power,",
    "  so the sink competes with enhancement/shop gold without shortening",
    "  first-rebirth EXP pacing directly.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Current | Next | Feed cost | Cumulative cost | Dog gold after feed | Bird drop after feed |",
    "| ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.petFeedPressure.rows.map(
      (row) =>
        `| Lv${row.currentLevel} | Lv${row.nextLevel} | ${row.feedCost} | ${row.cumulativeFeedCost} | ${row.dogGoldBonusPercentAfterFeed}% | ${row.birdDropBonusPercentAfterFeed}% |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Achievement Multiplier Pressure",
    "",
    `- Soft cap starts at ${report.model.achievementPressure.softCapStartPoints} points.`,
    `- Soft-cap bonus budget: ${report.model.achievementPressure.softCapBonusPoints} effective points.`,
    "- 100 points stays at x2 before the soft-cap slope decays toward x2.5.",
    `- Composite reference: transcend ${report.model.achievementPressure.referenceTranscendCount} (x${report.model.achievementPressure.referenceTranscendMultiplier})`,
    `  and tower floor ${report.model.achievementPressure.referenceTowerFloor} (x${report.model.achievementPressure.referenceTowerMultiplier}).`,
    "- Achievement tiers remain infinite for collection depth, but stat growth",
    "  is bounded enough that transcend and tower stay the primary prestige",
    "  multipliers.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Points | Legacy x | Soft-capped x | Composite x |",
    "| ---: | ---: | ---: | ---: |",
    ...report.model.achievementPressure.rows.map(
      (row) =>
        `| ${row.totalPoints} | ${row.legacyMultiplier} | ${row.softCappedMultiplier} | ${row.compositeWithTranscendAndTower} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Rune Growth Pressure",
    "",
    `- Rune slots: ${report.model.runePressure.slotCount}`,
    "- Core rune growth is intentionally uncapped; the table shows one rune's",
    "  stat bonus and the resulting same-lane 6-slot multiplier.",
    "- Combat rows apply six matching Mythic +50 core runes to the shared Lv100",
    "  review loadout. CP and DPS are reported as pressure signals, not new",
    "  tuning gates.",
    "- Util rows show the first Mythic enhance level that reaches the per-rune",
    "  cap. Six capped utility runes are possible and therefore create very",
    "  large specialized economy multipliers.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Rarity | Enhance | Single rune bonus | 6-slot multiplier |",
    "| --- | ---: | ---: | ---: |",
    ...report.model.runePressure.coreRows.map(
      (row) =>
        `| ${row.rarity} | +${row.enhanceLevel} | ${row.singleRuneBonusPercent}% | x${row.sixSlotMultiplier} |`,
    ),
    "",
    "| Rune set | Class | Lv | Base CP | Rune CP | CP x | Base DPS | Rune DPS | DPS x |",
    "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.runePressure.combatRows.map(
      (row) =>
        `| ${row.runeSet} | ${row.className} | ${row.level} | ${row.baseCombatPower} | ${row.runeCombatPower} | x${row.cpMultiplier} | ${row.baseDps} | ${row.runeDps} | x${row.dpsMultiplier} |`,
    ),
    "",
    "## Class Mastery Rune Pressure",
    "",
    "- ClassMastery uses the seventh dedicated rune slot. The table applies one",
    "  Mythic +50 class rune to each Lv100 review loadout and keeps the regular",
    "  six core slots out of the comparison.",
    "- Damage-role rows are checked against the PR #60 +/-15% DPS band after the",
    "  class rune is applied. Two-stat Warrior, Cleric, and Paladin rows are",
    "  treated as CP/survival compensation, not an extra DPS lane.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Class | Role | Mastery stats | Base CP | Rune CP | CP x | Base DPS | Rune DPS | DPS x |",
    "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.runePressure.classMasteryRows.map(
      (row) =>
        `| ${row.className} | ${row.role} | ${row.masteryStats.join(", ")} | ${row.baseCombatPower} | ${row.classRuneCombatPower} | x${row.cpMultiplier} | ${row.baseDps} | ${row.classRuneDps} | x${row.dpsMultiplier} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Rune Set Bonus Pressure",
    "",
    "- Set bonuses apply only to the six regular rune slots; the ClassMastery",
    "  slot remains outside set counting.",
    "- Rows compare 2/4/6 same-set tiers on the shared Lv100 Warrior review",
    "  loadout. Utility economy lanes are listed as pressure signals, while",
    "  only combat-facing lanes change CP or DPS in this table.",
    "- Rune set acquisition is not injected into the sampled first-rebirth run;",
    "  the 1000-run median remains the PR #61 baseline guard.",
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Rune set | Count | Bonus | Bonus lanes | Base CP | Set CP | CP x | Base DPS | Set DPS | DPS x |",
    "| --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |",
    ...report.model.runePressure.setRows.map(
      (row) =>
        `| ${row.runeSet} | ${row.count} | ${row.tierBonusPercent}% | ${row.bonusLanes.join(", ")} | ${row.baseCombatPower} | ${row.setCombatPower} | x${row.cpMultiplier} | ${row.baseDps} | ${row.setDps} | x${row.dpsMultiplier} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "| Util rune | Rarity | Cap enhance | Single rune value | 6-slot total | Effective multiplier |",
    "| --- | --- | ---: | ---: | ---: | ---: |",
    ...report.model.runePressure.utilRows.map(
      (row) =>
        `| ${row.runeType} | ${row.rarity} | +${row.enhanceLevel} | ${row.singleRuneValuePercent}% | ${row.sixSlotUncappedTotalPercent}% | x${row.effectiveEconomicMultiplier} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
    "## Rune Codex Collection Pressure",
    "",
    `- Total cells: ${report.model.runeCodexPressure.totalCells}`,
    `- Per-cell core bonus: +${report.model.runeCodexPressure.perCellCoreBonusPercent}%`,
    `- All-cell core bonus: +${report.model.runeCodexPressure.allCellsCoreBonusPercent}%`,
    `- All row-completion bonuses: +${report.model.runeCodexPressure.allRowsCoreBonusPercent}%`,
    `- Core category completion bonus: +${report.model.runeCodexPressure.coreCategoryBonusPercent}%`,
    `- Util category cap extension: +${report.model.runeCodexPressure.utilCategoryCapExtensionPercent}%`,
    `- Full codex core bonus: +${report.model.runeCodexPressure.fullCodexCoreStatAddPercent}%`,
    `- Base median first-rebirth time: ${report.model.runeCodexPressure.baseMedianRebirthHours}h`,
    `- Projected full-codex median if applied directly to first rebirth DPS: ${report.model.runeCodexPressure.projectedFullCodexMedianHours}h (${report.model.runeCodexPressure.projectedMedianDeltaPercent}%).`,
    "- Not injected into the sampled first-rebirth run. The 1000-run median",
    "  remains the baseline guard until rune acquisition/drop rates are wired",
    "  into the simulation model.",
    "",
    "## Class Balance Snapshot",
    "",
    "- Effective DPS uses the class attack route through `computeClassDamage`,",
    "  review defense, attack speed, crit expectation, and active skill",
    "  `damageCoeff / cooldown` pressure from `skills.ts`.",
    "- DPS classes target +/-15% around each level's DPS median.",
    "- Paladin and Cleric are role exceptions: tank/healer utility may sit below",
    "  the DPS band while preserving survival/support compensation.",
    "",
    ...renderClassBalanceTables(report.model.classBalance),
    "## Formula Sources",
    "",
    ...report.model.formulas.map((source) => `- ${source}`),
  ];
  return `${lines.join("\n")}\n`;
}

function renderClassBalanceTables(snapshot: ClassBalanceSnapshot): string[] {
  return snapshot.levels.flatMap((level) => [
    `### Lv${level}`,
    "",
    "<!-- markdownlint-disable MD013 -->",
    "",
    "| Class | Role | HP | Effective ATK | Effective DPS | DPS delta | CP |",
    "| --- | --- | ---: | ---: | ---: | ---: | ---: |",
    ...snapshot.rowsByLevel[level].map(
      (row) =>
        `| ${row.className} | ${row.role} | ${row.hp} | ${row.effectiveAttack} | ${row.effectiveDps} | ${Math.round(row.dpsDeltaFromMedian * 100)}% | ${row.combatPower} |`,
    ),
    "",
    "<!-- markdownlint-enable MD013 -->",
    "",
  ]);
}

function formatNumber(value: number): string {
  return new Intl.NumberFormat("en-US", {
    maximumFractionDigits: 2,
    minimumFractionDigits: Number.isInteger(value) ? 0 : 2,
  }).format(value);
}

function randomClass(random: () => number): ClassId {
  return ALL_CLASSES[Math.floor(random() * ALL_CLASSES.length)] ?? 1;
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

const ALL_CLASSES: ClassId[] = [1, 2, 3, 4, 5, 6, 7, 8];

const CLASS_NAMES: Record<ClassId, string> = {
  1: "Warrior",
  2: "Mage",
  3: "Archer",
  4: "Thief",
  5: "Cleric",
  6: "Paladin",
  7: "Berserker",
  8: "Summoner",
};

const CLASS_ROLES: Record<ClassId, ClassRole> = {
  1: "dps",
  2: "dps",
  3: "dps",
  4: "dps",
  5: "healer",
  6: "tank",
  7: "dps",
  8: "dps",
};

function isMagicClass(classId: ClassId): boolean {
  return classId === 2 || classId === 5 || classId === 8;
}

if (process.argv[1] === fileURLToPath(import.meta.url)) {
  const distribution = simulateRebirthDistribution();
  const report = buildBalanceReport(distribution);
  const paths = writeBalanceReport(report);
  console.log(report.markdown);
  console.log(`Wrote ${paths.markdownPath}`);
  console.log(`Wrote ${paths.jsonPath}`);
}
