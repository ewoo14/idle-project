// 일일/주간 미션(Daily/Weekly Missions) — 서버 parity 미러.
// 미션 진행/수령/리셋은 클라 로컬 세이브 권위(MissionService). 서버는 카탈로그/보상
// 매핑을 1:1 미러링하여 drift 를 방지한다(라우트/DB 없음).
// metric 키는 클라 EMissionMetric(MissionTypes.h) enum 이름 문자열과 동일(미션 전용 누적형).

// 미션 기간. 클라 EMissionPeriod 와 1:1.
export type MissionPeriod = "Daily" | "Weekly";

// 미션 진행 메트릭(미션 전용 누적형). 클라 EMissionMetric 과 1:1.
// 업적 메트릭과 이름이 겹치는 항목은 의미도 동일(누적 카운터).
export type MissionMetric =
  | "MonstersKilled"
  | "BossesKilled"
  | "StagesCleared"
  | "DungeonRuns"
  | "GearEnhanced"
  | "GoldEarned";

// 미션 보상 종류. 클라 EMissionReward(Gold/Essence/Consumable)와 1:1.
export type MissionRewardType = "gold" | "essence" | "consumable";

export type MissionDefinition = {
  id: string;
  period: MissionPeriod;
  metric: MissionMetric;
  target: number;
  rewardType: MissionRewardType;
  rewardValue: number;
};

// 미션 카탈로그 — 일일 6 + 주간 4 = 10종. 고정 풀(랜덤 아님, parity 단순).
// 일일은 target 소(하루 분량), 주간은 target 대(한 주 누적). 보상은 재화 보충(영구 성장 아님).
// id 는 영문 스네이크. 한글 목표/설명은 클라가 로컬라이즈(여기선 id/수치만).
// 클라 MissionService.InitializeDefaultMissions 가 이 표를 1:1 이식한다.
const MISSION_DEFINITIONS: readonly MissionDefinition[] = [
  // 일일(Daily) — target 소
  {
    id: "daily_kill_300",
    period: "Daily",
    metric: "MonstersKilled",
    target: 300,
    rewardType: "gold",
    rewardValue: 50000,
  },
  {
    id: "daily_stage_20",
    period: "Daily",
    metric: "StagesCleared",
    target: 20,
    rewardType: "essence",
    rewardValue: 5,
  },
  {
    id: "daily_dungeon_3",
    period: "Daily",
    metric: "DungeonRuns",
    target: 3,
    rewardType: "consumable",
    rewardValue: 1,
  },
  {
    id: "daily_enhance_10",
    period: "Daily",
    metric: "GearEnhanced",
    target: 10,
    rewardType: "gold",
    rewardValue: 80000,
  },
  {
    id: "daily_boss_5",
    period: "Daily",
    metric: "BossesKilled",
    target: 5,
    rewardType: "essence",
    rewardValue: 3,
  },
  {
    id: "daily_gold_1m",
    period: "Daily",
    metric: "GoldEarned",
    target: 1000000,
    rewardType: "consumable",
    rewardValue: 1,
  },
  // 주간(Weekly) — target 대
  {
    id: "weekly_kill_5000",
    period: "Weekly",
    metric: "MonstersKilled",
    target: 5000,
    rewardType: "gold",
    rewardValue: 500000,
  },
  {
    id: "weekly_boss_50",
    period: "Weekly",
    metric: "BossesKilled",
    target: 50,
    rewardType: "essence",
    rewardValue: 30,
  },
  {
    id: "weekly_stage_150",
    period: "Weekly",
    metric: "StagesCleared",
    target: 150,
    rewardType: "consumable",
    rewardValue: 3,
  },
  {
    id: "weekly_dungeon_15",
    period: "Weekly",
    metric: "DungeonRuns",
    target: 15,
    rewardType: "gold",
    rewardValue: 800000,
  },
] as const;

export const MISSION_CATALOG: readonly MissionDefinition[] =
  MISSION_DEFINITIONS;

// 미션 id 의 보상(type/value) 조회. 무효 id 는 null.
export function getMissionReward(
  id: string,
): { type: MissionRewardType; value: number } | null {
  const definition = MISSION_CATALOG.find((entry) => entry.id === id);
  if (!definition) {
    return null;
  }
  return { type: definition.rewardType, value: definition.rewardValue };
}

// 기간(Daily|Weekly)별 미션 정의 목록 반환. 카탈로그 순서 유지.
export function getMissionsByPeriod(
  period: MissionPeriod,
): MissionDefinition[] {
  return MISSION_CATALOG.filter((entry) => entry.period === period);
}
