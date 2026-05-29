import { describe, expect, it } from "vitest";
import {
  getEffectiveBonusPercent,
  getPetCatalog,
  getPetEvolveCost,
  getPetStarMultiplier,
  PET_BONUS_TYPE,
  PET_EVOLVE_BASE,
  PET_STAR_STEP,
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

describe("pet evolution cost (star)", () => {
  it("starts at the base cost for star 0", () => {
    expect(getPetEvolveCost(0)).toBe(PET_EVOLVE_BASE);
    expect(getPetEvolveCost(0)).toBe(50000);
  });

  it("grows geometrically and matches the UE5 parity table", () => {
    expect(getPetEvolveCost(1)).toBe(90000);
    expect(getPetEvolveCost(2)).toBe(162000);
    expect(getPetEvolveCost(3)).toBe(291600);
    expect(getPetEvolveCost(4)).toBe(524880);
    expect(getPetEvolveCost(5)).toBe(944784);
  });

  it("is strictly monotonic increasing and always positive", () => {
    let previous = 0;
    for (let star = 0; star <= 12; star += 1) {
      const cost = getPetEvolveCost(star);
      expect(cost).toBeGreaterThan(0);
      expect(cost).toBeGreaterThan(previous);
      previous = cost;
    }
  });

  it("guards negative stars to the star 0 cost", () => {
    expect(getPetEvolveCost(-1)).toBe(getPetEvolveCost(0));
    expect(getPetEvolveCost(-5)).toBe(PET_EVOLVE_BASE);
  });
});

describe("pet star multiplier", () => {
  it("is exactly 1.0 at star 0", () => {
    expect(getPetStarMultiplier(0)).toBe(1);
  });

  it("increases linearly with the same fround path as the client", () => {
    for (let star = 0; star <= 5; star += 1) {
      expect(getPetStarMultiplier(star)).toBe(
        Math.fround(1 + PET_STAR_STEP * star),
      );
    }
    expect(getPetStarMultiplier(5)).toBe(1.75);
  });

  it("is strictly monotonic increasing", () => {
    let previous = 0;
    for (let star = 0; star <= 12; star += 1) {
      const multiplier = getPetStarMultiplier(star);
      expect(multiplier).toBeGreaterThan(previous);
      previous = multiplier;
    }
  });

  it("guards negative stars to the star 0 multiplier", () => {
    expect(getPetStarMultiplier(-1)).toBe(1);
    expect(getPetStarMultiplier(-9)).toBe(getPetStarMultiplier(0));
  });
});
