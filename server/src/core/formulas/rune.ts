export const RUNE_SLOT_COUNT = 6;

const CORE_BASE_BY_RARITY = [0, 0.02, 0.035, 0.05, 0.08, 0.12, 0.18] as const;
const CORE_STEP_BY_RARITY = [
  0, 0.004, 0.006, 0.009, 0.014, 0.02, 0.03,
] as const;
const UTIL_BASE_BY_RARITY = [0, 0.02, 0.03, 0.04, 0.06, 0.09, 0.12] as const;
const DISENCHANT_BASE_BY_RARITY = [0, 1, 2, 5, 12, 30, 80] as const;

function clampedLevel(level: number): number {
  return Math.max(0, Math.trunc(level));
}

function validRarity(rarity: number): boolean {
  return Number.isInteger(rarity) && rarity >= 1 && rarity <= 6;
}

export function isCoreRuneType(type: number): boolean {
  return Number.isInteger(type) && type >= 1 && type <= 5;
}

export function isUtilRuneType(type: number): boolean {
  return Number.isInteger(type) && type >= 6 && type <= 9;
}

export function getCoreRuneMultiplier(
  rarity: number,
  enhanceLevel: number,
): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return Math.fround(
    CORE_BASE_BY_RARITY[rarity] +
      clampedLevel(enhanceLevel) * CORE_STEP_BY_RARITY[rarity],
  );
}

export function getUtilCap(type: number): number {
  switch (type) {
    case 6:
      return 1;
    case 7:
    case 8:
      return 2;
    case 9:
      return 0.5;
    default:
      return 0;
  }
}

function getUtilStep(type: number): number {
  return type === 9 ? 0.003 : isUtilRuneType(type) ? 0.005 : 0;
}

export function getUtilRuneValue(
  type: number,
  rarity: number,
  enhanceLevel: number,
): number {
  if (!isUtilRuneType(type) || !validRarity(rarity)) {
    return 0;
  }

  return Math.fround(
    Math.min(
      UTIL_BASE_BY_RARITY[rarity] +
        clampedLevel(enhanceLevel) * getUtilStep(type),
      getUtilCap(type),
    ),
  );
}

export function getEnhanceEssenceCost(currentLevel: number): number {
  return 10 * (clampedLevel(currentLevel) + 1) ** 2;
}

export function getEnhanceGoldCost(currentLevel: number): number {
  return 1000 * (clampedLevel(currentLevel) + 1) ** 2;
}

export function getDisenchantEssence(
  rarity: number,
  enhanceLevel: number,
): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return DISENCHANT_BASE_BY_RARITY[rarity] + clampedLevel(enhanceLevel) * 2;
}

export function getShopRuneRollCost(progressIndex: number): number {
  return Math.round(5000 * (1 + Math.max(0, Math.trunc(progressIndex)) * 0.1));
}
