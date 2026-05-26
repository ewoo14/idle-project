export const MAX_ENHANCE_LEVEL = 5;

export type EnhanceItemRarity =
  | "None"
  | "Common"
  | "Uncommon"
  | "Rare"
  | "Epic"
  | "Legendary";

const BASE_ENHANCE_COST = 100;
const ENHANCE_SUCCESS_RATES = [0.95, 0.85, 0.7, 0.55, 0.4] as const;

export function getRarityCostMultiplier(rarity: EnhanceItemRarity): number {
  switch (rarity) {
    case "Common":
      return 1;
    case "Uncommon":
      return 2;
    case "Rare":
      return 4;
    case "Epic":
      return 8;
    case "Legendary":
      return 16;
    case "None":
      return 0;
  }
}

export function getEnhanceCost(
  currentLevel: number,
  rarity: EnhanceItemRarity = "Common",
): number {
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return 0;
  }

  const clampedLevel = Math.max(0, currentLevel);
  return (
    BASE_ENHANCE_COST *
    (clampedLevel + 1) ** 2 *
    getRarityCostMultiplier(rarity)
  );
}

export function getEnhanceSuccessRate(currentLevel: number): number {
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return 0;
  }

  const clampedLevel = Math.max(
    0,
    Math.min(currentLevel, MAX_ENHANCE_LEVEL - 1),
  );
  return ENHANCE_SUCCESS_RATES[clampedLevel];
}

export function rollEnhanceSuccess(
  rate: number,
  rng: () => number = Math.random,
): boolean {
  const clampedRate = Math.max(0, Math.min(rate, 1));
  return rng() < clampedRate;
}
