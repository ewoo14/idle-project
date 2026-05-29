import { describe, expect, it } from "vitest";
import {
  CONSUMABLE_GRADE,
  CONSUMABLE_TYPE,
  getConsumableBuffDurationSec,
  getConsumableBuffPercent,
  getConsumableGradeMultiplier,
  isConsumableGrade,
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

  it("defaults to Standard grade for the legacy two-arg path", () => {
    expect(getConsumableBuffPercent(CONSUMABLE_TYPE.AttackTonic)).toBe(
      getConsumableBuffPercent(
        CONSUMABLE_TYPE.AttackTonic,
        CONSUMABLE_GRADE.Standard,
      ),
    );
  });

  it("scales buff percent by grade (Lesser 0.5x / Greater 2x)", () => {
    const standard = getConsumableBuffPercent(
      CONSUMABLE_TYPE.AttackTonic,
      CONSUMABLE_GRADE.Standard,
    );
    expect(standard).toBe(Math.fround(0.3));
    expect(
      getConsumableBuffPercent(
        CONSUMABLE_TYPE.AttackTonic,
        CONSUMABLE_GRADE.Lesser,
      ),
    ).toBe(Math.fround(0.3 * 0.5));
    expect(
      getConsumableBuffPercent(
        CONSUMABLE_TYPE.AttackTonic,
        CONSUMABLE_GRADE.Greater,
      ),
    ).toBe(Math.fround(0.3 * 2));
  });

  it("keeps duration fixed across grades", () => {
    for (const grade of [
      CONSUMABLE_GRADE.Lesser,
      CONSUMABLE_GRADE.Standard,
      CONSUMABLE_GRADE.Greater,
    ]) {
      expect(
        getConsumableBuffDurationSec(CONSUMABLE_TYPE.GoldFeast, grade),
      ).toBe(1800);
    }
  });

  it("exposes grade multipliers and validation guards", () => {
    expect(getConsumableGradeMultiplier(CONSUMABLE_GRADE.Lesser)).toBe(
      Math.fround(0.5),
    );
    expect(getConsumableGradeMultiplier(CONSUMABLE_GRADE.Greater)).toBe(
      Math.fround(2),
    );
    expect(isConsumableGrade(CONSUMABLE_GRADE.Standard)).toBe(true);
    expect(isConsumableGrade(99)).toBe(false);
    expect(getConsumableBuffPercent(CONSUMABLE_TYPE.AttackTonic, 99)).toBe(0);
    expect(getConsumableBuffDurationSec(CONSUMABLE_TYPE.AttackTonic, 99)).toBe(
      0,
    );
  });
});
