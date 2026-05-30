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
  const highest = Math.max(cleared, Math.trunc(input.highestClearedGlobalStage));
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
