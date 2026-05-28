import { computeRewardMultiplier } from "./stage.js";

export const BOSS_REWARD_BONUS = 8;
export const ELITE_REWARD_BONUS = 3;

export function computeKillExp(
  baseExp: number,
  globalStageIndex: number,
  isBoss: boolean,
  isElite = false,
): number {
  return scaleKillReward(baseExp, globalStageIndex, isBoss, isElite);
}

export function computeKillGold(
  baseGold: number,
  globalStageIndex: number,
  isBoss: boolean,
  isElite = false,
): number {
  return scaleKillReward(baseGold, globalStageIndex, isBoss, isElite);
}

export function getMonsterLevelForStage(globalStageIndex: number): number {
  return 1 + Math.max(0, globalStageIndex - 1);
}

function scaleKillReward(
  baseReward: number,
  globalStageIndex: number,
  isBoss: boolean,
  isElite: boolean,
): number {
  const stageTypeMultiplier = isBoss
    ? BOSS_REWARD_BONUS
    : isElite
      ? ELITE_REWARD_BONUS
      : 1;
  const scaledReward = Math.round(
    baseReward *
      computeRewardMultiplier(globalStageIndex) *
      stageTypeMultiplier,
  );

  return Math.max(0, scaledReward);
}
