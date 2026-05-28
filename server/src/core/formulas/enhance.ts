export const MAX_ENHANCE_LEVEL = 50;

export type EnhanceItemRarity =
  | "None"
  | "Common"
  | "Rare"
  | "Epic"
  | "Unique"
  | "Legendary"
  | "Transcendent"
  | "Mythic";

const BASE_ENHANCE_COST = 100;
const MAX_ENHANCE_SUCCESS_RATE = 0.95;
const MIN_ENHANCE_SUCCESS_RATE = 0.05;
const ENHANCE_SUCCESS_RATE_DECAY_PER_LEVEL = 0.018;

export function getRarityCostMultiplier(rarity: EnhanceItemRarity): number {
  switch (rarity) {
    case "Common":
      return 1;
    case "Rare":
      return 2;
    case "Epic":
      return 4;
    case "Unique":
      return 8;
    case "Legendary":
      return 16;
    case "Transcendent":
      return 32;
    case "Mythic":
      return 64;
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
  return Math.fround(
    Math.max(
      MIN_ENHANCE_SUCCESS_RATE,
      Math.min(
        MAX_ENHANCE_SUCCESS_RATE,
        MAX_ENHANCE_SUCCESS_RATE -
          clampedLevel * ENHANCE_SUCCESS_RATE_DECAY_PER_LEVEL,
      ),
    ),
  );
}

export function rollEnhanceSuccess(
  rate: number,
  rng: () => number = Math.random,
): boolean {
  const clampedRate = Math.max(0, Math.min(rate, 1));
  return rng() < clampedRate;
}
