import { computeRewardMultiplier } from "./stage.js";

export const BOSS_REWARD_BONUS = 8;

export function computeKillExp(
  baseExp: number,
  globalStageIndex: number,
  isBoss: boolean,
): number {
  return scaleKillReward(baseExp, globalStageIndex, isBoss);
}

export function computeKillGold(
  baseGold: number,
  globalStageIndex: number,
  isBoss: boolean,
): number {
  return scaleKillReward(baseGold, globalStageIndex, isBoss);
}

export function getMonsterLevelForStage(globalStageIndex: number): number {
  return 1 + Math.max(0, globalStageIndex);
}

function scaleKillReward(
  baseReward: number,
  globalStageIndex: number,
  isBoss: boolean,
): number {
  const bossMultiplier = isBoss ? BOSS_REWARD_BONUS : 1;
  const scaledReward = Math.round(
    baseReward * computeRewardMultiplier(globalStageIndex) * bossMultiplier,
  );

  return Math.max(0, scaledReward);
}
