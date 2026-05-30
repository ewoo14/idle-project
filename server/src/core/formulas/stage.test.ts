import { describe, expect, it } from "vitest";
import type { SkillElement } from "./combat.js";
import {
  computeGlobalStageIndex,
  computeMonsterStatMultiplier,
  computeRewardMultiplier,
  DEFAULT_STAGES_PER_CHAPTER,
  getStageResistElement,
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
    { globalStageIndex: 31, expectedWeakElement: "Lightning" },
    { globalStageIndex: 32, expectedWeakElement: "Holy" },
    { globalStageIndex: 33, expectedWeakElement: "Ice" },
    { globalStageIndex: 34, expectedWeakElement: "Fire" },
    { globalStageIndex: 35, expectedWeakElement: "Dark" },
    { globalStageIndex: 36, expectedWeakElement: "Lightning" },
    { globalStageIndex: 37, expectedWeakElement: "Holy" },
    { globalStageIndex: 38, expectedWeakElement: "Ice" },
    { globalStageIndex: 39, expectedWeakElement: "Fire" },
    { globalStageIndex: 40, expectedWeakElement: "Dark" },
    { globalStageIndex: 41, expectedWeakElement: "Dark" },
    { globalStageIndex: 42, expectedWeakElement: "Fire" },
    { globalStageIndex: 43, expectedWeakElement: "Lightning" },
    { globalStageIndex: 44, expectedWeakElement: "Holy" },
    { globalStageIndex: 45, expectedWeakElement: "Dark" },
    { globalStageIndex: 46, expectedWeakElement: "Ice" },
    { globalStageIndex: 47, expectedWeakElement: "Dark" },
    { globalStageIndex: 48, expectedWeakElement: "Lightning" },
    { globalStageIndex: 49, expectedWeakElement: "Holy" },
    { globalStageIndex: 50, expectedWeakElement: "Dark" },
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

  it("챕터8 약점 71~80 (None 없음, Dark 가중)", () => {
    const ch8 = [71, 72, 73, 74, 75, 76, 77, 78, 79, 80].map(
      getStageWeakElement,
    );
    expect(ch8).toEqual([
      "Dark",
      "Lightning",
      "Holy",
      "Fire",
      "Dark",
      "Ice",
      "Dark",
      "Holy",
      "Fire",
      "Dark",
    ]);
    expect(ch8).not.toContain("None");
  });

  it("defines chapter 9 weak and resist elements (81~90, weak != resist)", () => {
    const weak: Record<number, string> = {
      81: "Dark",
      82: "Fire",
      83: "Lightning",
      84: "Holy",
      85: "Dark",
      86: "Ice",
      87: "Dark",
      88: "Holy",
      89: "Fire",
      90: "Dark",
    };
    const resist: Record<number, string> = {
      81: "Ice",
      82: "Holy",
      83: "Dark",
      84: "Fire",
      85: "Holy",
      86: "Lightning",
      87: "Fire",
      88: "Lightning",
      89: "Holy",
      90: "Ice",
    };
    for (let i = 81; i <= 90; i++) {
      expect(getStageWeakElement(i)).toBe(weak[i]);
      expect(getStageResistElement(i)).toBe(resist[i]);
      expect(getStageWeakElement(i)).not.toBe(getStageResistElement(i));
    }
    expect(getStageResistElement(80)).toBe("None");
    expect(getStageResistElement(1)).toBe("None");
  });

  it.each([
    { chapter: 0, stage: 0, expectedGlobalStageIndex: 1 },
    { chapter: 1, stage: 1, expectedGlobalStageIndex: 1 },
    { chapter: 1, stage: 10, expectedGlobalStageIndex: 10 },
    { chapter: 2, stage: 1, expectedGlobalStageIndex: 11 },
    { chapter: 3, stage: 10, expectedGlobalStageIndex: 30 },
    { chapter: 4, stage: 1, expectedGlobalStageIndex: 31 },
    { chapter: 4, stage: 10, expectedGlobalStageIndex: 40 },
    { chapter: 5, stage: 1, expectedGlobalStageIndex: 41 },
    { chapter: 5, stage: 10, expectedGlobalStageIndex: 50 },
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
