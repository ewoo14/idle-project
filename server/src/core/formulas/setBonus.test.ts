import { describe, expect, it } from "vitest";
import type { ItemInstance } from "./equipment.js";
import {
  computeSetBonus,
  getFourPieceBonus,
  getTwoPieceBonus,
  type ItemSet,
  rollItemSet,
} from "./setBonus.js";

function item(
  itemSet: ItemSet,
  overrides: Partial<ItemInstance> = {},
): ItemInstance {
  return {
    itemId: `item_${itemSet}_${overrides.slot ?? 1}`,
    slot: 1,
    rarity: 3,
    itemSet,
    bonusAtk: 0,
    bonusDef: 0,
    bonusHp: 0,
    enhanceLevel: 0,
    ...overrides,
  };
}

describe("set bonus formulas", () => {
  it("returns zero bonuses below the two piece threshold", () => {
    expect(computeSetBonus([item("Warrior")])).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
      magicDef: 0,
      critDmg: 0,
    });
  });

  it("applies Warrior two piece and four piece bonuses together", () => {
    expect(
      computeSetBonus([
        item("Warrior", { slot: 1 }),
        item("Warrior", { slot: 2 }),
        item("Warrior", { slot: 3 }),
        item("Warrior", { slot: 4 }),
      ]),
    ).toEqual({
      bonusAtk: 70,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0.05,
      atkSpeed: 0,
      magicAtk: 0,
      magicDef: 0,
      critDmg: 0,
    });
  });

  it("applies Guardian two piece and four piece bonuses together", () => {
    expect(
      computeSetBonus([
        item("Guardian", { slot: 1 }),
        item("Guardian", { slot: 2 }),
        item("Guardian", { slot: 3 }),
        item("Guardian", { slot: 4 }),
      ]),
    ).toEqual({
      bonusAtk: 0,
      bonusDef: 50,
      bonusHp: 350,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
      magicDef: 0,
      critDmg: 0,
    });
  });

  it("applies Arcane two piece and four piece bonuses together", () => {
    expect(
      computeSetBonus([
        item("Arcane", { slot: 1 }),
        item("Arcane", { slot: 2 }),
        item("Arcane", { slot: 3 }),
        item("Arcane", { slot: 4 }),
      ]),
    ).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 70,
      magicDef: 0,
      critDmg: 0.1,
    });
  });

  it("ignores invalid equipped entries for set counts", () => {
    expect(
      computeSetBonus([
        item("Warrior", { slot: 0, rarity: 3 } as Partial<ItemInstance>),
        item("Warrior", { slot: 1, rarity: "None" }),
        item("None", { slot: 2, rarity: 3 }),
        item("Warrior", { slot: 3, rarity: 3 }),
      ]),
    ).toEqual({
      bonusAtk: 0,
      bonusDef: 0,
      bonusHp: 0,
      critRate: 0,
      atkSpeed: 0,
      magicAtk: 0,
      magicDef: 0,
      critDmg: 0,
    });
  });

  it("matches the client anchor values for each piece bonus table", () => {
    expect(getTwoPieceBonus("Warrior")).toMatchObject({ bonusAtk: 20 });
    expect(getFourPieceBonus("Warrior")).toMatchObject({
      bonusAtk: 50,
      critRate: 0.05,
    });
    expect(getTwoPieceBonus("Guardian")).toMatchObject({
      bonusDef: 15,
      bonusHp: 100,
    });
    expect(getFourPieceBonus("Guardian")).toMatchObject({
      bonusDef: 35,
      bonusHp: 250,
    });
    expect(getTwoPieceBonus("Arcane")).toMatchObject({ magicAtk: 20 });
    expect(getFourPieceBonus("Arcane")).toMatchObject({
      magicAtk: 50,
      critDmg: 0.1,
    });
  });

  it("rolls no set for None and Common rarity items", () => {
    expect(rollItemSet("None", () => 0)).toBe("None");
    expect(rollItemSet("Common", () => 0)).toBe("None");
  });

  it("rolls eligible item sets deterministically from the supplied rng", () => {
    expect(rollItemSet("Rare", () => 0)).toBe("Warrior");
    expect(rollItemSet("Rare", () => 0.34)).toBe("Guardian");
    expect(rollItemSet("Rare", () => 0.67)).toBe("Arcane");
    expect(rollItemSet("Mythic", () => 0.67)).toBe("Arcane");
  });
});
