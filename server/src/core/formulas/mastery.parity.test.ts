import { describe, expect, it } from "vitest";
import {
  coreStatMultiplier,
  critRateAdd,
  dropRateAdd,
  expBoostPct,
  goldFindPct,
  localBonus,
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
});
