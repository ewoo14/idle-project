import { describe, expect, it } from "vitest";
import {
  coreStatMultiplier,
  critRateAdd,
  dropRateAdd,
  expBoostPct,
  goldFindPct,
  levelFromTotalXp,
  localBonus,
  MASTERY_TRACK_COUNT,
  MASTERY_XP_BASE,
  MASTERY_XP_GROWTH,
  worldPower,
  xpToNext,
} from "./mastery.js";

describe("mastery formula", () => {
  it("uses six mastery tracks", () => {
    expect(MASTERY_TRACK_COUNT).toBe(6);
  });

  it("uses the shared geometric xp curve", () => {
    expect(xpToNext(0)).toBe(MASTERY_XP_BASE);
    expect(xpToNext(1)).toBe(Math.floor(MASTERY_XP_BASE * MASTERY_XP_GROWTH));
    expect(xpToNext(10)).toBeGreaterThan(xpToNext(9));
  });

  it("converts total xp into level progress", () => {
    expect(levelFromTotalXp(0)).toEqual({
      level: 0,
      xpIntoLevel: 0,
      xpToNext: MASTERY_XP_BASE,
    });
    expect(levelFromTotalXp(MASTERY_XP_BASE)).toMatchObject({
      level: 1,
      xpIntoLevel: 0,
    });
    expect(levelFromTotalXp(100_000).level).toBeGreaterThan(
      levelFromTotalXp(1_000).level,
    );
  });

  it("computes monotonic global bonuses", () => {
    expect(coreStatMultiplier(0, 0, 0)).toBe(1);
    expect(coreStatMultiplier(10, 10, 10)).toBeGreaterThan(
      coreStatMultiplier(1, 1, 1),
    );
    expect(critRateAdd(0)).toBe(0);
    expect(dropRateAdd(0)).toBe(0);
    expect(goldFindPct(0)).toBe(0);
    expect(expBoostPct(0)).toBe(0);
    expect(critRateAdd(50)).toBeGreaterThan(0);
    expect(goldFindPct(50)).toBeGreaterThan(goldFindPct(5));
  });

  it("sums world power from track levels", () => {
    expect(worldPower([1, 2, 3, 4, 5, 6])).toBe(21);
  });

  it.each([
    0, 1, 2, 3, 4, 5,
  ])("computes monotonic local bonus for track %i", (track) => {
    expect(localBonus(track, 0)).toBe(0);
    expect(localBonus(track, -1)).toBe(0);
    expect(localBonus(track, 30)).toBeGreaterThan(localBonus(track, 1));
  });

  it("caps equipment local cost reduction at fifty percent", () => {
    expect(localBonus(1, 100)).toBe(Math.fround(0.01 * Math.log(101)));
    expect(localBonus(1, 1e30)).toBe(0.5);
  });
});
