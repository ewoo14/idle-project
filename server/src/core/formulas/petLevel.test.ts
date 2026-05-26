import { describe, expect, it } from "vitest";
import {
  getBonusMultiplier,
  getEffectiveBonusPercent,
  getFeedCost,
  MAX_PET_LEVEL,
} from "./petLevel.js";

describe("pet level formulas", () => {
  it("matches the UE5 max pet level", () => {
    expect(MAX_PET_LEVEL).toBe(10);
  });

  it.each([
    [0, 500],
    [1, 2000],
    [2, 4500],
    [9, 50000],
    [10, 0],
    [11, 0],
  ])("computes feed cost for current level %i", (currentLevel, expected) => {
    expect(getFeedCost(currentLevel)).toBe(expected);
  });

  it("clamps negative feed cost levels to level zero", () => {
    expect(getFeedCost(-1)).toBe(500);
  });

  it.each([
    [0, 1],
    [1, 1.100000023841858],
    [5, 1.5],
    [10, 2],
    [11, 2],
  ])("mirrors client bonus multiplier for level %i", (level, expected) => {
    expect(getBonusMultiplier(level)).toBe(expected);
  });

  it("clamps negative bonus levels to level zero", () => {
    expect(getBonusMultiplier(-1)).toBe(1);
  });

  it.each([
    { basePercent: 20, level: 10, expectedPercent: 40 },
    { basePercent: 15, level: 10, expectedPercent: 30 },
  ])("computes effective pet bonus percent", ({
    basePercent,
    level,
    expectedPercent,
  }) => {
    expect(getEffectiveBonusPercent(basePercent, level)).toBe(expectedPercent);
  });
});
