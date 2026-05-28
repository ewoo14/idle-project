export const DUNGEON_TYPE_GOLD = 1;
export const DUNGEON_TYPE_EXP = 2;
export const DUNGEON_TYPE_ESSENCE = 3;
export const DUNGEON_DAILY_ENTRY_LIMIT = 3;

const CP_REWARD_SCALE = 100.0;
const INT64_MAX = 9223372036854776000;

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

function getCpMultiplier(combatPower: number): number {
  return Math.max(1, Math.fround(Math.trunc(combatPower) / CP_REWARD_SCALE));
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
): DungeonReward {
  const normalizedType = Math.trunc(type);
  const normalizedCombatPower = Math.trunc(combatPower);
  const emptyReward = { gold: 0, exp: 0, essence: 0 };

  if (
    getDailyEntryLimit(normalizedType) <= 0 ||
    normalizedCombatPower <= 0 ||
    normalizedCombatPower < getMinimumCp(normalizedType)
  ) {
    return emptyReward;
  }

  const multiplier = getCpMultiplier(normalizedCombatPower);
  switch (normalizedType) {
    case DUNGEON_TYPE_GOLD:
      return {
        gold: roundAndClampToInt64(1000.0 * multiplier),
        exp: 0,
        essence: 0,
      };
    case DUNGEON_TYPE_EXP:
      return {
        gold: 0,
        exp: roundAndClampToInt64(500.0 * multiplier),
        essence: 0,
      };
    case DUNGEON_TYPE_ESSENCE:
      return {
        gold: 0,
        exp: 0,
        essence: roundAndClampToInt64(10.0 * multiplier),
      };
    default:
      return emptyReward;
  }
}
