import { describe, expect, it } from "vitest";
import {
  computeItemBonus,
  getRarityStatMultiplier,
  rollRarityForLevel,
} from "./drop.js";

describe("drop formulas", () => {
  it("keeps rarity stat multipliers aligned with FDropFormula", () => {
    expect(getRarityStatMultiplier("None")).toBe(0);
    expect(getRarityStatMultiplier("Common")).toBe(1);
    expect(getRarityStatMultiplier("Uncommon")).toBe(Math.fround(1.3));
    expect(getRarityStatMultiplier("Rare")).toBe(Math.fround(1.7));
    expect(getRarityStatMultiplier("Epic")).toBe(Math.fround(2.3));
    expect(getRarityStatMultiplier("Legendary")).toBe(Math.fround(3.2));
  });

  it("rolls only None through Rare at level 1", () => {
    expect(rollRarityForLevel(1, () => 0)).toBe("None");
    expect(rollRarityForLevel(1, () => 0.02)).toBe("Common");
    expect(rollRarityForLevel(1, () => 0.719_999)).toBe("Common");
    expect(rollRarityForLevel(1, () => 0.720_001)).toBe("Uncommon");
    expect(rollRarityForLevel(1, () => 0.919_999)).toBe("Uncommon");
    expect(rollRarityForLevel(1, () => 0.920_001)).toBe("Rare");
    expect(rollRarityForLevel(1, () => 0.999_999)).toBe("Rare");
  });

  it("rolls level 100 rarity using FDropFormula cumulative thresholds", () => {
    expect(rollRarityForLevel(100, () => 0.019_999)).toBe("None");
    expect(rollRarityForLevel(100, () => 0.02)).toBe("Common");
    expect(rollRarityForLevel(100, () => 0.519_999)).toBe("Common");
    expect(rollRarityForLevel(100, () => 0.520_001)).toBe("Uncommon");
    expect(rollRarityForLevel(100, () => 0.719_999)).toBe("Uncommon");
    expect(rollRarityForLevel(100, () => 0.720_001)).toBe("Rare");
    expect(rollRarityForLevel(100, () => 0.919_999)).toBe("Rare");
    expect(rollRarityForLevel(100, () => 0.920_001)).toBe("Epic");
    expect(rollRarityForLevel(100, () => 0.979_999)).toBe("Epic");
    expect(rollRarityForLevel(100, () => 0.980_001)).toBe("Legendary");
  });

  it("clamps level below 1 to the level 1 rarity table", () => {
    expect(rollRarityForLevel(0, () => 0.999_999)).toBe("Rare");
  });

  it("computes weapon, armor, and accessory bonuses with client parity", () => {
    expect(computeItemBonus(1, 10, "Rare", 2)).toEqual({
      bonusAtk: 34,
      bonusDef: 0,
      bonusHp: 0,
    });
    expect(computeItemBonus(2, 10, "Rare", 2)).toEqual({
      bonusAtk: 0,
      bonusDef: Math.fround(Math.fround(34) * Math.fround(0.7)),
      bonusHp: 102,
    });
    expect(computeItemBonus(8, 10, "Rare", 2)).toEqual({
      bonusAtk: 17,
      bonusDef: Math.fround(Math.fround(34) * Math.fround(0.3)),
      bonusHp: 68,
    });
  });

  it("rounds generated bonuses through client float arithmetic", () => {
    const bonus = computeItemBonus(2, 10, "Rare", 2);

    expect(bonus.bonusDef).toBe(
      Math.fround(Math.fround(34) * Math.fround(0.7)),
    );
    expect(bonus.bonusHp).toBe(Math.fround(Math.fround(34) * Math.fround(3)));
  });

  it("clamps item bonus level and variance before applying rarity", () => {
    expect(computeItemBonus(1, 0, "Common", 1)).toEqual({
      bonusAtk: 1,
      bonusDef: 0,
      bonusHp: 0,
    });
    expect(computeItemBonus(1, 10, "Legendary", -1)).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
    });
  });
});
