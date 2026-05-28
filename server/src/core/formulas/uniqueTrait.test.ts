import { describe, expect, it } from "vitest";
import {
  getTraitValue,
  rarityGrantsTwoTraits,
  rarityGrantsUnique,
  type UniqueTrait,
} from "./uniqueTrait.js";

describe("unique trait formulas", () => {
  it("only grants unique traits to Unique and Transcendent rarities", () => {
    for (const rarity of [0, 1, 2, 3, 5, 7]) {
      expect(rarityGrantsUnique(rarity)).toBe(false);
    }

    expect(rarityGrantsUnique(4)).toBe(true);
    expect(rarityGrantsUnique(6)).toBe(true);
  });

  it("only Transcendent rarity grants two traits", () => {
    for (const rarity of [0, 1, 2, 3, 4, 5, 7]) {
      expect(rarityGrantsTwoTraits(rarity)).toBe(false);
    }

    expect(rarityGrantsTwoTraits(6)).toBe(true);
  });

  it("mirrors client trait values with a Transcendent 1.5x multiplier", () => {
    const expectedUniqueValues: Record<UniqueTrait, number> = {
      0: 0,
      1: Math.fround(0.08),
      2: Math.fround(0.15),
      3: Math.fround(0.05),
      4: Math.fround(0.1),
      5: Math.fround(0.08),
      6: Math.fround(0.12),
      7: Math.fround(0.12),
      8: Math.fround(0.1),
    };

    for (const [trait, uniqueValue] of Object.entries(expectedUniqueValues)) {
      expect(getTraitValue(Number(trait), 4)).toBe(uniqueValue);
      expect(getTraitValue(Number(trait), 6)).toBe(
        Math.fround(uniqueValue * 1.5),
      );
    }
  });

  it("returns zero for None, unsupported rarities, and unknown traits", () => {
    expect(getTraitValue(1, 7)).toBe(0);
    expect(getTraitValue(1, 5)).toBe(0);
    expect(getTraitValue(0, 4)).toBe(0);
    expect(getTraitValue(99, 4)).toBe(0);
  });
});
