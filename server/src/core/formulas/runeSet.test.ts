import { describe, expect, it } from "vitest";
import {
  computeRuneSetBonus,
  getSetTierBonus,
  RUNE_SET_TIER1_BONUS,
  RUNE_SET_TIER1_COUNT,
  RUNE_SET_TIER2_BONUS,
  RUNE_SET_TIER2_COUNT,
  RUNE_SET_TIER3_BONUS,
  RUNE_SET_TIER3_COUNT,
} from "./runeSet.js";

describe("rune set formulas", () => {
  it("matches UE tier thresholds and bonuses", () => {
    expect(RUNE_SET_TIER1_COUNT).toBe(2);
    expect(RUNE_SET_TIER2_COUNT).toBe(4);
    expect(RUNE_SET_TIER3_COUNT).toBe(6);
    expect(RUNE_SET_TIER1_BONUS).toBe(Math.fround(0.05));
    expect(RUNE_SET_TIER2_BONUS).toBe(Math.fround(0.12));
    expect(RUNE_SET_TIER3_BONUS).toBe(Math.fround(0.25));
  });

  it.each([
    [1, 0],
    [2, 0.05],
    [4, 0.12],
    [6, 0.25],
    [7, 0.25],
  ])("returns the highest reached tier for %i runes", (count, expected) => {
    expect(getSetTierBonus(count)).toBe(Math.fround(expected));
  });

  it("computes all four rune set mappings as pure additive bonuses", () => {
    const bonus = computeRuneSetBonus({
      1: 6,
      2: 4,
      3: 2,
      4: 6,
    });

    expect(bonus.core).toEqual({
      physAtk: Math.fround(0.25),
      magicAtk: Math.fround(0.25),
      physDef: Math.fround(0.12),
      magicDef: Math.fround(0.12),
      hp: Math.fround(0.05),
    });
    expect(bonus.util).toEqual({
      critDamage: Math.fround(0.25),
      goldFind: Math.fround(0.25),
      expBoost: Math.fround(0.25),
      offlineEff: Math.fround(0.05),
    });
  });
});
