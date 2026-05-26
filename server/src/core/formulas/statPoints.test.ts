import { describe, expect, it } from "vitest";
import {
  applyAllocatedStats,
  getStatPointsForLevelUp,
  getTotalStatPointsForLevel,
  STAT_POINTS_PER_LEVEL,
} from "./statPoints.js";
import { deriveStats, type PrimaryStats } from "./stats.js";

describe("stat point formulas", () => {
  it("uses the same fixed stat point grant as the client formula", () => {
    expect(STAT_POINTS_PER_LEVEL).toBe(5);
  });

  it.each([
    [1, 0],
    [2, 5],
    [10, 5],
  ])("grants %i level-up stat points", (newLevel, expected) => {
    expect(getStatPointsForLevelUp(newLevel)).toBe(expected);
  });

  it.each([
    [1, 0],
    [2, 5],
    [10, 45],
  ])("calculates total stat points through level %i", (level, expected) => {
    expect(getTotalStatPointsForLevel(level)).toBe(expected);
  });

  it.each([
    1.5,
    2.5,
    Number.NaN,
    Number.POSITIVE_INFINITY,
  ])("rejects non-integer level %s to mirror the client int32 formula boundary", (level) => {
    expect(() => getStatPointsForLevelUp(level)).toThrow(
      "level must be an integer",
    );
    expect(() => getTotalStatPointsForLevel(level)).toThrow(
      "level must be an integer",
    );
  });

  it("applies allocated primary stats before deriving secondary stats", () => {
    const primary: PrimaryStats = {
      str: 12,
      dex: 6,
      int: 3,
      wis: 3,
      con: 10,
      luk: 4,
    };
    const allocated: PrimaryStats = {
      str: 3,
      dex: 1,
      int: 0,
      wis: 0,
      con: 1,
      luk: 0,
    };

    const baseDerived = deriveStats(primary, 1);
    const allocatedDerived = deriveStats(
      applyAllocatedStats(primary, allocated),
      1,
    );

    expect(applyAllocatedStats(primary, allocated)).toEqual({
      str: 15,
      dex: 7,
      int: 3,
      wis: 3,
      con: 11,
      luk: 4,
    });
    expect(allocatedDerived.physAtk).toBe(baseDerived.physAtk + 6);
    expect(allocatedDerived.hp).toBe(baseDerived.hp + 10);
  });

  it("keeps each allocated primary stat on the same derived stat path as the client", () => {
    const primary: PrimaryStats = {
      str: 12,
      dex: 6,
      int: 3,
      wis: 3,
      con: 10,
      luk: 4,
    };
    const allocated: PrimaryStats = {
      str: 1,
      dex: 0,
      int: 1,
      wis: 1,
      con: 1,
      luk: 1,
    };

    const baseDerived = deriveStats(primary, 1);
    const allocatedDerived = deriveStats(
      applyAllocatedStats(primary, allocated),
      1,
    );

    expect(allocatedDerived.physAtk).toBe(baseDerived.physAtk + 2);
    expect(allocatedDerived.magicAtk).toBe(baseDerived.magicAtk + 2);
    expect(allocatedDerived.hp).toBe(baseDerived.hp + 10);
    expect(allocatedDerived.critRate).toBe(baseDerived.critRate + 0.002);
  });
});
