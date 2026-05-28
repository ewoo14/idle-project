import { describe, expect, it } from "vitest";
import {
  getCoreRuneMultiplier,
  getDisenchantEssence,
  getEnhanceEssenceCost,
  getEnhanceGoldCost,
  getShopRuneRollCost,
  getUtilCap,
  getUtilRuneValue,
  isCoreRuneType,
  isUtilRuneType,
  RUNE_SLOT_COUNT,
} from "./rune.js";

describe("rune formulas", () => {
  it("matches the UE rune slot count", () => {
    expect(RUNE_SLOT_COUNT).toBe(6);
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
    expect(getCoreRuneMultiplier(6, 50)).toBe(Math.fround(1.68));
    expect(getCoreRuneMultiplier(0, 10)).toBe(0);
    expect(getCoreRuneMultiplier(1, -3)).toBe(Math.fround(0.02));
  });

  it("computes capped util values", () => {
    expect(getUtilRuneValue(7, 6, 0)).toBe(Math.fround(0.12));
    expect(getUtilRuneValue(7, 6, 10000)).toBe(2);
    expect(getUtilRuneValue(9, 6, 10000)).toBe(0.5);
    expect(getUtilRuneValue(1, 6, 10)).toBe(0);
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
    expect(getDisenchantEssence(6, 3)).toBe(86);
    expect(getDisenchantEssence(0, 3)).toBe(0);
    expect(getShopRuneRollCost(0)).toBe(5000);
    expect(getShopRuneRollCost(10)).toBe(10000);
    expect(getShopRuneRollCost(-5)).toBe(5000);
  });
});
