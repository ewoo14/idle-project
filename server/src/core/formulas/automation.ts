// 통합 자동화 시스템(Automation) — 서버 parity 미러.
// 진행 정책(클리어/사망 시 다음 스테이지 결정)과 기능 해금 게이팅, 효율 업그레이드
// 비용 곡선을 클라(UE5 AutomationService)와 1:1로 미러링하여 drift 를 방지한다.
// 실제 진행/세이브 권위는 클라 로컬이며, 서버는 동일 입력→동일 결정만 보장(라우트/DB 없음).

// 진행 모드. Advance: 클리어 시 전진 / FarmLock: 지정 스테이지 고정 파밍 /
// AutoRetreat: 전진하되 연속 사망 시 한 단계 후퇴.
export type ProgressionMode = "Advance" | "FarmLock" | "AutoRetreat";

// 진행 결정 액션. advance: 다음 스테이지로 / hold: 현 위치 유지 / retreat: 한 단계 후퇴.
export type ProgressionAction = "advance" | "hold" | "retreat";

// 클리어 시점 진행 결정 입력.
export type DecideOnClearInput = {
  mode: ProgressionMode;
  // 방금 클리어한 글로벌 스테이지.
  clearedGlobalStage: number;
  // 지금까지 도달한 최고 클리어 글로벌 스테이지.
  highestClearedGlobalStage: number;
  // FarmLock 모드에서 고정할 스테이지.
  farmLockStage: number;
  // 보스 스테이지 자동 도전 허용 여부.
  autoBossChallenge: boolean;
  // 다음 스테이지가 보스인지 여부.
  nextIsBoss: boolean;
};

// 진행 결정 결과.
export type ProgressionDecision = {
  action: ProgressionAction;
  targetGlobalStage: number;
};

// [min, max] 범위로 클램프(정수 입력 전제).
function clamp(value: number, min: number, max: number): number {
  return Math.min(Math.max(value, min), max);
}

// 클리어 시점의 진행 결정(클라 parity).
// cleared 는 1 하한·절삭, highest 는 cleared 와 입력 최고치 중 큰 값으로 정규화.
export function decideOnClear(input: DecideOnClearInput): ProgressionDecision {
  const cleared = Math.max(1, Math.trunc(input.clearedGlobalStage));
  const highest = Math.max(
    cleared,
    Math.trunc(input.highestClearedGlobalStage),
  );
  const next = cleared + 1;

  // FarmLock: 지정 스테이지 고정. 도달 가능 범위 [1, highest+1] 로 클램프.
  if (input.mode === "FarmLock") {
    return {
      action: "hold",
      targetGlobalStage: clamp(Math.trunc(input.farmLockStage), 1, highest + 1),
    };
  }

  // Advance / AutoRetreat: 다음이 보스이고 자동 도전이 꺼져 있으면 보스 앞에서 정지.
  if (input.nextIsBoss && !input.autoBossChallenge) {
    return { action: "hold", targetGlobalStage: cleared };
  }
  return { action: "advance", targetGlobalStage: next };
}

// 사망 시점 진행 결정 입력.
export type DecideOnDeathInput = {
  mode: ProgressionMode;
  // 현재 도전 중인 글로벌 스테이지.
  currentGlobalStage: number;
  // 연속 사망 횟수.
  consecutiveDeaths: number;
  // 후퇴를 발동시키는 연속 사망 임계치.
  deathThreshold: number;
};

// 사망 시점의 진행 결정(클라 parity).
// 후퇴는 AutoRetreat 모드에서만 발동. 연속 사망이 임계치 이상이고 1단계 초과일 때
// 한 단계 후퇴, 그 외에는 현 위치 유지.
export function decideOnDeath(input: DecideOnDeathInput): ProgressionDecision {
  const current = Math.max(1, Math.trunc(input.currentGlobalStage));

  // AutoRetreat 외 모드는 사망으로 진행이 바뀌지 않는다.
  if (input.mode !== "AutoRetreat") {
    return { action: "hold", targetGlobalStage: current };
  }

  const deaths = Math.max(0, Math.trunc(input.consecutiveDeaths));
  const threshold = Math.max(1, Math.trunc(input.deathThreshold));

  if (deaths >= threshold && current > 1) {
    return { action: "retreat", targetGlobalStage: current - 1 };
  }
  return { action: "hold", targetGlobalStage: current };
}

// 자동화 기능 4종. 값은 키 문자열과 동일(클라 EAutomationFeature parity).
// Progression: 진행 정책 / SkillTactics: 스킬 자동 사용 / AutoGear: 자동 장비 /
// AutoConsumable: 자동 소비 아이템.
export const AUTOMATION_FEATURE = {
  Progression: "Progression",
  SkillTactics: "SkillTactics",
  AutoGear: "AutoGear",
  AutoConsumable: "AutoConsumable",
} as const;

export type AutomationFeature =
  (typeof AUTOMATION_FEATURE)[keyof typeof AUTOMATION_FEATURE];

// 기능별 해금 요구치(클라 parity). 최고 클리어 챕터 + 환생 횟수 양쪽 충족 필요.
const FEATURE_REQUIREMENT: Record<
  AutomationFeature,
  { chapter: number; rebirth: number }
> = {
  Progression: { chapter: 0, rebirth: 0 },
  SkillTactics: { chapter: 3, rebirth: 0 },
  AutoGear: { chapter: 5, rebirth: 0 },
  AutoConsumable: { chapter: 0, rebirth: 1 },
};

// 진척도 입력.
export type AutomationProgress = {
  highestClearedChapter: number;
  rebirthCount: number;
};

// 기능 해금 여부(클라 parity). 챕터·환생 양 조건 모두 충족 시 true.
export function isFeatureUnlocked(
  feature: AutomationFeature,
  progress: AutomationProgress,
): boolean {
  const requirement = FEATURE_REQUIREMENT[feature];
  return (
    Math.trunc(progress.highestClearedChapter) >= requirement.chapter &&
    Math.trunc(progress.rebirthCount) >= requirement.rebirth
  );
}

// 효율 업그레이드 비용 곡선(클라 parity). base × growth^level, 기하 무한 성장(캡 없음).
// 음수 level 은 0 가드(=base). 정수 비용으로 반올림.
export function efficiencyUpgradeCost(
  base: number,
  growth: number,
  level: number,
): number {
  return Math.round(base * growth ** Math.max(0, Math.trunc(level)));
}
