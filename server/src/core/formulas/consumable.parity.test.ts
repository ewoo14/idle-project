import { describe, expect, it } from "vitest";
import {
  CONSUMABLE_GRADE,
  CONSUMABLE_TYPE,
  getConsumableBuffDurationSec,
  getConsumableBuffPercent,
} from "./consumable.js";

const cppFormulaAnchors = [
  {
    name: "AttackTonic",
    type: CONSUMABLE_TYPE.AttackTonic,
    percent: Math.fround(0.3),
    durationSec: 1800,
  },
  {
    name: "GuardTonic",
    type: CONSUMABLE_TYPE.GuardTonic,
    percent: Math.fround(0.3),
    durationSec: 1800,
  },
  {
    name: "AllStatElixir",
    type: CONSUMABLE_TYPE.AllStatElixir,
    percent: Math.fround(0.2),
    durationSec: 1800,
  },
  {
    name: "FortuneScroll",
    type: CONSUMABLE_TYPE.FortuneScroll,
    percent: Math.fround(0.3),
    durationSec: 1800,
  },
  {
    name: "GoldFeast",
    type: CONSUMABLE_TYPE.GoldFeast,
    percent: Math.fround(0.5),
    durationSec: 1800,
  },
  {
    name: "WisdomBooster",
    type: CONSUMABLE_TYPE.WisdomBooster,
    percent: Math.fround(0.5),
    durationSec: 1800,
  },
] as const;

describe("consumable formula parity", () => {
  it.each(cppFormulaAnchors)("matches FConsumableFormula for $name", ({
    type,
    percent,
    durationSec,
  }) => {
    expect(getConsumableBuffPercent(type)).toBe(percent);
    expect(getConsumableBuffDurationSec(type)).toBe(durationSec);
  });

  it("matches FConsumableFormula invalid type guards", () => {
    expect(getConsumableBuffPercent(-1)).toBe(0);
    expect(getConsumableBuffDurationSec(-1)).toBe(0);
    expect(getConsumableBuffPercent(99)).toBe(0);
    expect(getConsumableBuffDurationSec(99)).toBe(0);
  });

  // FConsumableFormula::GetBuffPercent(Type, Grade) 등급 차등 앵커 (0.5 / 1 / 2 배수).
  it.each(
    cppFormulaAnchors,
  )("matches FConsumableFormula grade scaling for $name", ({
    type,
    percent,
  }) => {
    expect(getConsumableBuffPercent(type, CONSUMABLE_GRADE.Standard)).toBe(
      percent,
    );
    expect(getConsumableBuffPercent(type, CONSUMABLE_GRADE.Lesser)).toBe(
      Math.fround(percent * 0.5),
    );
    expect(getConsumableBuffPercent(type, CONSUMABLE_GRADE.Greater)).toBe(
      Math.fround(percent * 2),
    );
    // 지속시간은 등급 무관 고정.
    expect(getConsumableBuffDurationSec(type, CONSUMABLE_GRADE.Lesser)).toBe(
      1800,
    );
    expect(getConsumableBuffDurationSec(type, CONSUMABLE_GRADE.Greater)).toBe(
      1800,
    );
  });
});
