import { describe, expect, it } from "vitest";
import {
  ENHANCE_PITY_THRESHOLD,
  ENHANCE_SAFE_MAX_LEVEL,
  getEnhanceCost,
  getEnhanceSuccessRate,
  getRarityCostMultiplier,
  isRiskLevel,
  MAX_ENHANCE_LEVEL,
  resolveEnhanceAttempt,
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
    ["Rare", 2],
    ["Epic", 4],
    ["Unique", 8],
    ["Legendary", 16],
    ["Transcendent", 32],
    ["Mythic", 64],
  ] as const)("computes %s enhance cost multiplier", (rarity, expected) => {
    expect(getRarityCostMultiplier(rarity)).toBe(expected);
  });

  it("applies rarity multiplier while keeping Common cost compatible", () => {
    expect(getEnhanceCost(1, "Common")).toBe(400);
    expect(getEnhanceCost(1, "Rare")).toBe(800);
    expect(getEnhanceCost(0, "Legendary")).toBe(1600);
    expect(getEnhanceCost(0, "Mythic")).toBe(6400);
    expect(getEnhanceCost(MAX_ENHANCE_LEVEL, "Legendary")).toBe(0);
  });

  it.each([
    ["Common", [100, 400, 900, 1600, 2500, 0]],
    ["Rare", [200, 800, 1800, 3200, 5000, 0]],
    ["Epic", [400, 1600, 3600, 6400, 10000, 0]],
    ["Unique", [800, 3200, 7200, 12800, 20000, 0]],
    ["Legendary", [1600, 6400, 14400, 25600, 40000, 0]],
    ["Transcendent", [3200, 12800, 28800, 51200, 80000, 0]],
    ["Mythic", [6400, 25600, 57600, 102400, 160000, 0]],
  ] as const)("keeps the old +0 to +4 rarity-scaled cost table for %s", (rarity, expectedCosts) => {
    for (const [currentLevel, expected] of expectedCosts.entries()) {
      const level =
        currentLevel === expectedCosts.length - 1 ? 50 : currentLevel;
      expect(getEnhanceCost(level, rarity)).toBe(expected);
    }
  });

  it("applies rarity multiplier through level 49", () => {
    expect(getEnhanceCost(49, "Common")).toBe(250000);
    expect(getEnhanceCost(49, "Rare")).toBe(500000);
    expect(getEnhanceCost(49, "Legendary")).toBe(4000000);
    expect(getEnhanceCost(49, "Mythic")).toBe(16000000);
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

  it.each([
    0, 1, 4, 10, 25, 40, 49, 50,
  ])("returns UE5 float parity success rate for current level %i", (currentLevel) => {
    const expected =
      currentLevel >= MAX_ENHANCE_LEVEL
        ? 0
        : Math.fround(
            Math.max(0.05, Math.min(0.95, 0.95 - currentLevel * 0.018)),
          );

    expect(getEnhanceSuccessRate(currentLevel)).toBe(expected);
  });

  it("clamps negative levels to the level zero success rate", () => {
    expect(getEnhanceSuccessRate(-1)).toBe(Math.fround(0.95));
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

  it("exposes safe and pity thresholds for client parity", () => {
    expect(ENHANCE_SAFE_MAX_LEVEL).toBe(9);
    expect(ENHANCE_PITY_THRESHOLD).toBe(12);
    expect(isRiskLevel(9)).toBe(false);
    expect(isRiskLevel(10)).toBe(true);
    expect(isRiskLevel(49)).toBe(true);
    expect(isRiskLevel(50)).toBe(false);
  });

  it("keeps safe-level failures at the same level and increments fail streak", () => {
    expect(
      resolveEnhanceAttempt({
        currentLevel: 5,
        failStreak: 2,
        useProtection: false,
        hasProtection: false,
        roll: 0.999,
      }),
    ).toEqual({
      attempted: true,
      success: false,
      consumedProtection: false,
      newLevel: 5,
      newFailStreak: 3,
      pityTriggered: false,
    });
  });

  it("downgrades risk-level failures unless protection is consumed", () => {
    expect(
      resolveEnhanceAttempt({
        currentLevel: 20,
        failStreak: 4,
        useProtection: false,
        hasProtection: false,
        roll: 0.999,
      }),
    ).toMatchObject({
      attempted: true,
      success: false,
      consumedProtection: false,
      newLevel: 19,
      newFailStreak: 5,
    });

    expect(
      resolveEnhanceAttempt({
        currentLevel: 20,
        failStreak: 4,
        useProtection: true,
        hasProtection: true,
        roll: 0.999,
      }),
    ).toMatchObject({
      attempted: true,
      success: false,
      consumedProtection: true,
      newLevel: 20,
      newFailStreak: 5,
    });
  });

  it("forces success on a risk-level pity attempt", () => {
    expect(
      resolveEnhanceAttempt({
        currentLevel: 20,
        failStreak: 12,
        useProtection: false,
        hasProtection: false,
        roll: 0.999,
      }),
    ).toEqual({
      attempted: true,
      success: true,
      consumedProtection: false,
      newLevel: 21,
      newFailStreak: 0,
      pityTriggered: true,
    });
  });
});
