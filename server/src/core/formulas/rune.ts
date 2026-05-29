export const RUNE_SLOT_COUNT = 7;

const CORE_BASE_BY_RARITY = [
  0, 0.02, 0.05, 0.08, 0.1, 0.12, 0.15, 0.18,
] as const;
const CORE_STEP_BY_RARITY = [
  0, 0.004, 0.009, 0.014, 0.017, 0.02, 0.025, 0.03,
] as const;
const UTIL_BASE_BY_RARITY = [
  0, 0.02, 0.04, 0.06, 0.075, 0.09, 0.105, 0.12,
] as const;
const DISENCHANT_BASE_BY_RARITY = [0, 1, 5, 12, 20, 30, 50, 80] as const;

// 세트 리롤: 레어도 가중 룬 정수 비용(레어도↑일수록 비싸짐, 단조 증가)
const REROLL_SET_ESSENCE_BY_RARITY = [
  0, 20, 50, 100, 200, 400, 800, 1500,
] as const;

// 등급 상승 시도: r→r+1 룬 정수 비용(기하 증가). 인덱스 7(Mythic)은 상승 불가 → 0
const RARITY_UPGRADE_ESSENCE_BY_RARITY = [
  0, 100, 250, 600, 1500, 4000, 10000, 0,
] as const;

// 등급 상승 시도: r→r+1 골드 비용(기하 증가). 인덱스 7(Mythic)은 상승 불가 → 0
const RARITY_UPGRADE_GOLD_BY_RARITY = [
  0, 5000, 15000, 50000, 150000, 500000, 1500000, 0,
] as const;

// 등급 상승 성공 확률: r→r+1, 등급↑일수록 ↓. 인덱스 7(Mythic)은 상승 불가 → 0
const RARITY_UPGRADE_CHANCE_BY_RARITY = [
  0, 0.6, 0.45, 0.3, 0.2, 0.12, 0.05, 0,
] as const;

// 전송 비용: base + sourceLevel * step (룬 정수)
const TRANSFER_ESSENCE_BASE = 50;
const TRANSFER_ESSENCE_STEP = 25;

function clampedLevel(level: number): number {
  return Math.max(0, Math.trunc(level));
}

function validRarity(rarity: number): boolean {
  return Number.isInteger(rarity) && rarity >= 1 && rarity <= 7;
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

// 세트 리롤 룬 정수 비용(레어도 가중). 유효 레어도 외에는 0
export function getRerollSetEssenceCost(rarity: number): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return REROLL_SET_ESSENCE_BY_RARITY[rarity];
}

// 등급 상승(r→r+1) 시도 룬 정수 비용. Mythic(7)/유효 외에는 0(상승 불가 가드)
export function getRarityUpgradeEssenceCost(rarity: number): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return RARITY_UPGRADE_ESSENCE_BY_RARITY[rarity];
}

// 등급 상승(r→r+1) 시도 골드 비용. Mythic(7)/유효 외에는 0(상승 불가 가드)
export function getRarityUpgradeGoldCost(rarity: number): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return RARITY_UPGRADE_GOLD_BY_RARITY[rarity];
}

// 등급 상승(r→r+1) 성공 확률(0~1). 등급↑일수록 ↓, Mythic(7)은 0(상한 가드)
export function getRarityUpgradeChance(rarity: number): number {
  if (!validRarity(rarity)) {
    return 0;
  }

  return Math.fround(RARITY_UPGRADE_CHANCE_BY_RARITY[rarity]);
}

// 강화 레벨 전송 룬 정수 비용(base + sourceLevel * step). sourceLevel은 clamp
export function getTransferEssenceCost(sourceLevel: number): number {
  return (
    TRANSFER_ESSENCE_BASE + clampedLevel(sourceLevel) * TRANSFER_ESSENCE_STEP
  );
}
