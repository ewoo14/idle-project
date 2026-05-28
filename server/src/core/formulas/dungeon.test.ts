import { describe, expect, it } from "vitest";
import {
  getDailyEntryLimit,
  getDungeonReward,
  getMinimumCp,
} from "./dungeon.js";

describe("dungeon formula", () => {
  it.each([
    [1, 3],
    [2, 3],
    [3, 3],
    [0, 0],
  ])("returns the daily entry limit for dungeon type %i", (type, expected) => {
    expect(getDailyEntryLimit(type)).toBe(expected);
  });

  it.each([
    [1, 100],
    [2, 250],
    [3, 500],
    [0, 0],
  ])("matches the client minimum combat power for type %i", (type, expected) => {
    expect(getMinimumCp(type)).toBe(expected);
  });

  it.each([
    [1, 100, { gold: 1000, exp: 0, essence: 0 }],
    [1, 350, { gold: 3500, exp: 0, essence: 0 }],
    [2, 250, { gold: 0, exp: 1250, essence: 0 }],
    [2, 750, { gold: 0, exp: 3750, essence: 0 }],
    [3, 500, { gold: 0, exp: 0, essence: 50 }],
    [3, 1200, { gold: 0, exp: 0, essence: 120 }],
  ])("returns one-resource rewards for type %i at CP %i", (type, combatPower, expected) => {
    expect(getDungeonReward(type, combatPower)).toEqual(expected);
  });

  it("gates rewards below minimum combat power", () => {
    expect(getDungeonReward(1, 99)).toEqual({ gold: 0, exp: 0, essence: 0 });
    expect(getDungeonReward(2, 249)).toEqual({ gold: 0, exp: 0, essence: 0 });
    expect(getDungeonReward(3, 499)).toEqual({ gold: 0, exp: 0, essence: 0 });
  });

  it("uses fround-compatible scaling anchors and clamps oversized rewards", () => {
    expect(getDungeonReward(1, 101)).toEqual({
      gold: 1010,
      exp: 0,
      essence: 0,
    });
    expect(getDungeonReward(3, Number.MAX_VALUE)).toEqual({
      gold: 0,
      exp: 0,
      essence: 9223372036854776000,
    });
  });
});
