import { describe, expect, it } from "vitest";
import {
  ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS,
  ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS,
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
    expect(ACHIEVEMENT_MULTIPLIER_SOFT_CAP_START_POINTS).toBe(100);
    expect(ACHIEVEMENT_MULTIPLIER_SOFT_CAP_BONUS_POINTS).toBe(50);
    for (const achievement of ACHIEVEMENTS) {
      expect(achievement.displayNameKey).toMatch(/^ACHIEVEMENT_NAME_/);
    }
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
    expect(
      getTierForValue(
        {
          ...monsterKills,
          baseThreshold: Number.MAX_SAFE_INTEGER,
          growth: 2,
        },
        Number.MAX_SAFE_INTEGER,
      ),
    ).toBe(1);
    expect(getPointsForTiers(monsterKills, 3)).toBe(3);
  });

  it("converts total points to the same soft-capped stat multiplier as the client", () => {
    expect(getAchievementStatMultiplier(0)).toBe(Math.fround(1));
    expect(getAchievementStatMultiplier(3)).toBe(Math.fround(1.03));
    expect(getAchievementStatMultiplier(100)).toBe(Math.fround(2));
    expect(getAchievementStatMultiplier(125)).toBeCloseTo(
      Math.fround(2.1967347),
      6,
    );
    expect(getAchievementStatMultiplier(250)).toBeCloseTo(
      Math.fround(2.4751065),
      6,
    );
    expect(getAchievementStatMultiplier(500)).toBeCloseTo(
      Math.fround(2.4998324),
      6,
    );
    expect(getAchievementStatMultiplier(-5)).toBe(Math.fround(1));
  });
});
