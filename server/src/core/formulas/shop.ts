import { computeRewardMultiplier } from "./stage.js";

export function getGearRollCost(globalStageIndex: number): number {
  const safeIndex = Math.max(0, globalStageIndex);
  const scaledCost = 300 * computeRewardMultiplier(safeIndex);

  return Math.max(1, Math.round(scaledCost));
}
