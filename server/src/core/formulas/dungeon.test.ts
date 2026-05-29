import { describe, expect, it } from "vitest";
import {
  getDailyEntryLimit,
  getDungeonReward,
  getMaxAccessibleTier,
  getMinimumCp,
  getTierCpRequirement,
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
    [1, 100, { gold: 20000, exp: 0, essence: 0 }],
    [1, 350, { gold: 37417, exp: 0, essence: 0 }],
    [2, 250, { gold: 0, exp: 20000, essence: 0 }],
    [2, 750, { gold: 0, exp: 34641, essence: 0 }],
    [3, 500, { gold: 0, exp: 0, essence: 12 }],
    [3, 1200, { gold: 0, exp: 0, essence: 19 }],
  ])("returns one-resource rewards for type %i at CP %i", (type, combatPower, expected) => {
    expect(getDungeonReward(type, combatPower)).toEqual(expected);
  });

  it("gates rewards below minimum combat power", () => {
    expect(getDungeonReward(1, 99)).toEqual({ gold: 0, exp: 0, essence: 0 });
    expect(getDungeonReward(2, 249)).toEqual({ gold: 0, exp: 0, essence: 0 });
    expect(getDungeonReward(3, 499)).toEqual({ gold: 0, exp: 0, essence: 0 });
  });

  it("returns tier combat power requirements with doubling growth", () => {
    expect(getTierCpRequirement(1, 1)).toBe(100);
    expect(getTierCpRequirement(1, 2)).toBe(200);
    expect(getTierCpRequirement(1, 3)).toBe(400);
    expect(getTierCpRequirement(2, 1)).toBe(250);
    expect(getTierCpRequirement(2, 2)).toBe(500);
    expect(getTierCpRequirement(2, 3)).toBe(1000);
    expect(getTierCpRequirement(3, 1)).toBe(500);
    expect(getTierCpRequirement(3, 2)).toBe(1000);
    expect(getTierCpRequirement(3, 3)).toBe(2000);
  });

  it("returns zero accessible tiers below the gate and logarithmic tiers at boundaries", () => {
    expect(getMaxAccessibleTier(1, 99)).toBe(0);
    expect(getMaxAccessibleTier(1, 100)).toBe(1);
    expect(getMaxAccessibleTier(1, 399)).toBe(2);
    expect(getMaxAccessibleTier(1, 400)).toBe(3);
    expect(getMaxAccessibleTier(3, 1999)).toBe(2);
    expect(getMaxAccessibleTier(3, 2000)).toBe(3);
  });

  it("defaults reward calls to tier one and scales accessible tier rewards linearly", () => {
    const tierOne = getDungeonReward(1, 400, 1);

    expect(getDungeonReward(1, 400)).toEqual(tierOne);
    expect(getDungeonReward(1, 400, 2)).toEqual({
      gold: tierOne.gold * 2,
      exp: 0,
      essence: 0,
    });
    expect(getDungeonReward(2, 1000, 3)).toEqual({
      gold: 0,
      exp: getDungeonReward(2, 1000, 1).exp * 3,
      essence: 0,
    });
  });

  it("returns empty rewards for invalid or inaccessible tiers", () => {
    expect(getDungeonReward(1, 400, 0)).toEqual({
      gold: 0,
      exp: 0,
      essence: 0,
    });
    expect(getDungeonReward(1, 199, 2)).toEqual({
      gold: 0,
      exp: 0,
      essence: 0,
    });
    expect(getDungeonReward(3, 999, 2)).toEqual({
      gold: 0,
      exp: 0,
      essence: 0,
    });
  });

  it("uses fround-compatible scaling anchors and clamps oversized rewards", () => {
    expect(getDungeonReward(1, 101)).toEqual({
      gold: 20100,
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
