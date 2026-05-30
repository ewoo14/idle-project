import { describe, expect, it } from "vitest";
import {
  computeItemPower,
  computeItemSellValue,
  type ItemPowerInput,
  RARITY_SELL_BASE,
} from "./itemSell.js";

// 매각 대상 아이템 파워 입력의 모든 필드를 0 으로 기본화하는 헬퍼.
// 테스트마다 검증할 필드만 덮어쓴다(클라 기본 장비 보너스=0 parity).
function item(overrides: Partial<ItemPowerInput> = {}): ItemPowerInput {
  return {
    bonusAtk: 0,
    bonusMagicAtk: 0,
    bonusDef: 0,
    bonusPhysDef: 0,
    bonusMagicDef: 0,
    bonusHp: 0,
    bonusAffixHp: 0,
    bonusCritRate: 0,
    bonusCritDmg: 0,
    enhanceLevel: 0,
    ...overrides,
  };
}

describe("computeItemSellValue", () => {
  it("returns the rarity base at enhance level 0", () => {
    expect(computeItemSellValue("Common", 0)).toBe(100);
    expect(computeItemSellValue("Epic", 0)).toBe(1500);
    expect(computeItemSellValue("Mythic", 0)).toBe(400000);
  });

  it("yields zero for the None rarity regardless of enhancement", () => {
    expect(computeItemSellValue("None", 5)).toBe(0);
  });

  it("adds 20% of the base per enhance level", () => {
    expect(computeItemSellValue("Common", 5)).toBe(200);
    expect(computeItemSellValue("Rare", 10)).toBe(1200);
  });

  it("guards negative enhance levels to the base", () => {
    expect(computeItemSellValue("Common", -3)).toBe(100);
  });

  it("exposes the rarity base table with parity values", () => {
    expect(RARITY_SELL_BASE).toEqual({
      None: 0,
      Common: 100,
      Rare: 400,
      Epic: 1500,
      Unique: 6000,
      Legendary: 25000,
      Transcendent: 100000,
      Mythic: 400000,
    });
  });
});

describe("computeItemPower", () => {
  it("sums flat combat stats", () => {
    expect(computeItemPower(item({ bonusAtk: 100, bonusDef: 50 }))).toBe(150);
  });

  it("weights hp at one tenth", () => {
    expect(computeItemPower(item({ bonusHp: 1000 }))).toBe(100);
  });

  it("weights crit rate by one thousand", () => {
    expect(computeItemPower(item({ bonusCritRate: 0.1 }))).toBe(100);
  });

  it("weights crit damage by one hundred", () => {
    expect(computeItemPower(item({ bonusCritDmg: 0.5 }))).toBe(50);
  });

  it("scales power by 10% per enhance level", () => {
    expect(computeItemPower(item({ bonusAtk: 100, enhanceLevel: 10 }))).toBe(
      200,
    );
  });

  it("guards negative enhance levels to no scaling", () => {
    expect(computeItemPower(item({ bonusAtk: 100, enhanceLevel: -5 }))).toBe(
      100,
    );
  });
});
