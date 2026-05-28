import { describe, expect, it } from "vitest";
import {
  getGearRollCost,
  getProtectionScrollCost,
  getRankCubeCost,
  getResetCubeCost,
} from "./index.js";
import {
  getGearRollCost as getGearRollCostDirect,
  getProtectionScrollCost as getProtectionScrollCostDirect,
  getRankCubeCost as getRankCubeCostDirect,
  getResetCubeCost as getResetCubeCostDirect,
} from "./shop.js";

describe("shop formulas", () => {
  it.each([
    { globalStageIndex: 0, expectedCost: 300 },
    { globalStageIndex: 1, expectedCost: 300 },
    { globalStageIndex: 4, expectedCost: 435 },
    { globalStageIndex: 5, expectedCost: 480 },
    { globalStageIndex: 9, expectedCost: 660 },
    { globalStageIndex: 10, expectedCost: 705 },
  ])("mirrors client gear roll cost for global stage index $globalStageIndex", ({
    globalStageIndex,
    expectedCost,
  }) => {
    expect(getGearRollCostDirect(globalStageIndex)).toBe(expectedCost);
    expect(getGearRollCost(globalStageIndex)).toBe(expectedCost);
  });

  it("clamps negative stage indexes before scaling", () => {
    expect(getGearRollCostDirect(-1)).toBe(300);
  });

  it("keeps gear roll cost at least one", () => {
    expect(getGearRollCostDirect(Number.NEGATIVE_INFINITY)).toBe(300);
  });

  it.each([
    {
      globalStageIndex: 0,
      protectionScrollCost: 300,
      resetCubeCost: 800,
      rankCubeCost: 4000,
    },
    {
      globalStageIndex: 4,
      protectionScrollCost: 435,
      resetCubeCost: 1160,
      rankCubeCost: 5800,
    },
    {
      globalStageIndex: 9,
      protectionScrollCost: 660,
      resetCubeCost: 1760,
      rankCubeCost: 8800,
    },
  ])("mirrors client material shop costs for global stage index $globalStageIndex", ({
    globalStageIndex,
    protectionScrollCost,
    resetCubeCost,
    rankCubeCost,
  }) => {
    expect(getProtectionScrollCostDirect(globalStageIndex)).toBe(
      protectionScrollCost,
    );
    expect(getProtectionScrollCost(globalStageIndex)).toBe(
      protectionScrollCost,
    );
    expect(getResetCubeCostDirect(globalStageIndex)).toBe(resetCubeCost);
    expect(getResetCubeCost(globalStageIndex)).toBe(resetCubeCost);
    expect(getRankCubeCostDirect(globalStageIndex)).toBe(rankCubeCost);
    expect(getRankCubeCost(globalStageIndex)).toBe(rankCubeCost);
  });
});
