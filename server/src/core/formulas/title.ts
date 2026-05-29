// 칭호(Title) 시스템 — 서버 parity 미러.
// 칭호 해금/장착은 클라 로컬 세이브 권위(TitleService). 서버는 카탈로그/해금/보너스
// 매핑을 1:1 미러링하여 drift 를 방지한다(라우트/DB 없음).
// metric 키는 클라 EAchievementMetric(AchievementFormula.h) enum 이름 문자열과 동일.

import type { AchievementMetric } from "./achievement.js";

// 칭호 보너스 차원. 클라 ETitleBonus(None 제외)와 1:1.
export type TitleBonusType = "AllStatPct" | "GoldPct" | "ExpPct" | "CritDmgPct";

export type TitleDefinition = {
  id: string;
  metric: AchievementMetric;
  threshold: number;
  bonusType: TitleBonusType;
  bonusValue: number;
};

// 칭호 카탈로그 — 18종. 전투/진행/탑/수집/경제/펫/퀘스트/강화/메타 카테고리.
// 임계는 합리적 곡선, 보너스는 보수적(+3~20%, bonusValue 0.03~0.20 비율).
// id 는 영문 스네이크. 한글 name 은 클라가 로컬라이즈(여기선 id 만).
// 클라 TitleService.InitializeDefaultTitles 가 이 표를 1:1 이식한다.
const TITLE_DEFINITIONS: readonly TitleDefinition[] = [
  // 전투(Combat)
  {
    id: "monster_hunter",
    metric: "MonstersKilled",
    threshold: 10000,
    bonusType: "AllStatPct",
    bonusValue: 0.03,
  },
  {
    id: "boss_slayer",
    metric: "BossesKilled",
    threshold: 500,
    bonusType: "CritDmgPct",
    bonusValue: 0.1,
  },
  {
    id: "monster_annihilator",
    metric: "MonstersKilled",
    threshold: 1000000,
    bonusType: "AllStatPct",
    bonusValue: 0.1,
  },
  {
    id: "boss_executioner",
    metric: "BossesKilled",
    threshold: 5000,
    bonusType: "CritDmgPct",
    bonusValue: 0.18,
  },
  // 진행(Progression)
  {
    id: "rebirth_master",
    metric: "RebirthCount",
    threshold: 10,
    bonusType: "AllStatPct",
    bonusValue: 0.05,
  },
  {
    id: "transcendent",
    metric: "TranscendCount",
    threshold: 5,
    bonusType: "AllStatPct",
    bonusValue: 0.08,
  },
  {
    id: "stage_conqueror",
    metric: "StagesCleared",
    threshold: 1000,
    bonusType: "ExpPct",
    bonusValue: 0.12,
  },
  {
    id: "level_legend",
    metric: "HighestLevelReached",
    threshold: 500,
    bonusType: "AllStatPct",
    bonusValue: 0.06,
  },
  // 탑(Tower)
  {
    id: "tower_conqueror",
    metric: "TowerHighestFloor",
    threshold: 100,
    bonusType: "GoldPct",
    bonusValue: 0.15,
  },
  {
    id: "tower_overlord",
    metric: "TowerHighestFloor",
    threshold: 300,
    bonusType: "CritDmgPct",
    bonusValue: 0.15,
  },
  // 수집(Collection)
  {
    id: "collector",
    metric: "ItemsCollected",
    threshold: 5000,
    bonusType: "GoldPct",
    bonusValue: 0.1,
  },
  {
    id: "unique_seeker",
    metric: "UniqueItemsFound",
    threshold: 100,
    bonusType: "AllStatPct",
    bonusValue: 0.07,
  },
  // 경제(Economy)
  {
    id: "gold_king",
    metric: "GoldEarned",
    threshold: 1000000000,
    bonusType: "GoldPct",
    bonusValue: 0.2,
  },
  // 펫(Pet)
  {
    id: "pet_whisperer",
    metric: "HighestPetLevel",
    threshold: 50,
    bonusType: "ExpPct",
    bonusValue: 0.08,
  },
  // 퀘스트(Quest)
  {
    id: "quest_champion",
    metric: "QuestsCompleted",
    threshold: 200,
    bonusType: "ExpPct",
    bonusValue: 0.1,
  },
  // 강화(Gear)
  {
    id: "enhance_artisan",
    metric: "HighestEnhanceLevel",
    threshold: 20,
    bonusType: "AllStatPct",
    bonusValue: 0.05,
  },
  {
    id: "enhance_grandmaster",
    metric: "HighestEnhanceLevel",
    threshold: 35,
    bonusType: "CritDmgPct",
    bonusValue: 0.12,
  },
  // 메타(Misc)
  {
    id: "veteran",
    metric: "DaysPlayed",
    threshold: 100,
    bonusType: "GoldPct",
    bonusValue: 0.1,
  },
] as const;

export const TITLE_CATALOG: readonly TitleDefinition[] = TITLE_DEFINITIONS;

// 메트릭 달성값 맵으로 해금된 칭호 id 목록 반환.
// 각 칭호는 (metricValues[metric] ?? 0) >= threshold 면 해금. 누락 메트릭 = 0(미해금).
export function getUnlockedTitles(
  metricValues: Record<string, number>,
): string[] {
  const unlocked: string[] = [];
  for (const definition of TITLE_CATALOG) {
    const value = metricValues[definition.metric] ?? 0;
    if (value >= definition.threshold) {
      unlocked.push(definition.id);
    }
  }
  return unlocked;
}

// 칭호 id 의 장착 보너스(type/value) 조회. 무효 id 는 null.
export function getTitleBonus(
  titleId: string,
): { type: TitleBonusType; value: number } | null {
  const definition = TITLE_CATALOG.find((entry) => entry.id === titleId);
  if (!definition) {
    return null;
  }
  return { type: definition.bonusType, value: definition.bonusValue };
}
