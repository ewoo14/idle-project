import { describe, expect, it } from "vitest";
import {
  computeGlobalStageIndex,
  getStageWeakElement,
  isBossStage,
} from "../formulas/stage.js";
import {
  CHAPTER_1_STAGE_DEFINITIONS,
  CHAPTER_2_STAGE_DEFINITIONS,
  getChapter1StageDefinition,
  getChapter2StageDefinition,
  STAGES_PER_CHAPTER,
} from "./stages.js";

const expectedKillsToAdvanceByStage = new Map([
  [1, 5],
  [2, 8],
  [3, 12],
  [4, 16],
  [5, 1],
]);

describe("chapter 1 stage definitions", () => {
  it("defines the five V1 chapter 1 stages in order", () => {
    expect(STAGES_PER_CHAPTER).toBe(5);
    expect(CHAPTER_1_STAGE_DEFINITIONS).toHaveLength(5);
    expect(
      CHAPTER_1_STAGE_DEFINITIONS.map(({ chapter, stage }) => ({
        chapter,
        stage,
      })),
    ).toEqual([
      { chapter: 1, stage: 1 },
      { chapter: 1, stage: 2 },
      { chapter: 1, stage: 3 },
      { chapter: 1, stage: 4 },
      { chapter: 1, stage: 5 },
    ]);
  });

  it("mirrors client kills-to-advance, boss, and weak-element parity", () => {
    for (const definition of CHAPTER_1_STAGE_DEFINITIONS) {
      const globalStageIndex = computeGlobalStageIndex(
        definition.chapter,
        definition.stage,
        STAGES_PER_CHAPTER,
      );

      expect(definition.killsToAdvance).toBe(
        expectedKillsToAdvanceByStage.get(definition.stage),
      );
      expect(definition.boss).toBe(
        isBossStage(definition.chapter, definition.stage, STAGES_PER_CHAPTER),
      );
      expect(definition.weakElement).toBe(
        getStageWeakElement(globalStageIndex),
      );
    }
  });

  it("looks up chapter 1 stage definitions by stage number", () => {
    expect(getChapter1StageDefinition(1)).toBe(CHAPTER_1_STAGE_DEFINITIONS[0]);
    expect(getChapter1StageDefinition(5)).toBe(CHAPTER_1_STAGE_DEFINITIONS[4]);
    expect(getChapter1StageDefinition(0)).toBeUndefined();
    expect(getChapter1StageDefinition(6)).toBeUndefined();
  });
});

describe("chapter 2 stage definitions", () => {
  it("defines the five V1 chapter 2 stages in order", () => {
    expect(STAGES_PER_CHAPTER).toBe(5);
    expect(CHAPTER_2_STAGE_DEFINITIONS).toHaveLength(5);
    expect(
      CHAPTER_2_STAGE_DEFINITIONS.map(({ chapter, stage }) => ({
        chapter,
        stage,
      })),
    ).toEqual([
      { chapter: 2, stage: 1 },
      { chapter: 2, stage: 2 },
      { chapter: 2, stage: 3 },
      { chapter: 2, stage: 4 },
      { chapter: 2, stage: 5 },
    ]);
  });

  it("mirrors client kills-to-advance, boss, and weak-element parity", () => {
    for (const definition of CHAPTER_2_STAGE_DEFINITIONS) {
      const globalStageIndex = computeGlobalStageIndex(
        definition.chapter,
        definition.stage,
        STAGES_PER_CHAPTER,
      );

      expect(definition.killsToAdvance).toBe(
        expectedKillsToAdvanceByStage.get(definition.stage),
      );
      expect(definition.boss).toBe(
        isBossStage(definition.chapter, definition.stage, STAGES_PER_CHAPTER),
      );
      expect(definition.weakElement).toBe(
        getStageWeakElement(globalStageIndex),
      );
    }
  });

  it("looks up chapter 2 stage definitions by stage number", () => {
    expect(getChapter2StageDefinition(1)).toBe(CHAPTER_2_STAGE_DEFINITIONS[0]);
    expect(getChapter2StageDefinition(5)).toBe(CHAPTER_2_STAGE_DEFINITIONS[4]);
    expect(getChapter2StageDefinition(0)).toBeUndefined();
    expect(getChapter2StageDefinition(6)).toBeUndefined();
  });
});
