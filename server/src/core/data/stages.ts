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

function defineChapter1Stage(stage: number): StageDefinition {
  const chapter = 1;
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

export const CHAPTER_1_STAGE_DEFINITIONS: StageDefinition[] = [
  defineChapter1Stage(1),
  defineChapter1Stage(2),
  defineChapter1Stage(3),
  defineChapter1Stage(4),
  defineChapter1Stage(5),
];

export function getChapter1StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_1_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}
