import { describe, expect, it } from "vitest";
import {
  CONSUMABLE_TYPE,
  getConsumableBuffDurationSec,
  getConsumableBuffPercent,
} from "./consumable.js";
import {
  getConsumableBuffDurationSec as getConsumableBuffDurationSecFromIndex,
  getConsumableBuffPercent as getConsumableBuffPercentFromIndex,
  CONSUMABLE_TYPE as INDEX_CONSUMABLE_TYPE,
} from "./index.js";

describe("consumable formulas", () => {
  it.each([
    { type: CONSUMABLE_TYPE.AttackTonic, percent: Math.fround(0.3) },
    { type: CONSUMABLE_TYPE.GuardTonic, percent: Math.fround(0.3) },
    { type: CONSUMABLE_TYPE.AllStatElixir, percent: Math.fround(0.2) },
    { type: CONSUMABLE_TYPE.FortuneScroll, percent: Math.fround(0.3) },
    { type: CONSUMABLE_TYPE.GoldFeast, percent: Math.fround(0.5) },
    { type: CONSUMABLE_TYPE.WisdomBooster, percent: Math.fround(0.5) },
  ])("matches client percent anchor for type $type", ({ type, percent }) => {
    expect(getConsumableBuffPercent(type)).toBe(percent);
    expect(getConsumableBuffDurationSec(type)).toBe(1800);
  });

  it("exports consumable formulas through the formula index", () => {
    expect(INDEX_CONSUMABLE_TYPE.AttackTonic).toBe(0);
    expect(getConsumableBuffPercentFromIndex(CONSUMABLE_TYPE.GoldFeast)).toBe(
      Math.fround(0.5),
    );
    expect(
      getConsumableBuffDurationSecFromIndex(CONSUMABLE_TYPE.WisdomBooster),
    ).toBe(1800);
  });

  it("rejects unknown consumable types with no effect", () => {
    expect(getConsumableBuffPercent(99)).toBe(0);
    expect(getConsumableBuffDurationSec(99)).toBe(0);
  });
});
