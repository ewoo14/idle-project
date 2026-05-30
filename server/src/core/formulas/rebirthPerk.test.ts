import { describe, expect, it } from "vitest";
import {
  getPerkBonus,
  getTotalRebirthPerkPoints,
  PERK_STEP,
  type RebirthPerk,
} from "./rebirthPerk.js";

describe("rebirth perk points (source)", () => {
  it("yields zero points at zero rebirths", () => {
    expect(getTotalRebirthPerkPoints(0)).toBe(0);
  });

  it("grants one point per rebirth (monotonic increasing)", () => {
    let previous = -1;
    for (let count = 0; count <= 20; count += 1) {
      const points = getTotalRebirthPerkPoints(count);
      expect(points).toBe(count);
      expect(points).toBeGreaterThan(previous);
      previous = points;
    }
  });

  it("grows without an upper cap (infinite)", () => {
    expect(getTotalRebirthPerkPoints(1000)).toBe(1000);
    expect(getTotalRebirthPerkPoints(1_000_000)).toBe(1_000_000);
  });

  it("truncates fractional rebirth counts", () => {
    expect(getTotalRebirthPerkPoints(3.9)).toBe(3);
    expect(getTotalRebirthPerkPoints(0.5)).toBe(0);
  });

  it("guards negative rebirth counts to zero", () => {
    expect(getTotalRebirthPerkPoints(-1)).toBe(0);
    expect(getTotalRebirthPerkPoints(-1000)).toBe(0);
    expect(getTotalRebirthPerkPoints(-2.5)).toBe(0);
  });
});

describe("rebirth perk bonus", () => {
  const perks: RebirthPerk[] = [
    "GoldPct",
    "DropPct",
    "CritDmgPct",
    "AllStatPct",
    "ExpPct",
    "OfflineEffPct",
  ];

  it("is exactly zero at level 0 for every perk", () => {
    for (const perk of perks) {
      expect(getPerkBonus(perk, 0)).toBe(0);
    }
  });

  it("maps each of the six perks to its PerkStep anchor at level 1", () => {
    expect(getPerkBonus("GoldPct", 1)).toBe(Math.fround(0.02));
    expect(getPerkBonus("DropPct", 1)).toBe(Math.fround(0.02));
    expect(getPerkBonus("CritDmgPct", 1)).toBe(Math.fround(0.03));
    expect(getPerkBonus("AllStatPct", 1)).toBe(Math.fround(0.01));
    expect(getPerkBonus("ExpPct", 1)).toBe(Math.fround(0.02));
    expect(getPerkBonus("OfflineEffPct", 1)).toBe(Math.fround(0.03));
  });

  it("exposes the PerkStep table with the six parity values", () => {
    expect(PERK_STEP).toEqual({
      GoldPct: 0.02,
      DropPct: 0.02,
      CritDmgPct: 0.03,
      AllStatPct: 0.01,
      ExpPct: 0.02,
      OfflineEffPct: 0.03,
    });
  });

  it("scales linearly with the same fround path as the client", () => {
    for (const perk of perks) {
      let previous = -1;
      for (let level = 0; level <= 50; level += 1) {
        const bonus = getPerkBonus(perk, level);
        expect(bonus).toBe(Math.fround(PERK_STEP[perk] * level));
        expect(bonus).toBeGreaterThanOrEqual(previous);
        previous = bonus;
      }
    }
  });

  it("guards negative levels to a zero bonus", () => {
    for (const perk of perks) {
      expect(getPerkBonus(perk, -1)).toBe(0);
      expect(getPerkBonus(perk, -100)).toBe(0);
    }
  });
});
