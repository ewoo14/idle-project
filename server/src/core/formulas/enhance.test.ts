import { describe, expect, it } from "vitest";
import {
  getEnhanceCost,
  getEnhanceSuccessRate,
  MAX_ENHANCE_LEVEL,
  rollEnhanceSuccess,
} from "./enhance.js";

describe("enhance formulas", () => {
  it("matches the UE5 max enhance level", () => {
    expect(MAX_ENHANCE_LEVEL).toBe(5);
  });

  it.each([
    [0, 100],
    [1, 400],
    [2, 900],
    [3, 1600],
    [4, 2500],
    [5, 0],
    [6, 0],
  ])("computes enhance cost for current level %i", (currentLevel, expected) => {
    expect(getEnhanceCost(currentLevel)).toBe(expected);
  });

  it("clamps negative levels to the level zero cost", () => {
    expect(getEnhanceCost(-1)).toBe(100);
  });

  it.each([
    [0, 0.95],
    [1, 0.85],
    [2, 0.7],
    [3, 0.55],
    [4, 0.4],
    [5, 0],
    [6, 0],
  ])("computes enhance success rate for current level %i", (currentLevel, expected) => {
    expect(getEnhanceSuccessRate(currentLevel)).toBe(expected);
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
