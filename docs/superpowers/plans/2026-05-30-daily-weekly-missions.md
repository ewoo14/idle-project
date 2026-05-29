# 일일/주간 미션 구현 계획

> 스펙: [`2026-05-30-daily-weekly-missions-design.md`](../specs/2026-05-30-daily-weekly-missions-design.md). v3 디스패치, 현행 재검증. SaveVer 21→22.

**Goal:** 매일/매주 리셋 미션(처치/스테이지/던전/강화/골드) 달성 → 보상. 리텐션 루프.

**Architecture:** 서버 `mission.ts` 카탈로그/parity + 클라 MissionService(진행/수령/리셋) + GameInstance 중앙 후크 + 미션 패널 UI. SaveVer 21→22.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/mission.ts` 신규: `MissionPeriod = "Daily"|"Weekly"`, `MISSION_CATALOG`(일일 6 + 주간 4: id/period/metric(문자열, 클라 EMissionMetric과 동일)/target/rewardType("gold"|"essence"|"consumable")/rewardValue), `getMissionReward(id)`, `getMissionsByPeriod(period)`. 기존 formulas 패턴.
- [ ] `mission.test.ts`: 카탈로그 무결성(id 유니크, target 양수, period 분포 6/4), 보상 매핑, 무효 id null. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 미션 카탈로그 backend parity (일일/주간 미션)`.

## Task 2: client (character)
- [ ] `GameCore/MissionTypes.h`: `EMissionPeriod{Daily,Weekly}`, `EMissionMetric{MonstersKilled,BossesKilled,StagesCleared,DungeonRuns,GearEnhanced,GoldEarned}`(미션 전용, 누적형), `EMissionReward{Gold,Essence,Consumable}`, `FMissionDefinition`(Id/Period/Metric/Target(int64)/RewardType/RewardValue), `FMissionProgress`.
- [ ] `GameCore/MissionService.{h,cpp}`(UObject): `InitializeDefaultMissions`(서버 MISSION_CATALOG 1:1), `TMap<FString,int64> Progress`, `TSet<FString> Claimed`, `FString DailyResetDate`/`WeeklyResetWeek`, `RecordProgress(EMissionMetric, int64 Delta)`(해당 metric 미션 progress += delta), `bool IsComplete(id)`(progress>=target), `bool ClaimMission(id)`(완료&미수령이면 Claimed.Add, 보상은 GameInstance), `EnsurePeriodFresh(date, week)`(date/week 변경 시 해당 period 미션 progress/claimed 리셋·마커 갱신), 접근자, `RestoreState(progress, claimed, date, week)`.
- [ ] GameInstance: MissionService 보유·초기화. **중앙 후크**: `RecordAchievementMetric(metric, amount)` 내부에서 대응 EMissionMetric 있으면 `MissionService->RecordProgress(미션metric, amount)`(MonstersKilled/BossesKilled/StagesCleared/GearEnhanced/GoldEarned 매핑). **던전 실행**(TryRunDungeon 성공) 시 `RecordProgress(DungeonRuns, 1)`. 로그인/세이브/진행 시 `EnsurePeriodFresh(GetCurrentUtcDateString, GetCurrentUtcWeekString)`. `ClaimMission(id)` 진입점(완료·미수령 검증→MissionService::ClaimMission→보상 지급 gold AddGold/essence RuneEssence/consumable AddConsumable→RequestAutosave). **보상 단일 지급 지점**.
- [ ] SaveVer **21→22**: Progress 맵/Claimed 셋/DailyResetDate/WeeklyResetWeek 직렬화. Capture=22/Apply `>=22` 가드(RestoreState). CloudSavePayloadMapper 정합. **전 세이브 테스트 단언 21→22 일괄 갱신**(grep `(21)`/`V21`/`v21`/`SaveVersion, 21`/`SaveVersion = 21` in Tests/, 레거시 <22 유지).
- [ ] Automation(`MissionServiceTests` 신규): RecordProgress 진행, IsComplete 경계, ClaimMission(완료 수령·미달 거부·중복 거부), 일일 리셋(date 변경→daily 0·weekly 유지), 주간 리셋(week 변경→weekly 0), 세이브 v22 라운드트립, parity(서버 카탈로그 target/reward). 익명 헬퍼 Mission~ prefix.
- [ ] 커밋 `feat: 미션 MissionService + SaveVer22 (일일/주간 미션)`.

## Task 3: UI (designer)
- [ ] 미션 패널: 일일/주간 탭, 미션별 진행 바(progress/target)·목표 설명·보상·수령 버튼(완료 시 활성/수령 시 비활성). ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 미션 패널 UI (일일/주간 미션)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/daily-weekly-missions-balance-note.md`: 일일 6/주간 4 미션 목표·보상, 페이싱(일일 달성 난도)·경제 영향(재화 보충량). SaveVer22.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 일일/주간 미션 밸런스 노트`.

## Task 5: qa (qa)
- [ ] E2E: 진행(메트릭/던전 후크)→완료→수령(1회·미달 거부)→일일 리셋(date)→주간 리셋(week)→보상 재화 반영→세이브 v22→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 일일/주간 미션 E2E`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 mission.ts(카탈로그/보상) ↔ 클라 MissionService 정의 1:1(target/reward) — TM cross-check. EMissionMetric ↔ 서버 metric 문자열 일치.
- **이중 카운트 가드**: RecordProgress는 RecordAchievementMetric 중앙 1곳 + 던전 1곳, 누적형 델타만. 보상 단일 지급.
- SaveVer 21→22 전 테스트 단언 갱신(stale 방지, #83 교훈). 진행/수령 누락=0(회귀안전).
- 리셋: 기존 UTC date/ISO week 헬퍼 재사용(#56/#68/#77). jumbo ODR(Mission~ prefix).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
