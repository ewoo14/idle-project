# 출석 보상 밸런스 노트

> 대상: 출석 보상 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-attendance-rewards-design.md`](../superpowers/specs/2026-05-30-attendance-rewards-design.md) · SaveVer 22→23

## 1. 출석 체크

- 하루 1회(UTC date, `LastAttendanceDate` 가드) → 누적 출석일(TotalAttendance) +1. 로그인/세이브 시 자동 체크인.
- 미션(#91, 일일 과제)과 직교 — 출석은 "돌아오기만 하면" 누적.

## 2. 마일스톤 (누적 출석일, 무한 기하)

`getAttendanceMilestoneThreshold(n) = floor(2 × 1.6^(n-1))` (f32 fround — n=6→20, n=7→33).

| n | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 누적 출석일 임계 | 2 | 3 | 5 | 8 | 13 | 20 | 33 | 53 |

무한 기하 → 장기 리텐션 목표([[project_infinite_growth]]). #77 주간 보스 마일스톤 패턴 재사용.

## 3. 보상 (3주기 순환)

`getAttendanceMilestoneReward(n)` — n%3: 1=골드 `floor(10000×1.5^(n-1))` / 2=룬 정수 `5n` / 0=소비 `n`.

| n | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 보상 | 골드 10,000 | 정수 10 | 소비 3 | 골드 33,750 | 정수 25 | 소비 6 | 골드 113,906 |

- 수령 1회/마일스톤(ClaimedMilestones), 신규 재화 없음.
- 보상 단일 지급(gold AddGold/essence RuneEssence/consumable AddConsumable). 마일스톤 누락 방지(#77 교훈).

## 4. 경제/페이싱

- 초반 마일스톤(출석 2~5일)은 빠른 첫 보상(온보딩). 골드 보상 기하(1.5^)는 장기 출석 인센티브이되 마일스톤 간격(1.6^)이 늘어 페이싱.
- 무한 성장 곡선과 직교(재화 보충형). median 영향 미미. 수치 초기값 재튜닝.

## 5. 세이브

- **22 → 23**: `AttendanceTotal`/`LastAttendanceDate`/`AttendanceClaimedMilestones`. 누락=0/빈(회귀안전), SaveVer<23 가드.
