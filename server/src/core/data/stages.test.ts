import { describe, expect, it } from "vitest";
import {
  computeGlobalStageIndex,
  getStageWeakElement,
  isBossStage,
} from "../formulas/stage.js";
import {
  CHAPTER_1_STAGE_DEFINITIONS,
  CHAPTER_2_STAGE_DEFINITIONS,
  CHAPTER_3_STAGE_DEFINITIONS,
  getChapter1StageDefinition,
  getChapter2StageDefinition,
  getChapter3StageDefinition,
  STAGES_PER_CHAPTER,
} from "./stages.js";

const expectedKillsToAdvanceByStage = new Map([
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

describe("chapter 1 stage definitions", () => {
  it("defines the ten chapter 1 stages in order", () => {
    expect(STAGES_PER_CHAPTER).toBe(10);
    expect(CHAPTER_1_STAGE_DEFINITIONS).toHaveLength(10);
    expect(
      CHAPTER_1_STAGE_DEFINITIONS.map(({ chapter, stage }) => ({
        chapter,
        stage,
      })),
    ).toEqual(
      Array.from({ length: 10 }, (_, index) => ({
        chapter: 1,
        stage: index + 1,
      })),
    );
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
    expect(getChapter1StageDefinition(10)).toBe(CHAPTER_1_STAGE_DEFINITIONS[9]);
    expect(getChapter1StageDefinition(0)).toBeUndefined();
    expect(getChapter1StageDefinition(11)).toBeUndefined();
  });
});

describe("chapter 2 stage definitions", () => {
  it("defines the ten chapter 2 stages in order", () => {
    expect(STAGES_PER_CHAPTER).toBe(10);
    expect(CHAPTER_2_STAGE_DEFINITIONS).toHaveLength(10);
    expect(
      CHAPTER_2_STAGE_DEFINITIONS.map(({ chapter, stage }) => ({
        chapter,
        stage,
      })),
    ).toEqual(
      Array.from({ length: 10 }, (_, index) => ({
        chapter: 2,
        stage: index + 1,
      })),
    );
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
    expect(getChapter2StageDefinition(10)).toBe(CHAPTER_2_STAGE_DEFINITIONS[9]);
    expect(getChapter2StageDefinition(0)).toBeUndefined();
    expect(getChapter2StageDefinition(11)).toBeUndefined();
  });
});

describe("chapter 3 stage definitions", () => {
  it("defines the ten chapter 3 stages in order", () => {
    expect(CHAPTER_3_STAGE_DEFINITIONS).toHaveLength(10);
    expect(
      CHAPTER_3_STAGE_DEFINITIONS.map(({ chapter, stage }) => ({
        chapter,
        stage,
      })),
    ).toEqual(
      Array.from({ length: 10 }, (_, index) => ({
        chapter: 3,
        stage: index + 1,
      })),
    );
  });

  it("mirrors client kills-to-advance, boss, and weak-element parity", () => {
    for (const definition of CHAPTER_3_STAGE_DEFINITIONS) {
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

  it("looks up chapter 3 stage definitions by stage number", () => {
    expect(getChapter3StageDefinition(1)).toBe(CHAPTER_3_STAGE_DEFINITIONS[0]);
    expect(getChapter3StageDefinition(10)).toBe(CHAPTER_3_STAGE_DEFINITIONS[9]);
    expect(getChapter3StageDefinition(0)).toBeUndefined();
    expect(getChapter3StageDefinition(11)).toBeUndefined();
  });
});
