export const MAX_PET_LEVEL = 10;

export function getFeedCost(currentLevel: number): number {
  if (currentLevel >= MAX_PET_LEVEL) {
    return 0;
  }

  const effectiveLevel = Math.max(0, currentLevel);
  const nextLevel = effectiveLevel + 1;
  return 500 * nextLevel * nextLevel;
}

export function getBonusMultiplier(level: number): number {
  const effectiveLevel = Math.max(0, Math.min(level, MAX_PET_LEVEL));
  return Math.fround(
    Math.fround(1) +
      Math.fround(Math.fround(effectiveLevel) * Math.fround(0.1)),
  );
}

export function getEffectiveBonusPercent(
  basePercent: number,
  level: number,
): number {
  return basePercent * getBonusMultiplier(level);
}
