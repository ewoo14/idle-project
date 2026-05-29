import { describe, expect, it } from "vitest";
import {
  getCoreRuneMultiplier,
  getDisenchantEssence,
  getEnhanceEssenceCost,
  getEnhanceGoldCost,
  getRarityUpgradeChance,
  getRarityUpgradeEssenceCost,
  getRarityUpgradeGoldCost,
  getRerollSetEssenceCost,
  getShopRuneRollCost,
  getTransferEssenceCost,
  getUtilCap,
  getUtilRuneValue,
  isCoreRuneType,
  isUtilRuneType,
  RUNE_SLOT_COUNT,
} from "./rune.js";

describe("rune formulas", () => {
  it("matches the UE rune slot count", () => {
    expect(RUNE_SLOT_COUNT).toBe(7);
  });

  it.each([
    [1, true],
    [5, true],
    [6, false],
    [0, false],
  ])("classifies core rune type %i", (type, expected) => {
    expect(isCoreRuneType(type)).toBe(expected);
  });

  it.each([
    [6, true],
    [9, true],
    [1, false],
    [0, false],
  ])("classifies util rune type %i", (type, expected) => {
    expect(isUtilRuneType(type)).toBe(expected);
  });

  it("computes UE float parity core multipliers", () => {
    expect(getCoreRuneMultiplier(1, 0)).toBe(Math.fround(0.02));
    expect(getCoreRuneMultiplier(4, 0)).toBe(Math.fround(0.1));
    expect(getCoreRuneMultiplier(6, 0)).toBe(Math.fround(0.15));
    expect(getCoreRuneMultiplier(7, 50)).toBe(Math.fround(1.68));
    expect(getCoreRuneMultiplier(0, 10)).toBe(0);
    expect(getCoreRuneMultiplier(1, -3)).toBe(Math.fround(0.02));
  });

  it("computes capped util values", () => {
    expect(getUtilRuneValue(7, 7, 0)).toBe(Math.fround(0.12));
    expect(getUtilRuneValue(7, 7, 10000)).toBe(2);
    expect(getUtilRuneValue(9, 7, 10000)).toBe(0.5);
    expect(getUtilRuneValue(1, 7, 10)).toBe(0);
    expect(getUtilRuneValue(7, 0, 10)).toBe(0);
  });

  it.each([
    [6, 1],
    [7, 2],
    [8, 2],
    [9, 0.5],
    [0, 0],
  ])("computes util cap for type %i", (type, expected) => {
    expect(getUtilCap(type)).toBe(expected);
  });

  it("computes rune economy anchors", () => {
    expect(getEnhanceEssenceCost(0)).toBe(10);
    expect(getEnhanceEssenceCost(1)).toBe(40);
    expect(getEnhanceEssenceCost(4)).toBe(250);
    expect(getEnhanceGoldCost(0)).toBe(1000);
    expect(getEnhanceGoldCost(4)).toBe(25000);
    expect(getDisenchantEssence(1, 0)).toBe(1);
    expect(getDisenchantEssence(4, 0)).toBe(20);
    expect(getDisenchantEssence(6, 0)).toBe(50);
    expect(getDisenchantEssence(7, 3)).toBe(86);
    expect(getDisenchantEssence(0, 3)).toBe(0);
    expect(getShopRuneRollCost(0)).toBe(5000);
    expect(getShopRuneRollCost(10)).toBe(10000);
    expect(getShopRuneRollCost(-5)).toBe(5000);
  });

  it("computes reroll set essence cost anchors", () => {
    expect(getRerollSetEssenceCost(1)).toBe(20);
    expect(getRerollSetEssenceCost(4)).toBe(200);
    expect(getRerollSetEssenceCost(7)).toBe(1500);
    expect(getRerollSetEssenceCost(0)).toBe(0);
    expect(getRerollSetEssenceCost(8)).toBe(0);
  });

  it("reroll set essence cost is positive and monotonic across rarity", () => {
    for (let r = 1; r <= 7; r += 1) {
      expect(getRerollSetEssenceCost(r)).toBeGreaterThan(0);
      if (r < 7) {
        expect(getRerollSetEssenceCost(r + 1)).toBeGreaterThan(
          getRerollSetEssenceCost(r),
        );
      }
    }
  });

  it("computes rarity upgrade cost anchors", () => {
    expect(getRarityUpgradeEssenceCost(1)).toBe(100);
    expect(getRarityUpgradeEssenceCost(6)).toBe(10000);
    expect(getRarityUpgradeGoldCost(1)).toBe(5000);
    expect(getRarityUpgradeGoldCost(6)).toBe(1500000);
  });

  it("rarity upgrade cost is positive and monotonic for upgradable rarities", () => {
    for (let r = 1; r <= 5; r += 1) {
      expect(getRarityUpgradeEssenceCost(r)).toBeGreaterThan(0);
      expect(getRarityUpgradeGoldCost(r)).toBeGreaterThan(0);
      expect(getRarityUpgradeEssenceCost(r + 1)).toBeGreaterThan(
        getRarityUpgradeEssenceCost(r),
      );
      expect(getRarityUpgradeGoldCost(r + 1)).toBeGreaterThan(
        getRarityUpgradeGoldCost(r),
      );
    }
  });

  it("guards Mythic (max rarity) against upgrade", () => {
    // Mythic(7)은 등급 상승 불가 → 비용/확률 모두 0
    expect(getRarityUpgradeEssenceCost(7)).toBe(0);
    expect(getRarityUpgradeGoldCost(7)).toBe(0);
    expect(getRarityUpgradeChance(7)).toBe(0);
    // 유효 범위 밖도 0
    expect(getRarityUpgradeEssenceCost(0)).toBe(0);
    expect(getRarityUpgradeGoldCost(8)).toBe(0);
    expect(getRarityUpgradeChance(0)).toBe(0);
  });

  it("rarity upgrade chance stays in range and decays with rarity", () => {
    const upperBound = Math.fround(0.6);
    for (let r = 1; r <= 6; r += 1) {
      const chance = getRarityUpgradeChance(r);
      expect(chance).toBeGreaterThan(0);
      expect(chance).toBeLessThanOrEqual(upperBound);
      if (r < 6) {
        // 등급↑일수록 성공 확률 단조 감소
        expect(getRarityUpgradeChance(r + 1)).toBeLessThan(chance);
      }
    }
    expect(getRarityUpgradeChance(1)).toBe(Math.fround(0.6));
    expect(getRarityUpgradeChance(6)).toBe(Math.fround(0.05));
  });

  it("computes transfer essence cost anchors", () => {
    expect(getTransferEssenceCost(0)).toBe(50);
    expect(getTransferEssenceCost(1)).toBe(75);
    expect(getTransferEssenceCost(10)).toBe(300);
    // 음수 레벨은 0으로 clamp → 기본값
    expect(getTransferEssenceCost(-5)).toBe(50);
  });

  it("transfer essence cost is monotonic in source level", () => {
    for (let level = 0; level < 20; level += 1) {
      expect(getTransferEssenceCost(level + 1)).toBeGreaterThan(
        getTransferEssenceCost(level),
      );
    }
  });
});
