export const TOWER_MILESTONE_STEP = 10;
export const TOWER_MILESTONE_BONUS_PER_STEP = 0.02;

export function getTowerMilestoneMultiplier(highestFloor: number): number {
  const safeHighestFloor = Math.max(0, highestFloor);
  const milestoneCount = Math.floor(safeHighestFloor / TOWER_MILESTONE_STEP);

  return Math.fround(
    Math.fround(1) +
      Math.fround(milestoneCount * Math.fround(TOWER_MILESTONE_BONUS_PER_STEP)),
  );
}
