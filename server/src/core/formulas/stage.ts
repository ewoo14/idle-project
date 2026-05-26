import type { SkillElement } from "./combat.js";

export type StageElement = SkillElement;

export const DEFAULT_STAGES_PER_CHAPTER = 5;

export function computeMonsterStatMultiplier(globalStageIndex: number): number {
  const clampedIndex = Math.max(0, globalStageIndex);
  return Math.fround(
    Math.fround(1) + Math.fround(Math.fround(clampedIndex) * Math.fround(0.15)),
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

export function getStageWeakElement(globalStageIndex: number): StageElement {
  switch (Math.max(0, globalStageIndex)) {
    case 2:
      return "Ice";
    case 3:
      return "Holy";
    case 4:
      return "Fire";
    default:
      return "None";
  }
}

export function computeGlobalStageIndex(
  chapter: number,
  stage: number,
  stagesPerChapter = DEFAULT_STAGES_PER_CHAPTER,
): number {
  return (
    (Math.max(1, chapter) - 1) * stagesPerChapter + (Math.max(1, stage) - 1)
  );
}
