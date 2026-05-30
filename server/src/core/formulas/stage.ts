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
    case 41:
      return "Dark";
    case 42:
      return "Fire";
    case 43:
      return "Lightning";
    case 44:
      return "Holy";
    case 45:
      return "Dark";
    case 46:
      return "Ice";
    case 47:
      return "Dark";
    case 48:
      return "Lightning";
    case 49:
      return "Holy";
    case 50:
      return "Dark";
    case 51:
      return "Dark";
    case 52:
      return "Lightning";
    case 53:
      return "Ice";
    case 54:
      return "Holy";
    case 55:
      return "Dark";
    case 56:
      return "Fire";
    case 57:
      return "Dark";
    case 58:
      return "Ice";
    case 59:
      return "Holy";
    case 60:
      return "Dark";
    case 61:
      return "Dark";
    case 62:
      return "Fire";
    case 63:
      return "Holy";
    case 64:
      return "Lightning";
    case 65:
      return "Dark";
    case 66:
      return "Ice";
    case 67:
      return "Dark";
    case 68:
      return "Fire";
    case 69:
      return "Holy";
    case 70:
      return "Dark";
    case 71:
      return "Dark";
    case 72:
      return "Lightning";
    case 73:
      return "Holy";
    case 74:
      return "Fire";
    case 75:
      return "Dark";
    case 76:
      return "Ice";
    case 77:
      return "Dark";
    case 78:
      return "Holy";
    case 79:
      return "Fire";
    case 80:
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
