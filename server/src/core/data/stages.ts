import {
  computeGlobalStageIndex,
  DEFAULT_STAGES_PER_CHAPTER,
  getStageWeakElement,
  isBossStage,
  type StageElement,
} from "../formulas/stage.js";

export const STAGES_PER_CHAPTER = DEFAULT_STAGES_PER_CHAPTER;

export type StageDefinition = {
  chapter: number;
  stage: number;
  killsToAdvance: number;
  boss: boolean;
  weakElement: StageElement;
};

const killsToAdvanceByStage = new Map([
  [1, 5],
  [2, 8],
  [3, 12],
  [4, 16],
  [5, 1],
]);

function defineChapterStage(chapter: number, stage: number): StageDefinition {
  const globalStageIndex = computeGlobalStageIndex(
    chapter,
    stage,
    STAGES_PER_CHAPTER,
  );

  return {
    chapter,
    stage,
    killsToAdvance: killsToAdvanceByStage.get(stage) ?? 0,
    boss: isBossStage(chapter, stage, STAGES_PER_CHAPTER),
    weakElement: getStageWeakElement(globalStageIndex),
  };
}

function defineChapter1Stage(stage: number): StageDefinition {
  return defineChapterStage(1, stage);
}

function defineChapter2Stage(stage: number): StageDefinition {
  return defineChapterStage(2, stage);
}

export const CHAPTER_1_STAGE_DEFINITIONS: StageDefinition[] = [
  defineChapter1Stage(1),
  defineChapter1Stage(2),
  defineChapter1Stage(3),
  defineChapter1Stage(4),
  defineChapter1Stage(5),
];

export const CHAPTER_2_STAGE_DEFINITIONS: StageDefinition[] = [
  defineChapter2Stage(1),
  defineChapter2Stage(2),
  defineChapter2Stage(3),
  defineChapter2Stage(4),
  defineChapter2Stage(5),
];

export function getChapter1StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_1_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}

export function getChapter2StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_2_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}
