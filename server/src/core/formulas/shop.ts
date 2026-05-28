import { computeRewardMultiplier } from "./stage.js";

function getScaledShopCost(globalStageIndex: number, baseCost: number): number {
  const safeIndex = Math.max(0, globalStageIndex);
  const scaledCost = baseCost * computeRewardMultiplier(safeIndex);

  return Math.max(1, Math.round(scaledCost));
}

export function getGearRollCost(globalStageIndex: number): number {
  return getScaledShopCost(globalStageIndex, 300);
}

export function getProtectionScrollCost(globalStageIndex: number): number {
  return getScaledShopCost(globalStageIndex, 300);
}

export function getResetCubeCost(globalStageIndex: number): number {
  return getScaledShopCost(globalStageIndex, 800);
}

export function getRankCubeCost(globalStageIndex: number): number {
  return getScaledShopCost(globalStageIndex, 4000);
}
