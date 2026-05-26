import { describe, expect, it } from "vitest";
import type { SkillElement } from "./combat.js";
import {
  computeGlobalStageIndex,
  computeMonsterStatMultiplier,
  computeRewardMultiplier,
  getStageWeakElement,
  isBossStage,
  type StageElement,
} from "./stage.js";

describe("stage formulas", () => {
  it.each([
    { globalStageIndex: -1, expectedMultiplier: 1 },
    { globalStageIndex: 0, expectedMultiplier: 1 },
    { globalStageIndex: 1, expectedMultiplier: 1.149999976158142 },
    { globalStageIndex: 2, expectedMultiplier: 1.2999999523162842 },
    { globalStageIndex: 3, expectedMultiplier: 1.4500000476837158 },
    { globalStageIndex: 4, expectedMultiplier: 1.600000023841858 },
    { globalStageIndex: 5, expectedMultiplier: 1.75 },
    { globalStageIndex: 6, expectedMultiplier: 1.9000000953674316 },
    { globalStageIndex: 7, expectedMultiplier: 2.0500001907348633 },
    { globalStageIndex: 8, expectedMultiplier: 2.200000047683716 },
    { globalStageIndex: 9, expectedMultiplier: 2.3499999046325684 },
  ])("mirrors client monster stat multiplier for global stage index $globalStageIndex", ({
    globalStageIndex,
    expectedMultiplier,
  }) => {
    expect(computeMonsterStatMultiplier(globalStageIndex)).toBe(
      expectedMultiplier,
    );
  });

  it("uses the monster stat multiplier as the reward multiplier", () => {
    for (const globalStageIndex of [0, 1, 2, 3, 4, 8]) {
      expect(computeRewardMultiplier(globalStageIndex)).toBe(
        computeMonsterStatMultiplier(globalStageIndex),
      );
    }
  });

  it.each([
    { chapter: 1, stage: 5, stagesPerChapter: 5, expectedBoss: true },
    { chapter: 1, stage: 4, stagesPerChapter: 5, expectedBoss: false },
    { chapter: 2, stage: 5, stagesPerChapter: 5, expectedBoss: true },
    { chapter: 2, stage: 4, stagesPerChapter: 5, expectedBoss: false },
    { chapter: 0, stage: 5, stagesPerChapter: 5, expectedBoss: false },
    { chapter: 1, stage: 5, stagesPerChapter: 0, expectedBoss: false },
  ])("mirrors client boss stage predicate for chapter $chapter stage $stage", ({
    chapter,
    stage,
    stagesPerChapter,
    expectedBoss,
  }) => {
    expect(isBossStage(chapter, stage, stagesPerChapter)).toBe(expectedBoss);
  });

  it.each([
    { globalStageIndex: 0, expectedWeakElement: "None" },
    { globalStageIndex: 1, expectedWeakElement: "None" },
    { globalStageIndex: 2, expectedWeakElement: "Ice" },
    { globalStageIndex: 3, expectedWeakElement: "Holy" },
    { globalStageIndex: 4, expectedWeakElement: "Fire" },
    { globalStageIndex: 5, expectedWeakElement: "None" },
    { globalStageIndex: 6, expectedWeakElement: "Lightning" },
    { globalStageIndex: 7, expectedWeakElement: "Ice" },
    { globalStageIndex: 8, expectedWeakElement: "Fire" },
    { globalStageIndex: 9, expectedWeakElement: "Holy" },
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
    { chapter: 0, stage: 0, expectedGlobalStageIndex: 0 },
    { chapter: 1, stage: 1, expectedGlobalStageIndex: 0 },
    { chapter: 1, stage: 5, expectedGlobalStageIndex: 4 },
    { chapter: 2, stage: 1, expectedGlobalStageIndex: 5 },
    { chapter: 3, stage: 4, expectedGlobalStageIndex: 13 },
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
