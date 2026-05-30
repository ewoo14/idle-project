// 출석 보상(Attendance Rewards) — 서버 parity 미러.
// 출석 체크인/누적 출석일/마일스톤 수령은 클라 로컬 세이브 권위(AttendanceService).
// 서버는 마일스톤 임계/보상 매핑을 1:1 미러링하여 drift 를 방지한다(라우트/DB 없음).
// 주간 보스(weeklyBoss.ts) 마일스톤 함수 패턴을 그대로 모방한다(무한 기하 임계 + getReached 루프).

// 마일스톤 임계 기하 곡선 상수(클라 parity). 누적 출석일 임계 = floor(BASE * GROWTH^(n-1)).
// 예: BASE=2, GROWTH=1.6 → 2,3,5,8,13,21,34,...(무한).
export const ATTENDANCE_MILESTONE_BASE = Math.fround(2);
export const ATTENDANCE_MILESTONE_GROWTH = Math.fround(1.6);

// 보상 기본값(클라 parity). gold 는 기하 성장, essence/consumable 은 비례.
const ATTENDANCE_BASE_GOLD_REWARD = Math.fround(10000);
const ATTENDANCE_GOLD_GROWTH = Math.fround(1.5);
const ATTENDANCE_BASE_ESSENCE_REWARD = Math.fround(5);
const ATTENDANCE_BASE_CONSUMABLE_REWARD = Math.fround(1);

// 보상 종류. 클라 EAttendanceReward(Gold/Essence/Consumable)와 1:1.
export type AttendanceRewardType = "gold" | "essence" | "consumable";

// 무한 루프 안전 상한(weeklyBoss.ts getReachedMilestones 와 동일 가드).
const ATTENDANCE_MILESTONE_LOOP_GUARD = 10_000;

// 마일스톤 n(>=1)의 누적 출석일 임계. n<1 은 0. 무한 기하(클라 floor 곡선과 동일).
export function getAttendanceMilestoneThreshold(n: number): number {
  const milestone = Math.trunc(n);
  if (milestone < 1) {
    return 0;
  }

  return Math.floor(
    Math.fround(
      ATTENDANCE_MILESTONE_BASE *
        Math.fround(ATTENDANCE_MILESTONE_GROWTH ** (milestone - 1)),
    ),
  );
}

// 누적 출석일(total)로 도달한 최대 마일스톤 n. 없으면 0.
// 무한 임계이므로 루프 상한 가드(클라와 동일).
export function getReachedAttendanceMilestones(total: number): number {
  const attendance = Math.max(0, Math.trunc(total));
  let reached = 0;
  let next = 1;

  while (attendance >= getAttendanceMilestoneThreshold(next)) {
    reached = next;
    next += 1;
    if (next > ATTENDANCE_MILESTONE_LOOP_GUARD) {
      break;
    }
  }

  return reached;
}

// 마일스톤 n 의 보상(type/value). n<1 은 gold 0(빈 보상).
// type 순환(gold→essence→consumable→gold...)로 재화 다양성 확보:
//   - gold: 기하 성장(BASE_GOLD * GOLD_GROWTH^(n-1))으로 장기 가치 유지.
//   - essence/consumable: 마일스톤 비례(BASE * n)로 완만 증가.
export function getAttendanceMilestoneReward(n: number): {
  type: AttendanceRewardType;
  value: number;
} {
  const milestone = Math.trunc(n);
  if (milestone < 1) {
    return { type: "gold", value: 0 };
  }

  // 3주기 순환: 1=gold, 2=essence, 0(3,6,..)=consumable.
  const cycle = milestone % 3;
  if (cycle === 1) {
    return {
      type: "gold",
      value: Math.floor(
        Math.fround(
          ATTENDANCE_BASE_GOLD_REWARD *
            Math.fround(ATTENDANCE_GOLD_GROWTH ** (milestone - 1)),
        ),
      ),
    };
  }
  if (cycle === 2) {
    return {
      type: "essence",
      value: Math.floor(
        Math.fround(ATTENDANCE_BASE_ESSENCE_REWARD * milestone),
      ),
    };
  }
  return {
    type: "consumable",
    value: Math.floor(
      Math.fround(ATTENDANCE_BASE_CONSUMABLE_REWARD * milestone),
    ),
  };
}
