import { describe, expect, it } from "vitest";
import {
  getTowerMilestoneMultiplier,
  TOWER_MILESTONE_BONUS_PER_STEP,
  TOWER_MILESTONE_STEP,
} from "./index.js";

describe("tower milestone formulas", () => {
  it("mirrors the client milestone constants", () => {
    expect(TOWER_MILESTONE_STEP).toBe(10);
    expect(TOWER_MILESTONE_BONUS_PER_STEP).toBe(0.02);
  });

  it.each([
    [0, Math.fround(1)],
    [9, Math.fround(1)],
    [10, Math.fround(1.02)],
    [25, Math.fround(1.04)],
    [100, Math.fround(1.2)],
  ])("matches FTowerMilestoneFormula multiplier for highestFloor=%i", (highestFloor, expected) => {
    expect(getTowerMilestoneMultiplier(highestFloor)).toBe(expected);
  });

  it("clamps negative highest floors like the client formula", () => {
    expect(getTowerMilestoneMultiplier(-1)).toBe(Math.fround(1));
  });
});
