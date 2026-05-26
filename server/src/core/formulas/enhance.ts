export const MAX_ENHANCE_LEVEL = 5;

const BASE_ENHANCE_COST = 100;
const ENHANCE_SUCCESS_RATES = [0.95, 0.85, 0.7, 0.55, 0.4] as const;

export function getEnhanceCost(currentLevel: number): number {
  if (currentLevel >= MAX_ENHANCE_LEVEL) {
    return 0;
  }

  const clampedLevel = Math.max(0, currentLevel);
  return BASE_ENHANCE_COST * (clampedLevel + 1) ** 2;
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
