import { describe, expect, it } from "vitest";
import {
  abyssBonusEntries,
  coreStatMultiplier,
  critRateAdd,
  dropRateAdd,
  expBoostPct,
  goldFindPct,
  localBonus,
  localBonus2,
} from "./mastery.js";

type MasteryParityAnchor = {
  level: number;
  coreStatMultiplier: number;
  critRateAdd: number;
  dropRateAdd: number;
  goldFindPct: number;
  expBoostPct: number;
  localBonus: number;
};

const masteryTracks = [
  { id: 0, name: "Combat" },
  { id: 1, name: "Equipment" },
  { id: 2, name: "Abyss" },
  { id: 3, name: "Rune" },
  { id: 4, name: "Beast" },
  { id: 5, name: "Explore" },
];

const anchors: MasteryParityAnchor[] = [
  {
    level: 0,
    coreStatMultiplier: 1,
    critRateAdd: 0,
    dropRateAdd: 0,
    goldFindPct: 0,
    expBoostPct: 0,
    localBonus: 0,
  },
  {
    level: 1,
    coreStatMultiplier: 1.0277259349822998,
    critRateAdd: 0.006931471638381481,
    dropRateAdd: 0.006931471638381481,
    goldFindPct: 0.013862943276762962,
    expBoostPct: 0.013862943276762962,
    localBonus: 0.006931471638381481,
  },
  {
    level: 5,
    coreStatMultiplier: 1.05545175075531,
    critRateAdd: 0.01791759394109249,
    dropRateAdd: 0.01791759394109249,
    goldFindPct: 0.03583518788218498,
    expBoostPct: 0.03583518788218498,
    localBonus: 0.01791759394109249,
  },
  {
    level: 30,
    coreStatMultiplier: 1.0902172327041626,
    critRateAdd: 0.034339871257543564,
    dropRateAdd: 0.034339871257543564,
    goldFindPct: 0.06867974251508713,
    expBoostPct: 0.06867974251508713,
    localBonus: 0.034339871257543564,
  },
  {
    level: 100,
    coreStatMultiplier: 1.1141421794891357,
    critRateAdd: 0.04615120589733124,
    dropRateAdd: 0.04615120589733124,
    goldFindPct: 0.09230241179466248,
    expBoostPct: 0.09230241179466248,
    localBonus: 0.04615120589733124,
  },
];

describe("mastery formula parity with FMasteryFormula", () => {
  it.each(anchors)("matches C++ float anchors at level $level", ({
    level,
    coreStatMultiplier: expectedCore,
    critRateAdd: expectedCrit,
    dropRateAdd: expectedDrop,
    goldFindPct: expectedGold,
    expBoostPct: expectedExp,
    localBonus: expectedLocal,
  }) => {
    expect(coreStatMultiplier(level, level, level)).toBe(expectedCore);
    expect(critRateAdd(level)).toBe(expectedCrit);
    expect(dropRateAdd(level)).toBe(expectedDrop);
    expect(goldFindPct(level)).toBe(expectedGold);
    expect(expBoostPct(level)).toBe(expectedExp);
    for (const { id: track } of masteryTracks) {
      expect(localBonus(track, level)).toBe(expectedLocal);
    }
  });

  it.each(
    masteryTracks,
  )("keeps $name local bonus aligned with FMasteryFormula at level 0/1/30/100", ({
    id: track,
  }) => {
    for (const { level, localBonus: expectedLocal } of anchors.filter(
      (anchor) => [0, 1, 30, 100].includes(anchor.level),
    )) {
      expect(localBonus(track, level)).toBe(expectedLocal);
      expect(localBonus(track, level)).toBe(
        Math.fround(0.01 * Math.log(1 + Math.max(0, level))),
      );
    }
  });

  it("caps Equipment local cost reduction at 0.50 with fround raw input", () => {
    expect(localBonus(1, 1e30)).toBe(0.5);
    expect(localBonus(1, 1e30)).toBe(
      Math.min(0.5, Math.fround(0.01 * Math.log(1 + 1e30))),
    );
  });

  it.each(anchors)("matches C++ localBonus2 float anchors at level $level", ({
    level,
    localBonus: expectedLocal,
  }) => {
    // V2는 1종과 동일 곡선 — 비클램프 트랙(Combat/Abyss/Rune/Explore)에서 동일 float 앵커.
    for (const track of [0, 2, 3, 5]) {
      expect(localBonus2(track, level)).toBe(expectedLocal);
    }
    // Equipment(1)·Beast(4)는 비용 절감 0.50 클램프 — 앵커 레벨에서는 미달이라 동일 값.
    expect(localBonus2(1, level)).toBe(expectedLocal);
    expect(localBonus2(4, level)).toBe(expectedLocal);
  });

  it("caps Equipment and Beast localBonus2 cost reduction at 0.50", () => {
    expect(localBonus2(1, 1e30)).toBe(0.5);
    expect(localBonus2(4, 1e30)).toBe(0.5);
    expect(localBonus2(1, 1e30)).toBe(
      Math.min(0.5, Math.fround(0.01 * Math.log(1 + 1e30))),
    );
  });

  it("mirrors FMasteryFormula::GetAbyssBonusEntries integer thresholds", () => {
    expect(abyssBonusEntries(0)).toBe(0);
    expect(abyssBonusEntries(49)).toBe(0);
    expect(abyssBonusEntries(50)).toBe(1);
    expect(abyssBonusEntries(100)).toBe(2);
    expect(abyssBonusEntries(150)).toBe(3);
    expect(abyssBonusEntries(1e9)).toBe(3);
  });
});
