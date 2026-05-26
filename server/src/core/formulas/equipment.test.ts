import { describe, expect, it } from "vitest";
import {
  computeInventoryBonus,
  computeItemPowerScore,
  type ItemInstance,
  type ItemRarity,
  type ItemSlot,
} from "./equipment.js";

const rareWeapon: ItemInstance = {
  itemId: "rare_weapon",
  slot: 1,
  rarity: 3,
  bonusAtk: 15,
  bonusDef: 0,
  bonusHp: 0,
  enhanceLevel: 0,
};

const clientServerEquipmentAnchors = [
  {
    name: "rare weapon base",
    item: rareWeapon,
    clientPowerScore: 15,
    clientBonus: {
      bonusAtk: 15,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  },
  {
    name: "enhanced weapon",
    item: { ...rareWeapon, enhanceLevel: 2 },
    clientPowerScore: 18,
    clientBonus: {
      bonusAtk: 18,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  },
  {
    name: "helmet def and hp",
    item: {
      itemId: "rare_helmet",
      slot: 2,
      rarity: 3,
      bonusAtk: 0,
      bonusDef: 9,
      bonusHp: 60,
      enhanceLevel: 0,
    } satisfies ItemInstance,
    clientPowerScore: 15,
    clientBonus: {
      bonusAtk: 0,
      bonusDef: 9,
      bonusHp: 60,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  },
  {
    name: "enhanced accessory mixed stats",
    item: {
      itemId: "uncommon_accessory",
      slot: 8,
      rarity: 2,
      bonusAtk: 5,
      bonusDef: 5,
      bonusHp: 0,
      enhanceLevel: 1,
    } satisfies ItemInstance,
    clientPowerScore: 11,
    clientBonus: {
      bonusAtk: 5.5,
      bonusDef: 5.5,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  },
  {
    name: "enhanced affix weapon",
    item: {
      itemId: "rare_affix_weapon",
      slot: 1,
      rarity: 3,
      bonusAtk: 10,
      bonusDef: 0,
      bonusHp: 0,
      bonusCritRate: 0.02,
      bonusAtkSpeed: 0.1,
      bonusMagicAtk: 5,
      enhanceLevel: 2,
    } satisfies ItemInstance,
    clientPowerScore: 54,
    clientBonus: {
      bonusAtk: 12,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0.024,
      atkSpeed: 0.12,
      magicAtk: 6,
    },
  },
  {
    name: "empty top low stats",
    item: {
      itemId: "common_top",
      slot: 3,
      rarity: 1,
      bonusAtk: 0,
      bonusDef: 2,
      bonusHp: 10,
      enhanceLevel: 0,
    } satisfies ItemInstance,
    clientPowerScore: 3,
    clientBonus: {
      bonusAtk: 0,
      bonusDef: 2,
      bonusHp: 10,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    },
  },
];

describe("equipment formulas", () => {
  it("computes item power score with the UE5 mirror formula", () => {
    expect(computeItemPowerScore(rareWeapon)).toBe(15);
  });

  it("applies 10 percent power score per enhance level", () => {
    expect(
      computeItemPowerScore({
        ...rareWeapon,
        enhanceLevel: 2,
      }),
    ).toBe(18);
  });

  it("includes HP as one tenth of power score", () => {
    expect(
      computeItemPowerScore({
        itemId: "rare_top",
        slot: 3,
        rarity: 3,
        bonusAtk: 0,
        bonusDef: 9,
        bonusHp: 60,
        enhanceLevel: 0,
      }),
    ).toBe(15);
  });

  it("sums enhanced inventory bonuses across equipped items", () => {
    expect(
      computeInventoryBonus([
        { ...rareWeapon, enhanceLevel: 1 },
        {
          itemId: "rare_helmet",
          slot: 2,
          rarity: 3,
          bonusAtk: 0,
          bonusDef: 10,
          bonusHp: 50,
          enhanceLevel: 2,
        },
      ]),
    ).toEqual({
      bonusAtk: 16.5,
      bonusDef: 12,
      bonusHp: 60,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
  });

  it("returns zero bonuses for an empty inventory", () => {
    expect(computeInventoryBonus([])).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
  });

  it("keeps affix defaults at zero for legacy equipment", () => {
    expect(computeInventoryBonus([rareWeapon])).toEqual({
      bonusAtk: 15,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
    });
  });

  it("applies the enhance multiplier to affix equipment bonuses", () => {
    expect(
      computeInventoryBonus([
        {
          ...rareWeapon,
          bonusCritRate: 0.03,
          bonusAtkSpeed: 0.12,
          bonusMagicAtk: 20,
          enhanceLevel: 2,
        },
      ]),
    ).toEqual({
      bonusAtk: 18,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0.036,
      atkSpeed: 0.144,
      magicAtk: 24,
    });
  });

  it("includes weighted affixes in power score before enhance multiplier", () => {
    expect(
      computeItemPowerScore({
        ...rareWeapon,
        bonusCritRate: 0.02,
        bonusAtkSpeed: 0.09,
        bonusMagicAtk: 10,
      }),
    ).toBe(54);
  });

  it("keeps slot and rarity values aligned to PR 9 enum ranges", () => {
    const slot: ItemSlot = 8;
    const rarity: ItemRarity = 3;
    const typedItem = {
      ...rareWeapon,
      slot,
      rarity,
    } satisfies ItemInstance;

    expect(typedItem.slot).toBe(8);
    expect(typedItem.rarity).toBe(3);
  });

  it.each(
    clientServerEquipmentAnchors,
  )("$name PowerScore cross-validation diff 0 anchor", ({
    item,
    clientPowerScore,
  }) => {
    const serverPowerScore = computeItemPowerScore(item);

    expect(serverPowerScore).toBe(clientPowerScore);
    expect(serverPowerScore - clientPowerScore).toBe(0);
  });

  it.each(
    clientServerEquipmentAnchors,
  )("$name equipment bonus cross-validation diff 0 anchor", ({
    item,
    clientBonus,
  }) => {
    const serverBonus = computeInventoryBonus([item]);

    expect(serverBonus).toEqual(clientBonus);
    expect(serverBonus.bonusAtk - clientBonus.bonusAtk).toBe(0);
    expect(serverBonus.bonusDef - clientBonus.bonusDef).toBe(0);
    expect(serverBonus.bonusHp - clientBonus.bonusHp).toBe(0);
  });
});
