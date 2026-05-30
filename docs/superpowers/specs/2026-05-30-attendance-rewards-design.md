# 출석 보상 (Attendance Rewards) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 리텐션 루프(신규) · 무한 성장
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속")

## 0. 한 줄 요약
하루 1회 **출석 체크**로 누적 출석일을 쌓아 **무한 마일스톤 보상**을 받는 리텐션 루프.
주간 보스(#77) 마일스톤 패턴 + UTC date(#56) 재사용. 미션(#91)과 직교(출석 누적 vs 일일 과제).

## 1. 배경
- 리텐션: 일일 던전(#68)·주간 보스(#77)·미션(#91). **장기 누적 출석 보상**(돌아올 이유)이 비어 있음.
- 무한 성장([[project_infinite_growth]]): 누적 출석 → 무한 기하 마일스톤(주간 보스 패턴 재사용).
- [[project_content_richness]]: 출석 → 재화 보충 + 장기 목표.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 출석 | **하루 1회**(UTC date, `lastAttendanceDate` 가드) → 누적 출석일 +1 |
| 마일스톤 | 누적 출석일 ≥ 임계 → 보상 수령(무한 기하 임계, 주간 보스 패턴). 수령 1회/마일스톤. |
| 보상 | 마일스톤 비례 골드/룬 정수/소비(기하 또는 비례). 신규 재화 없음. |
| 연속(streak) | V1 제외(누적만, 단순·무한). streak 보너스는 후속. |
| 세이브 | **22→23**(lastAttendanceDate/totalAttendance/claimedMilestones, 누락=0/빈 회귀안전) |

## 3. 공식 (초기값 — balance-note 확정)
```
AttendanceMilestoneThreshold(n) = floor(ATT_BASE * ATT_GROWTH^(n-1))   # 누적 출석일, 무한
  예 ATT_BASE=2, ATT_GROWTH=1.6 → 2,3,5,8,13,21,34,...
GetReachedMilestones(total) = total >= threshold(n) 인 최대 n
AttendanceMilestoneReward(n): 골드/에센스/소비 = base * n (또는 기하) — balance 확정
CheckIn(date): date != lastAttendanceDate 면 totalAttendance++, lastAttendanceDate=date (1일 1회)
ClaimMilestone(n): n <= GetReachedMilestones(total) && 미수령 → 보상 지급 + 수령 표시
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `attendance.ts`(미러): `getAttendanceMilestoneThreshold(n)`, `getAttendanceMilestoneReward(n)`, `getReachedAttendanceMilestones(total)`. 주간 보스 마일스톤 함수 패턴 재사용. vitest. |
| character | `AttendanceService`(totalAttendance/lastAttendanceDate/claimedMilestones, `CheckIn(date)` 1일1회, `GetReachedMilestones`, `ClaimMilestone(n)`) + GameInstance(로그인/세이브 시 `CheckIn(GetCurrentUtcDateString)`, ClaimMilestone 진입점 보상 단일 지급). SaveVer **22→23**. Automation(체크인 1일1회·마일스톤 도달·수령 1회·세이브 v23·parity). |
| designer | 출석 패널: 누적 출석일·오늘 체크인 상태·마일스톤 목록(임계/보상/수령) + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/attendance-rewards-balance-note.md`: 마일스톤 임계/보상 곡선·페이싱·경제 영향. SaveVer23. |
| qa | 체크인(1일1회·중복 거부)·마일스톤 도달/수령(미달·중복 거부)·세이브 v23·parity. jumbo+게이트. |

## 5. 스코프
**In:** 일일 출석 체크 + 누적 무한 마일스톤 보상 + UI + parity + SaveVer 23.
**Out:** 연속 출석(streak) 보너스, 월간 캘린더형 보상, 복귀 유저 보상(후속).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 출석 중복(치트) | `lastAttendanceDate` UTC date 1일1회 가드(던전/미션과 동일 경계). |
| 마일스톤 보상 유실 | 이전 수령+1~요청 누적 지급(#77 교훈 — 상위 마일스톤 직접 수령 시 중간 누락 방지). |
| 리셋 타이밍 | UTC date(`GetCurrentUtcDateString`) 재사용(#56/#68/#91 일관). |
| 세이브 마이그레이션 | 누락=0/빈(회귀안전), SaveVer<23 가드. |
