export const DUNGEON_TYPE_GOLD = 1;
export const DUNGEON_TYPE_EXP = 2;
export const DUNGEON_TYPE_ESSENCE = 3;
export const DUNGEON_DAILY_ENTRY_LIMIT = 3;
export const TIER_CP_FACTOR = 2.0;

const INT64_MAX = 9223372036854776000;
const GOLD_DUNGEON_BASE_REWARD = 20000.0;
const EXP_DUNGEON_BASE_REWARD = 20000.0;
const ESSENCE_DUNGEON_BASE_REWARD = 12.0;

export type DungeonReward = {
  gold: number;
  exp: number;
  essence: number;
};

function roundAndClampToInt64(value: number): number {
  if (!Number.isFinite(value) || value >= INT64_MAX) {
    return INT64_MAX;
  }

  return Math.max(0, Math.round(value));
}

function getBaseReward(type: number): number {
  switch (type) {
    case DUNGEON_TYPE_GOLD:
      return GOLD_DUNGEON_BASE_REWARD;
    case DUNGEON_TYPE_EXP:
      return EXP_DUNGEON_BASE_REWARD;
    case DUNGEON_TYPE_ESSENCE:
      return ESSENCE_DUNGEON_BASE_REWARD;
    default:
      return 0;
  }
}

function getCpMultiplier(type: number, combatPower: number): number {
  const minimumCp = Math.max(1, getMinimumCp(type));
  const cpRatio = Math.fround(Math.trunc(combatPower) / minimumCp);
  return Math.fround(Math.sqrt(Math.max(1, cpRatio)));
}

export function getTierCpRequirement(type: number, tier: number): number {
  const normalizedTier = Math.trunc(tier);
  const minimumCp = getMinimumCp(type);
  if (getDailyEntryLimit(type) <= 0 || minimumCp <= 0 || normalizedTier < 1) {
    return 0;
  }

  return roundAndClampToInt64(
    Math.fround(minimumCp * TIER_CP_FACTOR ** (normalizedTier - 1)),
  );
}

export function getMaxAccessibleTier(
  type: number,
  combatPower: number,
): number {
  const normalizedCombatPower = Math.trunc(combatPower);
  const minimumCp = getMinimumCp(type);
  if (
    getDailyEntryLimit(type) <= 0 ||
    minimumCp <= 0 ||
    normalizedCombatPower < minimumCp
  ) {
    return 0;
  }

  const cpRatio = Math.fround(normalizedCombatPower / minimumCp);
  return Math.floor(Math.log(cpRatio) / Math.log(TIER_CP_FACTOR)) + 1;
}

export function tierRewardMultiplier(tier: number): number {
  return Math.max(1, Math.trunc(tier));
}

export function getDailyEntryLimit(type: number): number {
  switch (Math.trunc(type)) {
    case DUNGEON_TYPE_GOLD:
    case DUNGEON_TYPE_EXP:
    case DUNGEON_TYPE_ESSENCE:
      return DUNGEON_DAILY_ENTRY_LIMIT;
    default:
      return 0;
  }
}

export function getMinimumCp(type: number): number {
  switch (Math.trunc(type)) {
    case DUNGEON_TYPE_GOLD:
      return 100;
    case DUNGEON_TYPE_EXP:
      return 250;
    case DUNGEON_TYPE_ESSENCE:
      return 500;
    default:
      return 0;
  }
}

export function getDungeonReward(
  type: number,
  combatPower: number,
  tier = 1,
): DungeonReward {
  const normalizedType = Math.trunc(type);
  const normalizedCombatPower = Math.trunc(combatPower);
  const normalizedTier = Math.trunc(tier);
  const emptyReward = { gold: 0, exp: 0, essence: 0 };

  if (
    getDailyEntryLimit(normalizedType) <= 0 ||
    normalizedCombatPower <= 0 ||
    normalizedTier < 1 ||
    normalizedCombatPower < getTierCpRequirement(normalizedType, normalizedTier)
  ) {
    return emptyReward;
  }

  const multiplier = getCpMultiplier(normalizedType, normalizedCombatPower);
  const rewardMultiplier = tierRewardMultiplier(normalizedTier);
  const baseReward = getBaseReward(normalizedType);
  switch (normalizedType) {
    case DUNGEON_TYPE_GOLD:
      return {
        gold: roundAndClampToInt64(
          Math.fround(baseReward * multiplier) * rewardMultiplier,
        ),
        exp: 0,
        essence: 0,
      };
    case DUNGEON_TYPE_EXP:
      return {
        gold: 0,
        exp: roundAndClampToInt64(
          Math.fround(baseReward * multiplier) * rewardMultiplier,
        ),
        essence: 0,
      };
    case DUNGEON_TYPE_ESSENCE:
      return {
        gold: 0,
        exp: 0,
        essence: roundAndClampToInt64(
          Math.fround(baseReward * multiplier) * rewardMultiplier,
        ),
      };
    default:
      return emptyReward;
  }
}
