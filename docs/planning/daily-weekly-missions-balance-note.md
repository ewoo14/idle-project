# 일일/주간 미션 밸런스 노트

> 대상: 일일/주간 미션 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-daily-weekly-missions-design.md`](../superpowers/specs/2026-05-30-daily-weekly-missions-design.md) · SaveVer 21→22

## 1. 일일 미션 (6종, target 소, UTC date 리셋)

| id | 목표 | 보상 |
| --- | --- | --- |
| daily_kill_300 | 몬스터 300 처치 | 골드 50,000 |
| daily_stage_20 | 스테이지 20 클리어 | 룬 정수 5 |
| daily_dungeon_3 | 던전 3회 | 소비 1 |
| daily_enhance_10 | 강화 10회 | 골드 80,000 |
| daily_boss_5 | 보스 5 처치 | 룬 정수 3 |
| daily_gold_1m | 골드 1,000,000 획득 | 소비 1 |

## 2. 주간 미션 (4종, target 대, ISO week 리셋)

| id | 목표 | 보상 |
| --- | --- | --- |
| weekly_kill_5000 | 몬스터 5,000 처치 | 골드 500,000 |
| weekly_boss_50 | 보스 50 처치 | 룬 정수 30 |
| weekly_stage_150 | 스테이지 150 클리어 | 소비 3 |
| weekly_dungeon_15 | 던전 15회 | 골드 800,000 |

## 3. 진행/리셋

- 진행: `RecordAchievementMetric` 중앙 후크(MonstersKilled/BossesKilled/StagesCleared/GearEnhanced/GoldEarned) + 던전 실행(DungeonRuns) **단일 지점**, 누적형 델타. 이중 카운트 없음.
- 리셋: 일일=UTC date(`GetCurrentUtcDateString`), 주간=ISO week(`GetCurrentUtcWeekString`) — 기존 던전(#68)/주간보스(#77) 패턴. 각 period 독립 리셋(진행/수령).
- 수령: progress>=target & 미수령 1회(클라 권위, 보상 단일 지급).

## 4. 경제/페이싱

- 일일 보상(골드 5~8만/정수 3~5/소비 1)은 일일 던전(#68)과 유사 규모의 보충 — 하루 활동 유인. 주간은 ~10배 누적.
- 신규 영구 성장 없음(재화 보충형) → 무한 성장 곡선과 직교, median 영향 미미.
- 수치 초기값 — 일일 달성 시간/재화 수급 데이터로 재튜닝.

## 5. 세이브

- **21 → 22**: `MissionProgress`(맵)/`MissionClaimed`(셋)/`MissionDailyResetDate`/`MissionWeeklyResetWeek`. 누락=0/빈(회귀안전), SaveVer<22 가드.
