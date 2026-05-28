import { describe, expect, it } from "vitest";
import {
  CLASS_RUNE_SLOT_INDEX,
  getClassMasteryMultipliers,
  getClassRuneCraftCost,
} from "./classRune.js";
import { getCoreRuneMultiplier } from "./rune.js";

describe("class rune formulas", () => {
  it("matches the UE class rune slot index", () => {
    expect(CLASS_RUNE_SLOT_INDEX).toBe(6);
  });

  it.each([
    [1, "physAtk", "physDef"],
    [2, "magicAtk", undefined],
    [3, "physAtk", undefined],
    [4, "physAtk", undefined],
    [5, "magicAtk", "hp"],
    [6, "physDef", "hp"],
    [7, "physAtk", undefined],
    [8, "magicAtk", undefined],
  ] as const)("maps class %i to its mastery stats", (classId, first, second) => {
    const unit = getCoreRuneMultiplier(3, 4);
    const multipliers = getClassMasteryMultipliers(classId, 3, 4);
    const entries = Object.entries(multipliers).filter(
      ([, value]) => value !== 0,
    );

    expect(multipliers[first]).toBe(unit);
    if (second) {
      expect(multipliers[second]).toBe(unit);
      expect(entries).toHaveLength(2);
    } else {
      expect(entries).toHaveLength(1);
    }
  });

  it("returns zero multipliers for invalid classes", () => {
    expect(getClassMasteryMultipliers(0, 3, 4)).toEqual({
      physAtk: 0,
      magicAtk: 0,
      physDef: 0,
      magicDef: 0,
      hp: 0,
    });
  });

  it.each([
    [1, 25],
    [2, 60],
    [3, 150],
    [4, 400],
    [5, 1000],
    [6, 1700],
    [7, 2500],
    [0, 25],
  ])("computes class rune craft cost for rarity %i", (rarity, expected) => {
    expect(getClassRuneCraftCost(rarity)).toBe(expected);
  });
});
