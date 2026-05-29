# 일일/주간 미션 (Daily/Weekly Missions) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 리텐션 루프(신규) · 콘텐츠
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속 진행")

## 0. 한 줄 요약
매일/매주 리셋되는 **미션 목표**(처치·스테이지·던전·강화·골드 등)를 달성하면 보상을 받는
리텐션 루프. 기존 메트릭·UTC date(#56)·ISO week(#77) 재사용, 신규 게임플레이(일일/주간 목표).

## 1. 배경
- 반복 콘텐츠: 일일 던전(#68)·주간 보스(#77)·길드(#80) 있으나 **"오늘 할 일 목록"** 형 리텐션 루프 부재.
- `RecordAchievementMetric`가 처치/스테이지/골드/강화/펫 등 **중앙 이벤트 후크**(grep 다수) → 미션 진행 연동 단일 지점.
- 무한 성장과 직교(미션 보상=재화 보충, 영구 성장 아님). [[project_content_richness]].

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 미션 풀 | **고정 풀**(랜덤 아님, parity 단순): 일일 6종 + 주간 4종. 누적형 메트릭만(처치/스테이지/던전실행/골드획득/강화시도/펫먹이 등). |
| 진행 | MissionService 미션별 진행 카운터. `RecordProgress(metric, delta)`(RecordAchievementMetric 중앙 후크에서 호출). 누적형 델타 가산. |
| 리셋 | 일일=UTC date 변경(`GetCurrentUtcDateString`), 주간=ISO week(`GetCurrentUtcWeekString`). 진행/수령 리셋. |
| 보상 | 미션별 골드/룬 정수/소비아이템. **수령 1회/기간**(progress>=target & 미수령). + 일일 전체 완료 보너스(선택). |
| 세이브 | **21→22**(미션 진행 맵·수령 셋·일자/주차 마커, 누락=0 회귀안전) |

## 3. 데이터/공식 (초기값 — balance-note 확정)
```
미션: { id, period(Daily|Weekly), metric, target, rewardType, rewardValue }
예: daily_kill_300(MonstersKilled,300→골드), daily_stage_20(StagesCleared,20→정수),
    daily_dungeon_3(DungeonRuns,3→소비), daily_enhance_10(GearEnhanced,10→골드),
    daily_boss_5(BossesKilled,5→정수), daily_gold_1m(GoldEarned,1e6→소비)
    weekly_kill_5000 / weekly_boss_50 / weekly_stage_150 / weekly_dungeon_15
진행: RecordProgress(metric, delta) → 해당 기간 활성 미션 progress += delta
완료: progress >= target → 수령 가능. ClaimMission(id): 미수령·달성 시 보상 지급+수령 표시
리셋: 새 date/week → 해당 period 미션 progress/claimed 0
```
> `DungeonRuns`는 던전 실행 시 별도 RecordProgress(미션 전용 카운터, 업적 메트릭에 없으면 신설 enum 또는 미션 metric enum 분리).

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| backend | `mission.ts`(미러): `MISSION_CATALOG`(일일6+주간4: id/period/metric/target/reward) + `getMissionReward(id)`. vitest(카탈로그 무결성·보상 매핑). |
| character | `MissionTypes`(EMissionPeriod, EMissionMetric 또는 EAchievementMetric 재사용, FMissionDefinition) + `MissionService`(진행 맵·수령 셋·RecordProgress·ClaimMission·일/주 리셋·GetMissions) + GameInstance 후크(RecordAchievementMetric 중앙점에서 RecordProgress, 던전 실행 시 DungeonRuns, 로그인/세이브 시 리셋 체크). SaveVer **21→22**. Automation(진행·완료·수령 1회·일/주 리셋·세이브 v22·parity). |
| designer | 미션 패널: 일일/주간 탭, 미션별 진행 바·목표·보상·수령 버튼, 리셋까지 시간(선택) + ko/en + CsvIntegrity. 표준 jumbo. |
| balance | `docs/planning/daily-weekly-missions-balance-note.md`: 미션 목표/보상 곡선·일일/주간 페이싱·경제 영향. |
| qa | 진행(메트릭 후크)·완료·수령(1회·미달 거부)·일/주 리셋·세이브 v22·parity. jumbo+게이트. |

## 5. 스코프
**In:** 일일 6+주간 4 고정 미션 + 진행/수령 + 일/주 리셋 + 보상 + UI + parity + SaveVer 22.
**Out:** 랜덤 미션 풀, 미션 새로고침, 일일 로그인 보상(별도), 미션 패스/배틀패스(시즌 #22 별도).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 진행 이중 카운트 | RecordProgress는 RecordAchievementMetric 중앙 후크 1곳 + 던전 전용 1곳. 누적형 델타만. |
| 리셋 타이밍 drift | 기존 UTC date/ISO week 헬퍼 재사용(#56/#68/#77 일관). |
| 수령 중복(치트) | progress>=target & 미수령 셋 검증(클라 권위, 기존 던전/주간보스와 동일 경계). |
| 세이브 마이그레이션 | 진행/수령 누락=0(회귀안전), SaveVer<22 가드. |
