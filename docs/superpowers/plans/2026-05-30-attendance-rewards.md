# 출석 보상 구현 계획

> 스펙: [`2026-05-30-attendance-rewards-design.md`](../specs/2026-05-30-attendance-rewards-design.md). v3 디스패치, 현행 재검증. SaveVer 22→23.

**Goal:** 하루 1회 출석 → 누적 출석일 무한 마일스톤 보상. 리텐션 루프.

**Architecture:** 서버 `attendance.ts` 마일스톤 parity + 클라 AttendanceService(체크인/마일스톤/수령) + GameInstance + 출석 패널 UI. SaveVer 22→23. 주간 보스(#77) 마일스톤 패턴 재사용.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/attendance.ts` 신규(주간 보스 `weeklyBoss.ts` 마일스톤 함수 패턴 모방): `getAttendanceMilestoneThreshold(n)` = `floor(ATT_BASE * ATT_GROWTH^(n-1))`(ATT_BASE=2/ATT_GROWTH=1.6), `getReachedAttendanceMilestones(total)`(total>=threshold 최대 n), `getAttendanceMilestoneReward(n)` → {type:"gold"|"essence"|"consumable", value}(n 비례/기하). 상수 export(클라 parity).
- [ ] `attendance.test.ts`: 임계 단조 증가·n=1 기본, GetReached 경계(threshold-1/threshold), 보상 매핑, n=0 빈. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 출석 마일스톤 backend parity (출석 보상)`.

## Task 2: client (character)
- [ ] `GameCore/AttendanceTypes.h`: `EAttendanceReward{Gold,Essence,Consumable}`, `FAttendanceMilestone`(N/Threshold/RewardType/RewardValue).
- [ ] `GameCore/AttendanceService.{h,cpp}`(UObject): `int64 TotalAttendance`, `FString LastAttendanceDate`, `TSet<int32> ClaimedMilestones`, `bool CheckIn(const FString& Date)`(Date != LastAttendanceDate면 TotalAttendance++/LastAttendanceDate=Date, true; 이미 출석 false), `int32 GetReachedMilestones() const`(서버 parity), `FAttendanceMilestone GetMilestone(int32 N) const`, `bool ClaimMilestone(int32 N)`(N<=GetReached && !Claimed면 Add true), `RestoreState(total, date, claimed)`. 마일스톤 임계/보상은 서버 parity 미러(PotentialFormula 류 static).
- [ ] GameInstance: AttendanceService 보유·초기화. 로그인(ApplyFromSave)/세이브(CaptureToSave)/주요 진행 시 `CheckIn(UQuestService::GetCurrentUtcDateString())`. `ClaimAttendanceMilestone(N)` 진입점: N<=GetReached && !Claimed 검증 → ClaimMilestone → 보상 지급(gold AddGold/essence RuneEssence/consumable AddConsumable) → RequestAutosave. **보상 단일 지급, #77 교훈(상위 직접 수령 시 중간 누락 방지 — N 단건 수령이라 무관하나 UI가 순차 수령 유도)**.
- [ ] SaveVer **22→23**: TotalAttendance/LastAttendanceDate/ClaimedMilestones 직렬화(기존 Set/FString/int64 패턴). Capture=23/Apply `>=23` 가드(RestoreState + CheckIn). CloudSavePayloadMapper 정합. **전 세이브 테스트 단언 22→23 일괄 갱신**(grep `(22)`/`V22`/`v22`/`SaveVersion, 22`/`SaveVersion = 22` in Tests/, 레거시 <23 유지).
- [ ] Automation(`AttendanceServiceTests` 신규): CheckIn(1일1회·중복 false·날짜 변경 시 ++), GetReachedMilestones 경계, ClaimMilestone(도달 수령·미달 false·중복 false), 보상 지급, 세이브 v23 라운드트립, parity(서버 임계/보상). 익명 헬퍼 Attendance~ prefix.
- [ ] 커밋 `feat: 출석 AttendanceService + SaveVer23 (출석 보상)`.

## Task 3: UI (designer)
- [ ] 출석 패널: 누적 출석일·오늘 체크인 상태(완료/가능)·마일스톤 목록(임계·보상·진행/수령 버튼). ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 출석 패널 UI (출석 보상)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/attendance-rewards-balance-note.md`: 마일스톤 임계 곡선(2×1.6^)·보상 곡선·페이싱·경제 영향. SaveVer23.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 출석 보상 밸런스 노트`.

## Task 5: qa (qa)
- [ ] E2E: 체크인(1일1회·중복 거부·날짜 변경)→마일스톤 도달→수령(미달/중복 거부)→보상 반영→세이브 v23→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 출석 보상 E2E`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 attendance.ts ↔ 클라 AttendanceService(임계/보상/GetReached) 1:1 — TM cross-check.
- 보상 단일 지급(이중 지급 금지). 체크인 1일1회(UTC date 가드, #91/#68 일관).
- SaveVer 22→23 전 테스트 단언 갱신(stale 방지, #83 교훈). 누락=0/빈(회귀안전).
- jumbo ODR(Attendance~ prefix). 마일스톤 누락 방지(#77 교훈).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
