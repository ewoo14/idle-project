# 환생 특성 (Rebirth Perks) 구현 계획

> 스펙: [`2026-05-30-rebirth-perks-design.md`](../specs/2026-05-30-rebirth-perks-design.md). v3 디스패치, 현행 재검증. SaveVer 23→24.

**Goal:** 환생 횟수 기반 특성 포인트를 6종 특성에 자유 분배(무한 성장 프레스티지 빌드). 기존 환생 포인트 비파괴.

**Architecture:** 서버 `rebirthPerk.ts` parity + 클라 RebirthPerkService(분배/보너스) + GameInstance(rebirthCount 연동) + 환생 특성 패널 UI. SaveVer 23→24.

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/rebirthPerk.ts` 신규: `RebirthPerk` 타입(6종 문자열), `PERK_STEP` 상수(GoldPct 0.02/DropPct 0.02/CritDmgPct 0.03/AllStatPct 0.01/ExpPct 0.02/OfflineEffPct 0.03), `getTotalRebirthPerkPoints(rebirthCount)` = `max(0, rebirthCount)`, `getPerkBonus(perk, level)` = `fround(PERK_STEP[perk] * max(0, level))`. 기존 formulas 패턴.
- [ ] `rebirthPerk.test.ts`: total 단조(rebirthCount), perkBonus level0=0·선형·음수 가드, 6종 step 매핑. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 환생 특성 포인트/보너스 backend parity (환생 특성)`.

## Task 2: client (character)
- [ ] `GameCore/RebirthPerkTypes.h`: `ERebirthPerk{None,GoldPct,DropPct,CritDmgPct,AllStatPct,ExpPct,OfflineEffPct}`, `FRebirthPerkBonus`(6 float 또는 perk별).
- [ ] `GameCore/RebirthPerkService.{h,cpp}`(UObject): `TMap<ERebirthPerk,int32> Allocations`, `int32 TotalPoints`(GameInstance가 rebirthCount로 SetTotal), `int32 GetSpent()`/`GetAvailable()`, `bool AllocatePerk(ERebirthPerk)`(Available>0면 Allocations[perk]++), `bool DeallocatePerk(ERebirthPerk)`, `void ResetPerks()`, `int32 GetPerkLevel(ERebirthPerk)`, `float GetPerkBonus(ERebirthPerk)`(서버 parity 미러 PerkStep×level), `RestoreState(allocations)`. 분배 무결성(Spent ≤ Total).
- [ ] **보너스 적용(단일 지점, #72 가드)**: GoldPct→AddGold(펫/칭호/잠재 골드 옆) / DropPct→드롭(펫 Drop 집계점) / CritDmgPct→크리뎀 / AllStatPct→RefreshDerivedStats / ExpPct→AddExp / OfflineEffPct→오프라인 보상 계산. 각 1곳. (기존 GetEquippedPotential*/Title* 합산 지점 재사용.)
- [ ] GameInstance: RebirthPerkService 보유·초기화, **rebirthCount 변경 시 SetTotal**(환생 시 + 로드 시), AllocatePerk/DeallocatePerk/ResetPerks 진입점(→RefreshDerivedStats·Autosave). getter lazy-ensure(#91). SaveVer **23→24**: Allocations 직렬화(맵 perk→level). Capture=24/Apply `>=24` 가드(RestoreState + SetTotal). CloudSavePayloadMapper 정합. **전 세이브 테스트 단언 23→24 일괄 갱신**(grep `(23)`/`V23`/`v23`/`SaveVersion, 23`/`SaveVersion = 23` in Tests/, 레거시 <24 유지).
- [ ] Automation(`RebirthPerkServiceTests` 신규): AllocatePerk(Available 한도·중복), Deallocate/Reset, GetTotal(rebirthCount), GetPerkBonus(서버 parity), 보너스 적용(Gold/AllStat 등 CP·재화 반영), 세이브 v24 라운드트립, parity(PerkStep 6종). 익명 헬퍼 RebirthPerk~ prefix.
- [ ] 커밋 `feat: 환생 특성 RebirthPerkService + SaveVer24 (환생 특성)`.

## Task 3: UI (designer)
- [ ] 환생 특성 패널: 가용/총 포인트, 6종 특성(이름·현재 레벨·보너스%·+/− 버튼[가용/할당 시 활성]), 리셋 버튼. ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 환생 특성 패널 UI (환생 특성)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/rebirth-perks-balance-note.md`: PerkStep 6종 곡선·포인트 출처(1/환생)·파워크리프/median·기존 환생 포인트 불변. SaveVer24.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 환생 특성 밸런스 노트`.

## Task 5: qa (qa)
- [ ] E2E: 분배(가용 한도·중복 거부)→리셋→보너스 적용(Gold/Drop/CritDmg/AllStat/Exp/Offline CP·재화 반영)→환생 시 포인트 증가→세이브 v24→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 환생 특성 E2E`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 rebirthPerk.ts ↔ 클라 RebirthPerkService(Total/PerkStep) 1:1 — TM cross-check.
- **이중 적용 가드(#72)**: 6종 각 단일 집계 지점, 기존 마스터리/펫/칭호/잠재 옆 합산. 기존 RebirthBonusPoints(HP/공격) **불변**(비파괴).
- 분배 무결성(Available≥0). SaveVer 23→24 전 테스트 단언 갱신(stale 방지, #83). getter lazy-ensure(#91).
- jumbo ODR(RebirthPerk~ prefix).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
