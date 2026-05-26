import { describe, expect, it } from "vitest";
import { getGearRollCost } from "./index.js";
import { getGearRollCost as getGearRollCostDirect } from "./shop.js";

describe("shop formulas", () => {
  it.each([
    { globalStageIndex: 0, expectedCost: 300 },
    { globalStageIndex: 4, expectedCost: 480 },
    { globalStageIndex: 9, expectedCost: 705 },
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
});
