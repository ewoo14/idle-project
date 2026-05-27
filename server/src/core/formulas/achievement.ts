export const ACHIEVEMENT_TIER_GROWTH_DEFAULT = 2;
export const ACHIEVEMENT_POINTS_MULTIPLIER = 0.01;
export const ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS = 100;
export const ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS = 50;

export type AchievementCategory =
  | "Combat"
  | "Progression"
  | "Gear"
  | "Economy"
  | "Skill"
  | "Pet"
  | "Quest"
  | "Collection"
  | "Misc";

export type AchievementMetric =
  | "MonstersKilled"
  | "BossesKilled"
  | "HighestLevelReached"
  | "StagesCleared"
  | "RebirthCount"
  | "TranscendCount"
  | "TowerHighestFloor"
  | "GearEnhanced"
  | "HighestEnhanceLevel"
  | "ItemsCollected"
  | "UniqueItemsFound"
  | "GoldEarned"
  | "GoldSpent"
  | "GearRollsPurchased"
  | "SkillRankUps"
  | "HighestSkillRank"
  | "PetsFed"
  | "HighestPetLevel"
  | "QuestsCompleted"
  | "SeasonRewardsClaimed"
  | "OfflineRewardsClaimed"
  | "DaysPlayed";

export type AchievementMetricMode = "Cumulative" | "Maximum";

export type AchievementDefinition = {
  id: string;
  category: AchievementCategory;
  metric: AchievementMetric;
  metricMode: AchievementMetricMode;
  baseThreshold: number;
  growth: number;
  pointsPerTier: number;
  displayNameKey: string;
  displayName: string;
};

type AchievementDefinitionSeed = readonly [
  string,
  AchievementCategory,
  AchievementMetric,
  AchievementMetricMode,
  number,
  number,
  string,
  string,
];

const ACHIEVEMENT_SEEDS: AchievementDefinitionSeed[] = [
  [
    "combat_monster_slayer",
    "Combat",
    "MonstersKilled",
    "Cumulative",
    10,
    1,
    "ACHIEVEMENT_NAME_COMBAT_MONSTER_SLAYER",
    "Monster Slayer",
  ],
  [
    "combat_boss_breaker",
    "Combat",
    "BossesKilled",
    "Cumulative",
    1,
    2,
    "ACHIEVEMENT_NAME_COMBAT_BOSS_BREAKER",
    "Boss Breaker",
  ],
  [
    "combat_tower_climber",
    "Combat",
    "TowerHighestFloor",
    "Maximum",
    10,
    2,
    "ACHIEVEMENT_NAME_COMBAT_TOWER_CLIMBER",
    "Tower Climber",
  ],
  [
    "progress_level_peak",
    "Progression",
    "HighestLevelReached",
    "Maximum",
    10,
    1,
    "ACHIEVEMENT_NAME_PROGRESS_LEVEL_PEAK",
    "Level Peak",
  ],
  [
    "progress_stage_clear",
    "Progression",
    "StagesCleared",
    "Cumulative",
    5,
    1,
    "ACHIEVEMENT_NAME_PROGRESS_STAGE_CLEAR",
    "Stage Clear",
  ],
  [
    "progress_rebirth_cycle",
    "Progression",
    "RebirthCount",
    "Maximum",
    1,
    3,
    "ACHIEVEMENT_NAME_PROGRESS_REBIRTH_CYCLE",
    "Rebirth Cycle",
  ],
  [
    "progress_transcend_path",
    "Progression",
    "TranscendCount",
    "Maximum",
    1,
    5,
    "ACHIEVEMENT_NAME_PROGRESS_TRANSCEND_PATH",
    "Transcend Path",
  ],
  [
    "gear_enhancer",
    "Gear",
    "GearEnhanced",
    "Cumulative",
    5,
    1,
    "ACHIEVEMENT_NAME_GEAR_ENHANCER",
    "Gear Enhancer",
  ],
  [
    "gear_peak_level",
    "Gear",
    "HighestEnhanceLevel",
    "Maximum",
    5,
    2,
    "ACHIEVEMENT_NAME_GEAR_PEAK_LEVEL",
    "Peak Enhancement",
  ],
  [
    "gear_collector",
    "Gear",
    "ItemsCollected",
    "Cumulative",
    10,
    1,
    "ACHIEVEMENT_NAME_GEAR_COLLECTOR",
    "Gear Collector",
  ],
  [
    "economy_gold_earner",
    "Economy",
    "GoldEarned",
    "Cumulative",
    1000,
    1,
    "ACHIEVEMENT_NAME_ECONOMY_GOLD_EARNER",
    "Gold Earner",
  ],
  [
    "economy_gold_sink",
    "Economy",
    "GoldSpent",
    "Cumulative",
    1000,
    1,
    "ACHIEVEMENT_NAME_ECONOMY_GOLD_SINK",
    "Gold Sink",
  ],
  [
    "economy_shop_rolls",
    "Economy",
    "GearRollsPurchased",
    "Cumulative",
    5,
    1,
    "ACHIEVEMENT_NAME_ECONOMY_SHOP_ROLLS",
    "Shop Regular",
  ],
  [
    "skill_rank_ups",
    "Skill",
    "SkillRankUps",
    "Cumulative",
    5,
    1,
    "ACHIEVEMENT_NAME_SKILL_RANK_UPS",
    "Skill Training",
  ],
  [
    "skill_rank_peak",
    "Skill",
    "HighestSkillRank",
    "Maximum",
    5,
    2,
    "ACHIEVEMENT_NAME_SKILL_RANK_PEAK",
    "Skill Mastery",
  ],
  [
    "pet_feeder",
    "Pet",
    "PetsFed",
    "Cumulative",
    5,
    1,
    "ACHIEVEMENT_NAME_PET_FEEDER",
    "Pet Care",
  ],
  [
    "pet_level_peak",
    "Pet",
    "HighestPetLevel",
    "Maximum",
    5,
    2,
    "ACHIEVEMENT_NAME_PET_LEVEL_PEAK",
    "Pet Bond",
  ],
  [
    "quest_helper",
    "Quest",
    "QuestsCompleted",
    "Cumulative",
    3,
    1,
    "ACHIEVEMENT_NAME_QUEST_HELPER",
    "Quest Helper",
  ],
  [
    "quest_season_rewards",
    "Quest",
    "SeasonRewardsClaimed",
    "Cumulative",
    3,
    1,
    "ACHIEVEMENT_NAME_QUEST_SEASON_REWARDS",
    "Season Claimer",
  ],
  [
    "collection_unique_items",
    "Collection",
    "UniqueItemsFound",
    "Cumulative",
    5,
    2,
    "ACHIEVEMENT_NAME_COLLECTION_UNIQUE_ITEMS",
    "Unique Finds",
  ],
  [
    "misc_offline_claims",
    "Misc",
    "OfflineRewardsClaimed",
    "Cumulative",
    3,
    1,
    "ACHIEVEMENT_NAME_MISC_OFFLINE_CLAIMS",
    "Welcome Back",
  ],
  [
    "misc_days_played",
    "Misc",
    "DaysPlayed",
    "Cumulative",
    1,
    1,
    "ACHIEVEMENT_NAME_MISC_DAYS_PLAYED",
    "Daily Rhythm",
  ],
];

export const ACHIEVEMENTS: AchievementDefinition[] = ACHIEVEMENT_SEEDS.map(
  ([
    id,
    category,
    metric,
    metricMode,
    baseThreshold,
    pointsPerTier,
    displayNameKey,
    displayName,
  ]) => ({
    id,
    category,
    metric,
    metricMode,
    baseThreshold,
    growth: ACHIEVEMENT_TIER_GROWTH_DEFAULT,
    pointsPerTier,
    displayNameKey,
    displayName,
  }),
);

export function getTierForValue(
  definition: AchievementDefinition,
  value: number,
): number {
  if (value <= 0 || definition.baseThreshold <= 0 || definition.growth <= 1) {
    return 0;
  }

  let tier = 0;
  let threshold = definition.baseThreshold;
  while (threshold <= value && Number.isFinite(threshold)) {
    tier += 1;
    if (threshold > Number.MAX_SAFE_INTEGER / definition.growth) {
      break;
    }
    threshold *= definition.growth;
  }
  return tier;
}

export function getPointsForTiers(
  definition: AchievementDefinition,
  tier: number,
): number {
  return (
    Math.max(0, Math.trunc(tier)) *
    Math.max(1, Math.trunc(definition.pointsPerTier))
  );
}

export function getAchievementStatMultiplier(totalPoints: number): number {
  const safePoints = Math.max(0, Math.trunc(totalPoints));
  const effectivePoints =
    safePoints <= ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS
      ? safePoints
      : ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS +
        ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS *
          (1 -
            Math.exp(
              -(
                (safePoints - ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS) /
                ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS
              ),
            ));

  return Math.fround(
    Math.fround(1) +
      Math.fround(effectivePoints * Math.fround(ACHIEVEMENT_POINTS_MULTIPLIER)),
  );
}
