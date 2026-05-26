import { describe, expect, it } from "vitest";
import {
  getEnhanceCost,
  getEnhanceSuccessRate,
  getRarityCostMultiplier,
  MAX_ENHANCE_LEVEL,
  rollEnhanceSuccess,
} from "./enhance.js";

describe("enhance formulas", () => {
  it("matches the UE5 max enhance level", () => {
    expect(MAX_ENHANCE_LEVEL).toBe(50);
  });

  it.each([
    [0, 100],
    [1, 400],
    [2, 900],
    [3, 1600],
    [4, 2500],
    [5, 3600],
    [49, 250000],
    [50, 0],
    [51, 0],
  ])("computes enhance cost for current level %i", (currentLevel, expected) => {
    expect(getEnhanceCost(currentLevel)).toBe(expected);
  });

  it("clamps negative levels to the level zero cost", () => {
    expect(getEnhanceCost(-1)).toBe(100);
  });

  it.each([
    ["None", 0],
    ["Common", 1],
    ["Uncommon", 2],
    ["Rare", 4],
    ["Epic", 8],
    ["Legendary", 16],
  ] as const)("computes %s enhance cost multiplier", (rarity, expected) => {
    expect(getRarityCostMultiplier(rarity)).toBe(expected);
  });

  it("applies rarity multiplier while keeping Common cost compatible", () => {
    expect(getEnhanceCost(1, "Common")).toBe(400);
    expect(getEnhanceCost(1, "Rare")).toBe(1600);
    expect(getEnhanceCost(0, "Legendary")).toBe(1600);
    expect(getEnhanceCost(MAX_ENHANCE_LEVEL, "Legendary")).toBe(0);
  });

  it.each([
    ["Common", [100, 400, 900, 1600, 2500, 0]],
    ["Uncommon", [200, 800, 1800, 3200, 5000, 0]],
    ["Rare", [400, 1600, 3600, 6400, 10000, 0]],
    ["Epic", [800, 3200, 7200, 12800, 20000, 0]],
    ["Legendary", [1600, 6400, 14400, 25600, 40000, 0]],
  ] as const)("keeps the old +0 to +4 rarity-scaled cost table for %s", (rarity, expectedCosts) => {
    for (const [currentLevel, expected] of expectedCosts.entries()) {
      const level =
        currentLevel === expectedCosts.length - 1 ? 50 : currentLevel;
      expect(getEnhanceCost(level, rarity)).toBe(expected);
    }
  });

  it("applies rarity multiplier through level 49", () => {
    expect(getEnhanceCost(49, "Common")).toBe(250000);
    expect(getEnhanceCost(49, "Rare")).toBe(1000000);
    expect(getEnhanceCost(49, "Legendary")).toBe(4000000);
  });

  it.each([
    [0, 0.95],
    [4, 0.878],
    [10, 0.77],
    [25, 0.5],
    [40, 0.23],
    [49, 0.068],
    [50, 0],
    [51, 0],
  ])("computes enhance success rate for current level %i", (currentLevel, expected) => {
    expect(getEnhanceSuccessRate(currentLevel)).toBeCloseTo(expected, 6);
  });

  it("clamps negative levels to the level zero success rate", () => {
    expect(getEnhanceSuccessRate(-1)).toBe(0.95);
  });

  it("always fails at rate zero and succeeds at rate one", () => {
    expect(rollEnhanceSuccess(0, () => 0)).toBe(false);
    expect(rollEnhanceSuccess(1, () => 0.999)).toBe(true);
  });

  it("uses the provided random source against the clamped success rate", () => {
    expect(rollEnhanceSuccess(0.4, () => 0.399)).toBe(true);
    expect(rollEnhanceSuccess(0.4, () => 0.4)).toBe(false);
    expect(rollEnhanceSuccess(2, () => 0.999)).toBe(true);
    expect(rollEnhanceSuccess(-1, () => 0)).toBe(false);
  });
});
