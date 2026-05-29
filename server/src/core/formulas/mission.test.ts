import { describe, expect, it } from "vitest";
import {
  getMissionReward,
  getMissionsByPeriod,
  MISSION_CATALOG,
  type MissionMetric,
  type MissionPeriod,
  type MissionRewardType,
} from "./mission.js";

// 클라 EMissionMetric(MissionTypes.h) enum 이름 — parity 검증용(미션 전용 누적형).
const VALID_METRICS = new Set<MissionMetric>([
  "MonstersKilled",
  "BossesKilled",
  "StagesCleared",
  "DungeonRuns",
  "GearEnhanced",
  "GoldEarned",
]);

const VALID_REWARD_TYPES = new Set<MissionRewardType>([
  "gold",
  "essence",
  "consumable",
]);

const VALID_PERIODS = new Set<MissionPeriod>(["Daily", "Weekly"]);

describe("mission catalog integrity", () => {
  it("has unique ids", () => {
    const ids = MISSION_CATALOG.map((m) => m.id);
    expect(new Set(ids).size).toBe(ids.length);
  });

  it("has positive integer targets", () => {
    for (const definition of MISSION_CATALOG) {
      expect(definition.target).toBeGreaterThan(0);
      expect(Number.isInteger(definition.target)).toBe(true);
    }
  });

  it("has positive integer reward values", () => {
    for (const definition of MISSION_CATALOG) {
      expect(definition.rewardValue).toBeGreaterThan(0);
      expect(Number.isInteger(definition.rewardValue)).toBe(true);
    }
  });

  it("distributes 6 Daily and 4 Weekly missions (10 total)", () => {
    expect(MISSION_CATALOG.length).toBe(10);
    const daily = MISSION_CATALOG.filter((m) => m.period === "Daily");
    const weekly = MISSION_CATALOG.filter((m) => m.period === "Weekly");
    expect(daily.length).toBe(6);
    expect(weekly.length).toBe(4);
  });

  it("uses valid periods, metrics and reward types", () => {
    for (const definition of MISSION_CATALOG) {
      expect(VALID_PERIODS.has(definition.period)).toBe(true);
      expect(VALID_METRICS.has(definition.metric)).toBe(true);
      expect(VALID_REWARD_TYPES.has(definition.rewardType)).toBe(true);
    }
  });
});

describe("getMissionReward", () => {
  it("maps every catalog id to its reward", () => {
    for (const definition of MISSION_CATALOG) {
      expect(getMissionReward(definition.id)).toEqual({
        type: definition.rewardType,
        value: definition.rewardValue,
      });
    }
  });

  it("returns null for an unknown id", () => {
    expect(getMissionReward("does_not_exist")).toBeNull();
    expect(getMissionReward("")).toBeNull();
  });
});

describe("getMissionsByPeriod", () => {
  it("returns 6 Daily missions", () => {
    const daily = getMissionsByPeriod("Daily");
    expect(daily.length).toBe(6);
    expect(daily.every((m) => m.period === "Daily")).toBe(true);
  });

  it("returns 4 Weekly missions", () => {
    const weekly = getMissionsByPeriod("Weekly");
    expect(weekly.length).toBe(4);
    expect(weekly.every((m) => m.period === "Weekly")).toBe(true);
  });
});
