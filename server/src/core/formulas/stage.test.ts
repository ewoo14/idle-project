import { describe, expect, it } from "vitest";
import type { SkillElement } from "./combat.js";
import {
  computeGlobalStageIndex,
  computeMonsterStatMultiplier,
  computeRewardMultiplier,
  DEFAULT_STAGES_PER_CHAPTER,
  getStageWeakElement,
  isBossStage,
  isEliteStage,
  type StageElement,
} from "./stage.js";

describe("stage formulas", () => {
  it.each([
    { globalStageIndex: -1, expectedMultiplier: 1 },
    { globalStageIndex: 0, expectedMultiplier: 1 },
    { globalStageIndex: 1, expectedMultiplier: 1 },
    { globalStageIndex: 2, expectedMultiplier: 1.149999976158142 },
    { globalStageIndex: 3, expectedMultiplier: 1.2999999523162842 },
    { globalStageIndex: 4, expectedMultiplier: 1.4500000476837158 },
    { globalStageIndex: 5, expectedMultiplier: 1.600000023841858 },
    { globalStageIndex: 6, expectedMultiplier: 1.75 },
    { globalStageIndex: 7, expectedMultiplier: 1.9000000953674316 },
    { globalStageIndex: 8, expectedMultiplier: 2.0500001907348633 },
    { globalStageIndex: 9, expectedMultiplier: 2.200000047683716 },
    { globalStageIndex: 10, expectedMultiplier: 2.3499999046325684 },
  ])("mirrors client monster stat multiplier for global stage index $globalStageIndex", ({
    globalStageIndex,
    expectedMultiplier,
  }) => {
    expect(computeMonsterStatMultiplier(globalStageIndex)).toBe(
      expectedMultiplier,
    );
  });

  it("uses the monster stat multiplier as the reward multiplier", () => {
    for (const globalStageIndex of [1, 2, 3, 4, 10, 30]) {
      expect(computeRewardMultiplier(globalStageIndex)).toBe(
        computeMonsterStatMultiplier(globalStageIndex),
      );
    }
  });

  it("uses ten stages per chapter by default", () => {
    expect(DEFAULT_STAGES_PER_CHAPTER).toBe(10);
  });

  it.each([
    { chapter: 1, stage: 10, stagesPerChapter: 10, expectedBoss: true },
    { chapter: 1, stage: 5, stagesPerChapter: 10, expectedBoss: false },
    { chapter: 2, stage: 10, stagesPerChapter: 10, expectedBoss: true },
    { chapter: 3, stage: 10, stagesPerChapter: 10, expectedBoss: true },
    { chapter: 0, stage: 10, stagesPerChapter: 10, expectedBoss: false },
    { chapter: 1, stage: 10, stagesPerChapter: 0, expectedBoss: false },
  ])("mirrors client boss stage predicate for chapter $chapter stage $stage", ({
    chapter,
    stage,
    stagesPerChapter,
    expectedBoss,
  }) => {
    expect(isBossStage(chapter, stage, stagesPerChapter)).toBe(expectedBoss);
  });

  it.each([
    { stage: 5, expectedElite: true },
    { stage: 10, expectedElite: false },
    { stage: 4, expectedElite: false },
    { stage: 0, expectedElite: false },
  ])("mirrors client elite stage predicate for stage $stage", ({
    stage,
    expectedElite,
  }) => {
    expect(isEliteStage(stage)).toBe(expectedElite);
  });

  it.each([
    { globalStageIndex: 1, expectedWeakElement: "Fire" },
    { globalStageIndex: 2, expectedWeakElement: "Ice" },
    { globalStageIndex: 3, expectedWeakElement: "Lightning" },
    { globalStageIndex: 4, expectedWeakElement: "Holy" },
    { globalStageIndex: 5, expectedWeakElement: "Dark" },
    { globalStageIndex: 10, expectedWeakElement: "Dark" },
    { globalStageIndex: 11, expectedWeakElement: "Ice" },
    { globalStageIndex: 15, expectedWeakElement: "Dark" },
    { globalStageIndex: 20, expectedWeakElement: "Dark" },
    { globalStageIndex: 21, expectedWeakElement: "Dark" },
    { globalStageIndex: 25, expectedWeakElement: "Dark" },
    { globalStageIndex: 30, expectedWeakElement: "Holy" },
  ] satisfies Array<{
    globalStageIndex: number;
    expectedWeakElement: SkillElement;
  }>)("mirrors client weak element for global stage index $globalStageIndex", ({
    globalStageIndex,
    expectedWeakElement,
  }) => {
    const weakElement: StageElement = getStageWeakElement(globalStageIndex);

    expect(weakElement).toBe(expectedWeakElement);
  });

  it.each([
    { chapter: 0, stage: 0, expectedGlobalStageIndex: 1 },
    { chapter: 1, stage: 1, expectedGlobalStageIndex: 1 },
    { chapter: 1, stage: 10, expectedGlobalStageIndex: 10 },
    { chapter: 2, stage: 1, expectedGlobalStageIndex: 11 },
    { chapter: 3, stage: 10, expectedGlobalStageIndex: 30 },
  ])("computes global stage index from chapter and stage", ({
    chapter,
    stage,
    expectedGlobalStageIndex,
  }) => {
    expect(computeGlobalStageIndex(chapter, stage)).toBe(
      expectedGlobalStageIndex,
    );
  });
});
