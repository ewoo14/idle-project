const BASE_REBIRTH_POINTS = 5;
const POINTS_PER_REBIRTH = 2;
const MIN_REBIRTH_LEVEL = 100;
const LEVEL_STEP = 10;

export function getRebirthPointsReward(
  rebirthCount: number,
  levelAtRebirth: number,
): number {
  const safeCount = Math.max(0, rebirthCount);
  const safeLevel = Math.max(MIN_REBIRTH_LEVEL, levelAtRebirth);
  const levelBonus = Math.floor((safeLevel - MIN_REBIRTH_LEVEL) / LEVEL_STEP);

  return BASE_REBIRTH_POINTS + safeCount * POINTS_PER_REBIRTH + levelBonus;
}
