import { describe, expect, it } from "vitest";
import {
  ACHIEVEMENT_POINTS_MULTIPLIER,
  ACHIEVEMENT_TIER_GROWTH_DEFAULT,
  ACHIEVEMENTS,
  getAchievementStatMultiplier,
  getPointsForTiers,
  getTierForValue,
} from "./index.js";

describe("achievement formulas", () => {
  it("mirrors the client achievement catalog breadth", () => {
    expect(ACHIEVEMENTS.length).toBeGreaterThanOrEqual(20);
    expect(
      new Set(ACHIEVEMENTS.map((achievement) => achievement.category)).size,
    ).toBeGreaterThanOrEqual(8);
    expect(ACHIEVEMENT_TIER_GROWTH_DEFAULT).toBe(2);
    expect(ACHIEVEMENT_POINTS_MULTIPLIER).toBe(0.01);
  });

  it("uses geometric tier thresholds matching FAchievementFormula", () => {
    const monsterKills = ACHIEVEMENTS.find(
      (achievement) => achievement.metric === "MonstersKilled",
    );
    expect(monsterKills).toBeDefined();
    if (!monsterKills) {
      return;
    }

    expect(getTierForValue(monsterKills, 9)).toBe(0);
    expect(getTierForValue(monsterKills, 10)).toBe(1);
    expect(getTierForValue(monsterKills, 20)).toBe(2);
    expect(getTierForValue(monsterKills, 40)).toBe(3);
    expect(getPointsForTiers(monsterKills, 3)).toBe(3);
  });

  it("converts total points to the same uncapped stat multiplier as the client", () => {
    expect(getAchievementStatMultiplier(0)).toBe(Math.fround(1));
    expect(getAchievementStatMultiplier(3)).toBe(Math.fround(1.03));
    expect(getAchievementStatMultiplier(125)).toBe(Math.fround(2.25));
    expect(getAchievementStatMultiplier(-5)).toBe(Math.fround(1));
  });
});
