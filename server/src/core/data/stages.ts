import {
  computeGlobalStageIndex,
  DEFAULT_STAGES_PER_CHAPTER,
  getStageWeakElement,
  isBossStage,
  isEliteStage,
  type StageElement,
} from "../formulas/stage.js";

export const STAGES_PER_CHAPTER = DEFAULT_STAGES_PER_CHAPTER;

export type StageDefinition = {
  chapter: number;
  stage: number;
  killsToAdvance: number;
  boss: boolean;
  elite: boolean;
  weakElement: StageElement;
};

const killsToAdvanceByStage = new Map([
  [1, 5],
  [2, 8],
  [3, 12],
  [4, 16],
  [5, 1],
  [6, 20],
  [7, 24],
  [8, 28],
  [9, 32],
  [10, 1],
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
    elite: isEliteStage(stage),
    weakElement: getStageWeakElement(globalStageIndex),
  };
}

function defineChapter1Stage(stage: number): StageDefinition {
  return defineChapterStage(1, stage);
}

function defineChapter2Stage(stage: number): StageDefinition {
  return defineChapterStage(2, stage);
}

function defineChapter3Stage(stage: number): StageDefinition {
  return defineChapterStage(3, stage);
}

function defineChapter4Stage(stage: number): StageDefinition {
  return defineChapterStage(4, stage);
}

function defineChapter5Stage(stage: number): StageDefinition {
  return defineChapterStage(5, stage);
}

function defineChapter6Stage(stage: number): StageDefinition {
  return defineChapterStage(6, stage);
}

export const CHAPTER_1_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter1Stage(index + 1),
  ),
];

export const CHAPTER_2_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter2Stage(index + 1),
  ),
];

export const CHAPTER_3_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter3Stage(index + 1),
  ),
];

export const CHAPTER_4_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter4Stage(index + 1),
  ),
];

export const CHAPTER_5_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter5Stage(index + 1),
  ),
];

export const CHAPTER_6_STAGE_DEFINITIONS: StageDefinition[] = [
  ...Array.from({ length: STAGES_PER_CHAPTER }, (_, index) =>
    defineChapter6Stage(index + 1),
  ),
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

export function getChapter3StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_3_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}

export function getChapter4StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_4_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}

export function getChapter5StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_5_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}

export function getChapter6StageDefinition(
  stage: number,
): StageDefinition | undefined {
  return CHAPTER_6_STAGE_DEFINITIONS.find(
    (definition) => definition.stage === stage,
  );
}
