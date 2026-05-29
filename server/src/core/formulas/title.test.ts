import { describe, expect, it } from "vitest";
import {
  getTitleBonus,
  getUnlockedTitles,
  TITLE_CATALOG,
  type TitleBonusType,
} from "./title.js";

// 클라 EAchievementMetric(AchievementFormula.h) enum 이름 — parity 검증용.
const KNOWN_METRICS = new Set<string>([
  "MonstersKilled",
  "BossesKilled",
  "HighestLevelReached",
  "StagesCleared",
  "RebirthCount",
  "TranscendCount",
  "TowerHighestFloor",
  "GearEnhanced",
  "HighestEnhanceLevel",
  "ItemsCollected",
  "UniqueItemsFound",
  "GoldEarned",
  "GoldSpent",
  "GearRollsPurchased",
  "SkillRankUps",
  "HighestSkillRank",
  "PetsFed",
  "HighestPetLevel",
  "QuestsCompleted",
  "SeasonRewardsClaimed",
  "OfflineRewardsClaimed",
  "DaysPlayed",
]);

const VALID_BONUS_TYPES = new Set<TitleBonusType>([
  "AllStatPct",
  "GoldPct",
  "ExpPct",
  "CritDmgPct",
]);

describe("title unlock boundaries", () => {
  it("does not unlock at threshold-1 and unlocks exactly at threshold", () => {
    const monsterHunter = TITLE_CATALOG.find((t) => t.id === "monster_hunter");
    expect(monsterHunter).toBeDefined();
    if (!monsterHunter) {
      return;
    }
    const { metric, threshold } = monsterHunter;

    expect(getUnlockedTitles({ [metric]: threshold - 1 })).not.toContain(
      "monster_hunter",
    );
    expect(getUnlockedTitles({ [metric]: threshold })).toContain(
      "monster_hunter",
    );
    expect(getUnlockedTitles({ [metric]: threshold + 1 })).toContain(
      "monster_hunter",
    );
  });

  it("returns no titles for empty metric values", () => {
    expect(getUnlockedTitles({})).toEqual([]);
  });

  it("treats missing metrics as zero (no unlock)", () => {
    // boss_slayer 만 충족, 누락된 다른 메트릭은 0 으로 처리되어 미해금.
    const bossSlayer = TITLE_CATALOG.find((t) => t.id === "boss_slayer");
    expect(bossSlayer).toBeDefined();
    if (!bossSlayer) {
      return;
    }
    const unlocked = getUnlockedTitles({
      [bossSlayer.metric]: bossSlayer.threshold,
    });
    expect(unlocked).toContain("boss_slayer");
    expect(unlocked).not.toContain("monster_hunter");
  });

  it("unlocks multiple titles across multiple metrics", () => {
    const unlocked = getUnlockedTitles({
      MonstersKilled: 10000,
      BossesKilled: 500,
      RebirthCount: 10,
    });
    expect(unlocked).toEqual(
      expect.arrayContaining([
        "monster_hunter",
        "boss_slayer",
        "rebirth_master",
      ]),
    );
  });

  it("unlocks tiered titles on the same metric together once the higher threshold is met", () => {
    // MonstersKilled 가 더 높은 임계(annihilator)를 넘으면 낮은 임계(hunter)도 함께 해금.
    const unlocked = getUnlockedTitles({ MonstersKilled: 1000000 });
    expect(unlocked).toContain("monster_hunter");
    expect(unlocked).toContain("monster_annihilator");
  });
});

describe("getTitleBonus", () => {
  it("maps every catalog id to its bonus", () => {
    for (const definition of TITLE_CATALOG) {
      expect(getTitleBonus(definition.id)).toEqual({
        type: definition.bonusType,
        value: definition.bonusValue,
      });
    }
  });

  it("returns null for an unknown id", () => {
    expect(getTitleBonus("does_not_exist")).toBeNull();
    expect(getTitleBonus("")).toBeNull();
  });
});

describe("catalog integrity", () => {
  it("has 16~20 titles", () => {
    expect(TITLE_CATALOG.length).toBeGreaterThanOrEqual(16);
    expect(TITLE_CATALOG.length).toBeLessThanOrEqual(20);
  });

  it("has unique ids", () => {
    const ids = TITLE_CATALOG.map((t) => t.id);
    expect(new Set(ids).size).toBe(ids.length);
  });

  it("uses known EAchievementMetric enum names", () => {
    for (const definition of TITLE_CATALOG) {
      expect(KNOWN_METRICS.has(definition.metric)).toBe(true);
    }
  });

  it("uses valid bonus types and bonus values in the +3~20% range", () => {
    for (const definition of TITLE_CATALOG) {
      expect(VALID_BONUS_TYPES.has(definition.bonusType)).toBe(true);
      expect(definition.bonusValue).toBeGreaterThanOrEqual(0.03);
      expect(definition.bonusValue).toBeLessThanOrEqual(0.2);
    }
  });

  it("has positive integer thresholds", () => {
    for (const definition of TITLE_CATALOG) {
      expect(definition.threshold).toBeGreaterThan(0);
      expect(Number.isInteger(definition.threshold)).toBe(true);
    }
  });
});
