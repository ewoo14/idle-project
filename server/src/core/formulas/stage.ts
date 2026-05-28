import type { SkillElement } from "./combat.js";

export type StageElement = SkillElement;

export const DEFAULT_STAGES_PER_CHAPTER = 10;

export function computeMonsterStatMultiplier(globalStageIndex: number): number {
  const progressionStep = Math.max(0, globalStageIndex - 1);
  return Math.fround(
    Math.fround(1) +
      Math.fround(Math.fround(progressionStep) * Math.fround(0.15)),
  );
}

export function computeRewardMultiplier(globalStageIndex: number): number {
  return computeMonsterStatMultiplier(globalStageIndex);
}

export function isBossStage(
  chapter: number,
  stage: number,
  stagesPerChapter = DEFAULT_STAGES_PER_CHAPTER,
): boolean {
  return chapter > 0 && stagesPerChapter > 0 && stage === stagesPerChapter;
}

export function isEliteStage(stage: number): boolean {
  return stage === 5;
}

export function getStageWeakElement(globalStageIndex: number): StageElement {
  switch (Math.max(0, globalStageIndex)) {
    case 1:
      return "Fire";
    case 2:
      return "Ice";
    case 3:
      return "Lightning";
    case 4:
      return "Holy";
    case 5:
      return "Dark";
    case 6:
      return "Fire";
    case 7:
      return "Ice";
    case 8:
      return "Lightning";
    case 9:
      return "Holy";
    case 10:
      return "Dark";
    case 11:
      return "Ice";
    case 12:
      return "Fire";
    case 13:
      return "Holy";
    case 14:
      return "Lightning";
    case 15:
      return "Dark";
    case 16:
      return "Ice";
    case 17:
      return "Fire";
    case 18:
      return "Holy";
    case 19:
      return "Lightning";
    case 20:
      return "Dark";
    case 21:
      return "Dark";
    case 22:
      return "Holy";
    case 23:
      return "Dark";
    case 24:
      return "Lightning";
    case 25:
      return "Dark";
    case 26:
      return "Fire";
    case 27:
      return "Dark";
    case 28:
      return "Ice";
    case 29:
      return "Dark";
    case 30:
      return "Holy";
    case 31:
      return "Lightning";
    case 32:
      return "Holy";
    case 33:
      return "Ice";
    case 34:
      return "Fire";
    case 35:
      return "Dark";
    case 36:
      return "Lightning";
    case 37:
      return "Holy";
    case 38:
      return "Ice";
    case 39:
      return "Fire";
    case 40:
      return "Dark";
    default:
      return "None";
  }
}

export function computeGlobalStageIndex(
  chapter: number,
  stage: number,
  stagesPerChapter = DEFAULT_STAGES_PER_CHAPTER,
): number {
  return (Math.max(1, chapter) - 1) * stagesPerChapter + Math.max(1, stage);
}
