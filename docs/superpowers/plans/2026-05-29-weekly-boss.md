# 주간 보스 (Weekly Boss) 구현 계획

> **For agentic workers:** v3 워크플로우(각 Task→Codex 파트). 문서/주석/커밋 본문 **한글**, 식별자 영문.

**Goal:** 주간 리셋 보스에 CP 기반 누적 데미지(주 7회)를 쌓아 무한 마일스톤 보상을 받는 주간 엔드게임 루프.

**Architecture:** `FWeeklyBossFormula`↔`weeklyBoss.ts` 미러 + `UWeeklyBossService`(주 리셋=ISO week, 누적/도전/수령) + GameInstance 진입점 + 세이브 14→15.

**설계 출처:** [`docs/superpowers/specs/2026-05-29-weekly-boss-design.md`](../specs/2026-05-29-weekly-boss-design.md)

---

## 계약 (양쪽 동일)
```
WEEKLY_CHALLENGE_LIMIT = 7
getChallengeDamage(combatPower) = max(0, trunc(combatPower))
milestoneThreshold(n) = floor(1000 * 1.5^(n-1))   // n>=1, fround
getReachedMilestones(accumDamage) = accumDamage>=threshold(n)인 최대 n (0이면 미달)
milestoneGoldReward(n) = floor(5000 * 1.5^(n-1))    // balance 확정
milestoneEssenceReward(n) = floor(3 * n)            // balance 확정
```

## Task 1: 서버 미러 + parity (backend)
**Files:** Create `server/src/core/formulas/weeklyBoss.ts` + `weeklyBoss.test.ts`; Modify `index.ts`, `server/src/modules/save/save.schema.ts` + `save.test.ts`.
- [ ] 실패 테스트: 계약 6함수(임계 n1=1000/n2=1500/n3=2250, getReachedMilestones 경계, 보상 단조, challenge damage=cp clamp).
- [ ] 구현(`Math.fround`/`Math.pow`/`Math.floor`), `export * from "./weeklyBoss.js"`, save payload 선택 필드(weeklyBossDamage/weeklyBossWeekId 등).
- [ ] `cd server; npm run lint && npm run test -- weeklyBoss save && npm run build` GREEN(**lint 필수, #65/#72 교훈**).
- [ ] 커밋 `feat: 주간 보스 공식 서버 미러 (PR #77)`.

## Task 2: 클라 공식 + 서비스 + GameInstance + 세이브 (character)
**Files:** Create `GameCore/WeeklyBossTypes.h`, `WeeklyBossFormula.h/.cpp`, `WeeklyBossService.h/.cpp`; Modify `IdleGameInstance.h/.cpp`, `IdleSaveGame.h`, `CloudSavePayloadMapper.cpp`, `Tests/WeeklyBossTests.cpp`, `SaveSystemTests.cpp`.
- [ ] `FWeeklyBossFormula` — 서버 weeklyBoss.ts와 동일값(GetChallengeDamage/MilestoneThreshold/GetReachedMilestones/MilestoneGoldReward/MilestoneEssenceReward).
- [ ] `UWeeklyBossService` — 상태(WeekId/Damage/ChallengesUsed/ClaimedMilestones), `EnsureWeek(CurrentWeek)`(새 주면 리셋), `Challenge(CP, CurrentWeek)`(잔여>0이면 누적+사용++), `ClaimMilestone(n)`(도달&미수령), 접근자(GetRemainingChallenges/GetDamage/GetReachedMilestones), Export/ImportSave.
- [ ] GameInstance: `UWeeklyBossService* WeeklyBossService` + Ensure + `TryChallengeWeeklyBoss()`(GetCurrentUtcWeekString·CP, 결과 반환)·`ClaimWeeklyBossMilestone(n)`(보상 AddGold/RuneEssence)·접근자. Init Ensure. 세이브 Capture/Apply(SaveVersion>=15).
- [ ] `IdleSaveGame.h`: SaveVersion 15 + 4 필드. v14→0 마이그레이션.
- [ ] Automation: 도전 누적·잔여 한도·마일스톤 도달/수령(중복 차단)·주 리셋(주 id 변경 시 초기화)·v14→v15·서버 parity 앵커.
- [ ] 커밋 `feat: 주간 보스 클라 + 서비스 + 세이브 v15 (PR #77)`.

## Task 3: UI 패널 (designer)
**Files:** 주간 보스 패널 + ko/en CSV.
- [ ] 진행(누적 vs 다음 마일스톤 게이지), 도전 버튼+잔여, 마일스톤 목록+수령 CTA, 주 리셋 표시. 도전→`TryChallengeWeeklyBoss`, 수령→`ClaimWeeklyBossMilestone`.
- [ ] ko/en 키 + CsvIntegrity.
- [ ] 커밋 `feat: 주간 보스 패널 UI + ko/en (PR #77)`.

## Task 4: 밸런스 (balance)
**Files:** Create `docs/planning/weekly-boss-v1-balance-note.md`.
- [ ] 도전 제한(7)·데미지(CP)·마일스톤 곡선(1.5^)·보상 근거 + 초기/중반/엔드 도달 마일스톤 예시 + 무한 비폭주 + 주간 cadence 페이싱. balance-sim median 무변동(주간 부가).
- [ ] 커밋 `docs: 주간 보스 V1 밸런스 노트 (PR #77)`.

## Task 5: QA (qa)
**Files:** `Tests/WeeklyBossTests.cpp` 보강 + `weeklyBoss.parity.test.ts` + qa 노트.
- [ ] 도전/누적/한도, 마일스톤 도달·수령·중복 차단, 주 리셋 경계, v14→v15, 회귀, parity(임계/보상 경계).
- [ ] 커밋 `test: 주간 보스 E2E/parity (PR #77)`.

---

## Self-Review
- 스펙 §3 → Task 1·2 / 흐름 → Task 2 / UI → Task 3 / balance → Task 4 / parity·회귀 → Task 1·5.
- 세이브 14→15(누락=0). jumbo(unity) ODR 주의(#76): 신규 익명 헬퍼 동명 grep, **표준 jumbo 빌드 PM 검증**.
- Placeholder 없음. `FWeeklyBossFormula`/`UWeeklyBossService`/`milestoneThreshold` 명칭 일관.

## 워크플로우 v3 매핑
1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
