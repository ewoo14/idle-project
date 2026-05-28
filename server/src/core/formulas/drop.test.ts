import { describe, expect, it } from "vitest";
import {
  computeItemBonus,
  getAffixCount,
  getRarityDropChances,
  getRarityStatMultiplier,
  rollAffixes,
  rollBaseItem,
  rollRarityForLevel,
} from "./drop.js";

describe("drop formulas", () => {
  function seededRng(seed: number) {
    let state = seed >>> 0;
    return () => {
      state = (state * 1664525 + 1013904223) >>> 0;
      return state / 0x1_0000_0000;
    };
  }

  function clientFloatThresholds(level: number) {
    const safeLevel = Math.max(level, 1);
    const levelScale = Math.fround(
      Math.min(Math.max(Math.fround(Math.fround(safeLevel - 1) / 99), 0), 1),
    );

    const noneChance = Math.fround(0.02);
    const rareChance = Math.fround(
      Math.fround(0.28) + Math.fround(Math.fround(0.02) * levelScale),
    );
    const epicChance = Math.fround(Math.fround(0.06) * levelScale);
    const uniqueChance = Math.fround(Math.fround(0.025) * levelScale);
    const legendaryChance = Math.fround(Math.fround(0.015) * levelScale);
    const transcendentChance = Math.fround(Math.fround(0.007) * levelScale);
    const mythicChance = Math.fround(Math.fround(0.005) * levelScale);
    let commonChance = Math.fround(1 - noneChance);
    commonChance = Math.fround(commonChance - rareChance);
    commonChance = Math.fround(commonChance - epicChance);
    commonChance = Math.fround(commonChance - uniqueChance);
    commonChance = Math.fround(commonChance - legendaryChance);
    commonChance = Math.fround(commonChance - transcendentChance);
    commonChance = Math.fround(commonChance - mythicChance);
    commonChance = Math.fround(Math.max(0, commonChance));

    let threshold = Math.fround(noneChance + commonChance);
    const commonThreshold = threshold;
    threshold = Math.fround(threshold + rareChance);
    const rareThreshold = threshold;
    threshold = Math.fround(threshold + epicChance);
    const epicThreshold = threshold;
    threshold = Math.fround(threshold + uniqueChance);
    const uniqueThreshold = threshold;
    threshold = Math.fround(threshold + legendaryChance);
    const legendaryThreshold = threshold;
    threshold = Math.fround(threshold + transcendentChance);

    return {
      commonThreshold,
      rareThreshold,
      epicThreshold,
      uniqueThreshold,
      legendaryThreshold,
      transcendentThreshold: threshold,
      mythicChance,
    };
  }

  it("keeps rarity stat multipliers aligned with FDropFormula", () => {
    expect(getRarityStatMultiplier("None")).toBe(0);
    expect(getRarityStatMultiplier("Common")).toBe(1);
    expect(getRarityStatMultiplier("Rare")).toBe(Math.fround(1.7));
    expect(getRarityStatMultiplier("Epic")).toBe(Math.fround(2.3));
    expect(getRarityStatMultiplier("Unique")).toBe(Math.fround(2.75));
    expect(getRarityStatMultiplier("Legendary")).toBe(Math.fround(3.2));
    expect(getRarityStatMultiplier("Transcendent")).toBe(Math.fround(3.85));
    expect(getRarityStatMultiplier("Mythic")).toBe(Math.fround(4.5));
  });

  it("rolls only None through Rare at level 1", () => {
    const thresholds = clientFloatThresholds(1);
    expect(rollRarityForLevel(1, () => 0)).toBe("None");
    expect(rollRarityForLevel(1, () => 0.02)).toBe("Common");
    expect(
      rollRarityForLevel(1, () => thresholds.commonThreshold - 0.000_001),
    ).toBe("Common");
    expect(rollRarityForLevel(1, () => thresholds.commonThreshold)).toBe(
      "Rare",
    );
    expect(rollRarityForLevel(1, () => 0.999_999)).toBe("Rare");
  });

  it("rolls level 100 rarity using FDropFormula cumulative thresholds", () => {
    const thresholds = clientFloatThresholds(100);
    expect(rollRarityForLevel(100, () => 0.019_999)).toBe("None");
    expect(rollRarityForLevel(100, () => 0.02)).toBe("Common");
    expect(
      rollRarityForLevel(100, () => thresholds.commonThreshold - 0.000_001),
    ).toBe("Common");
    expect(rollRarityForLevel(100, () => thresholds.commonThreshold)).toBe(
      "Rare",
    );
    expect(
      rollRarityForLevel(100, () => thresholds.rareThreshold - 0.000_001),
    ).toBe("Rare");
    expect(rollRarityForLevel(100, () => thresholds.rareThreshold)).toBe(
      "Epic",
    );
    expect(
      rollRarityForLevel(100, () => thresholds.epicThreshold - 0.000_001),
    ).toBe("Epic");
    expect(rollRarityForLevel(100, () => thresholds.epicThreshold)).toBe(
      "Unique",
    );
    expect(
      rollRarityForLevel(100, () => thresholds.uniqueThreshold - 0.000_001),
    ).toBe("Unique");
    expect(rollRarityForLevel(100, () => thresholds.uniqueThreshold)).toBe(
      "Legendary",
    );
    expect(
      rollRarityForLevel(100, () => thresholds.legendaryThreshold - 0.000_001),
    ).toBe("Legendary");
    expect(rollRarityForLevel(100, () => thresholds.legendaryThreshold)).toBe(
      "Transcendent",
    );
    expect(
      rollRarityForLevel(
        100,
        () => thresholds.transcendentThreshold - 0.000_001,
      ),
    ).toBe("Transcendent");
    expect(
      rollRarityForLevel(100, () => thresholds.transcendentThreshold),
    ).toBe("Mythic");
  });

  it("exposes level-scaled rarity chances that sum to one", () => {
    const level1 = getRarityDropChances(1);
    const level100 = getRarityDropChances(100);

    expect(Object.keys(level100)).toEqual([
      "None",
      "Common",
      "Rare",
      "Epic",
      "Unique",
      "Legendary",
      "Transcendent",
      "Mythic",
    ]);
    expect(
      Object.values(level1).reduce((sum, chance) => sum + chance, 0),
    ).toBeCloseTo(1, 6);
    expect(
      Object.values(level100).reduce((sum, chance) => sum + chance, 0),
    ).toBeCloseTo(1, 6);
    expect(level100.Unique).toBeGreaterThan(0);
    expect(level100.Unique).toBeLessThan(level100.Epic);
    expect(level100.Transcendent).toBeGreaterThan(0);
    expect(level100.Transcendent).toBeLessThan(level100.Legendary);
    expect(level100.Mythic).toBeGreaterThan(level1.Mythic);
  });

  it("keeps Mythic boundary aligned with client float arithmetic", () => {
    const thresholds = clientFloatThresholds(56);
    expect(thresholds.mythicChance).toBeGreaterThan(0);

    const rollJustInsideClientLegendary =
      thresholds.legendaryThreshold - 0.000_000_05;

    expect(rollRarityForLevel(56, () => rollJustInsideClientLegendary)).toBe(
      "Legendary",
    );
    expect(rollRarityForLevel(56, () => thresholds.transcendentThreshold)).toBe(
      "Mythic",
    );
  });

  it("clamps level below 1 to the level 1 rarity table", () => {
    expect(rollRarityForLevel(0, () => 0.999_999)).toBe("Rare");
  });

  it("computes weapon, armor, and accessory bonuses with client parity", () => {
    expect(computeItemBonus(1, 10, "Rare", 2)).toEqual({
      bonusAtk: 34,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
    expect(computeItemBonus(2, 10, "Rare", 2)).toEqual({
      bonusAtk: 0,
      bonusDef: Math.fround(Math.fround(34) * Math.fround(0.7)),
      bonusHp: 102,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
    expect(computeItemBonus(8, 10, "Rare", 2)).toEqual({
      bonusAtk: 17,
      bonusDef: Math.fround(Math.fround(34) * Math.fround(0.3)),
      bonusHp: 68,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
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
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
    expect(computeItemBonus(1, 10, "Legendary", -1)).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
  });

  it("gets affix count by rarity with deterministic legendary branching", () => {
    expect(getAffixCount("None", () => 0.99)).toBe(0);
    expect(getAffixCount("Common", () => 0.99)).toBe(0);
    expect(getAffixCount("Rare", () => 0.99)).toBe(1);
    expect(getAffixCount("Epic", () => 0.99)).toBe(2);
    expect(getAffixCount("Unique", () => 0.99)).toBe(2);
    expect(getAffixCount("Legendary", () => 0.49)).toBe(2);
    expect(getAffixCount("Legendary", () => 0.5)).toBe(3);
    expect(getAffixCount("Transcendent", () => 0.5)).toBe(3);
    expect(getAffixCount("Mythic", () => 0.99)).toBe(3);
  });

  it("rolls no affixes for common equipment", () => {
    expect(rollAffixes("Common", 20, () => 0.99)).toEqual({
      bonusCritRate: 0,
      bonusAtkSpeed: 0,
      bonusMagicAtk: 0,
      bonusPhysDef: 0,
      bonusMagicDef: 0,
      bonusAffixHp: 0,
      bonusCritDmg: 0,
    });
  });

  it("keeps one-affix rarity rolls unique without stale affix carryover", () => {
    const rolls = [0.99, 0.99, 0.5];
    const rng = () => rolls.shift() ?? 0;
    const affixes = rollAffixes("Rare", 20, rng);
    const affixCount = [
      affixes.bonusCritRate,
      affixes.bonusAtkSpeed,
      affixes.bonusMagicAtk,
    ].filter((value) => value > 0).length;

    expect(affixCount).toBe(1);
    expect(affixes.bonusCritRate).toBeGreaterThanOrEqual(0);
    expect(affixes.bonusCritRate).toBeLessThanOrEqual(0.05);
    expect(affixes.bonusAtkSpeed).toBeGreaterThanOrEqual(0);
    expect(affixes.bonusAtkSpeed).toBeLessThanOrEqual(0.15);
    expect(affixes.bonusMagicAtk).toBeGreaterThanOrEqual(0);
    expect(affixes.bonusMagicAtk).toBeLessThanOrEqual(30);
  });

  it("rolls unique deterministic affixes with client ranges and rounding", () => {
    const rolls = [0.5, 0, 0.99, 0.5, 0.5, 0.5];
    const rng = () => rolls.shift() ?? 0;

    expect(rollAffixes("Legendary", 20, rng)).toEqual({
      bonusCritRate: 0,
      bonusAtkSpeed: 0.05,
      bonusMagicAtk: 0,
      bonusPhysDef: 6,
      bonusMagicDef: 0,
      bonusAffixHp: 0,
      bonusCritDmg: 0.05,
    });
  });

  it("rolls all three affixes for Mythic equipment", () => {
    const rolls = [0.5, 0, 0.99, 0.5, 0.5, 0.5];
    const rng = () => rolls.shift() ?? 0;

    expect(rollAffixes("Mythic", 20, rng)).toEqual({
      bonusCritRate: 0,
      bonusAtkSpeed: 0.05,
      bonusMagicAtk: 0,
      bonusPhysDef: 0,
      bonusMagicDef: 0,
      bonusAffixHp: 40,
      bonusCritDmg: 0.05,
    });
  });

  it("clamps affix level below one before flat scaling", () => {
    const rolls = [0, 0.99, 0.5];
    const rng = () => rolls.shift() ?? 0;

    expect(rollAffixes("Rare", 0, rng)).toEqual({
      bonusCritRate: 0,
      bonusAtkSpeed: 0.05,
      bonusMagicAtk: 0,
      bonusPhysDef: 0,
      bonusMagicDef: 0,
      bonusAffixHp: 0,
      bonusCritDmg: 0,
    });
  });

  it("selects deterministic base item names per slot", () => {
    const rolls = [0.99];
    const rng = () => rolls.shift() ?? 0;

    expect(rollBaseItem(1, rng)).toEqual({
      baseItemId: "wand",
      nameKo: "마법봉",
      nameEn: "Wand",
      statBias: "magic",
      atkScale: 0.85,
      defScale: 1,
      hpScale: 1,
    });
  });

  it("keeps base item stat scales aligned with ItemFactory catalog", () => {
    expect(rollBaseItem(1, () => 1 / 6)).toMatchObject({
      baseItemId: "greatsword",
      atkScale: 1.15,
      defScale: 0.9,
      hpScale: 1,
    });
    expect(rollBaseItem(2, () => 0)).toMatchObject({
      baseItemId: "helm",
      atkScale: 1,
      defScale: 1.08,
      hpScale: 1,
    });
    expect(rollBaseItem(8, () => 1 / 3)).toMatchObject({
      baseItemId: "amulet",
      atkScale: 0.95,
      defScale: 1,
      hpScale: 1.05,
    });
  });

  it("exposes at least six weapon base items", () => {
    const weaponBaseIds = new Set<string>();
    for (let index = 0; index < 6; index += 1) {
      weaponBaseIds.add(rollBaseItem(1, () => index / 6).baseItemId);
    }

    expect(weaponBaseIds.size).toBeGreaterThanOrEqual(6);
  });

  it("can roll every expanded affix from the Mythic pool", () => {
    const found = new Set<string>();
    for (let seed = 1; seed <= 200; seed += 1) {
      const affixes = rollAffixes("Mythic", 30, seededRng(seed));
      if (affixes.bonusPhysDef > 0) found.add("PhysDef");
      if (affixes.bonusMagicDef > 0) found.add("MagicDef");
      if (affixes.bonusAffixHp > 0) found.add("Hp");
      if (affixes.bonusCritDmg > 0) found.add("CritDmg");
    }

    expect(found).toEqual(new Set(["PhysDef", "MagicDef", "Hp", "CritDmg"]));
  });
});
