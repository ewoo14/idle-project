import { describe, expect, it } from "vitest";
import {
  DUNGEON_TYPE_ESSENCE,
  DUNGEON_TYPE_EXP,
  DUNGEON_TYPE_GOLD,
  getDungeonReward,
  getMaxAccessibleTier,
  getTierCpRequirement,
} from "./dungeon.js";

type DungeonParityCase = {
  type: number;
  cp: number;
  tier: number;
  requirement: number;
  maxTier: number;
  reward: {
    gold: number;
    exp: number;
    essence: number;
  };
};

const cppParityCases: DungeonParityCase[] = [
  {
    type: DUNGEON_TYPE_GOLD,
    cp: 99,
    tier: 1,
    requirement: 100,
    maxTier: 0,
    reward: { gold: 0, exp: 0, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_GOLD,
    cp: 100,
    tier: 1,
    requirement: 100,
    maxTier: 1,
    reward: { gold: 20000, exp: 0, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_GOLD,
    cp: 399,
    tier: 3,
    requirement: 400,
    maxTier: 2,
    reward: { gold: 0, exp: 0, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_GOLD,
    cp: 400,
    tier: 3,
    requirement: 400,
    maxTier: 3,
    reward: { gold: 120000, exp: 0, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_EXP,
    cp: 1000,
    tier: 3,
    requirement: 1000,
    maxTier: 3,
    reward: { gold: 0, exp: 120000, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_ESSENCE,
    cp: 1999,
    tier: 3,
    requirement: 2000,
    maxTier: 2,
    reward: { gold: 0, exp: 0, essence: 0 },
  },
  {
    type: DUNGEON_TYPE_ESSENCE,
    cp: 2000,
    tier: 3,
    requirement: 2000,
    maxTier: 3,
    reward: { gold: 0, exp: 0, essence: 72 },
  },
  {
    type: DUNGEON_TYPE_ESSENCE,
    cp: 4000,
    tier: 4,
    requirement: 4000,
    maxTier: 4,
    reward: { gold: 0, exp: 0, essence: 136 },
  },
];

describe("dungeon formula parity with FDungeonFormula", () => {
  it.each(
    cppParityCases,
  )("matches C++ tier gate and fround reward anchors for type $type tier $tier cp $cp", ({
    type,
    cp,
    tier,
    requirement,
    maxTier,
    reward,
  }) => {
    expect(getTierCpRequirement(type, tier)).toBe(requirement);
    expect(getMaxAccessibleTier(type, cp)).toBe(maxTier);
    expect(getDungeonReward(type, cp, tier)).toEqual(reward);
  });

  it("keeps tier one equivalent to the legacy two-argument reward call", () => {
    for (const { type, cp } of cppParityCases.filter(
      ({ maxTier }) => maxTier > 0,
    )) {
      expect(getDungeonReward(type, cp, 1)).toEqual(getDungeonReward(type, cp));
    }
  });
});
