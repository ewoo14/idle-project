import { describe, expect, it } from "vitest";
import {
  getEffectiveBonusPercent,
  getPetCatalog,
  PET_BONUS_TYPE,
} from "./petBonus.js";

describe("pet bonus catalog", () => {
  it("mirrors the UE5 pet expansion catalog", () => {
    expect(getPetCatalog()).toEqual([
      {
        id: "dog",
        name: "Dog",
        bonusType: PET_BONUS_TYPE.Gold,
        bonusPercent: 20,
      },
      {
        id: "bird",
        name: "Bird",
        bonusType: PET_BONUS_TYPE.Drop,
        bonusPercent: 15,
      },
      {
        id: "cat",
        name: "Cat",
        bonusType: PET_BONUS_TYPE.Exp,
        bonusPercent: 15,
      },
      {
        id: "wolf",
        name: "Wolf",
        bonusType: PET_BONUS_TYPE.PhysAtk,
        bonusPercent: 10,
      },
      {
        id: "owl",
        name: "Owl",
        bonusType: PET_BONUS_TYPE.MagicAtk,
        bonusPercent: 10,
      },
      {
        id: "bear",
        name: "Bear",
        bonusType: PET_BONUS_TYPE.Hp,
        bonusPercent: 12,
      },
      {
        id: "turtle",
        name: "Turtle",
        bonusType: PET_BONUS_TYPE.Def,
        bonusPercent: 12,
      },
      {
        id: "fox",
        name: "Fox",
        bonusType: PET_BONUS_TYPE.Gold,
        bonusPercent: 30,
      },
      {
        id: "rabbit",
        name: "Rabbit",
        bonusType: PET_BONUS_TYPE.Drop,
        bonusPercent: 25,
      },
      {
        id: "dragon",
        name: "Dragon",
        bonusType: PET_BONUS_TYPE.AllStat,
        bonusPercent: 8,
      },
    ]);
  });

  it("computes effective percent with the same fround path as pet leveling", () => {
    expect(getEffectiveBonusPercent(10, 1)).toBe(11);
    expect(getEffectiveBonusPercent(12, 5)).toBe(18);
    expect(getEffectiveBonusPercent(8, 10)).toBe(16);
  });
});
