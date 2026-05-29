# 던전 난이도 티어 구현 계획

> **For agentic workers:** 본 저장소는 PM→Codex 5-team→Claude 리뷰→TM→머지 v3(각 Task→Codex 파트). 문서/주석/커밋 본문 **한글**, 식별자 영문.

**Goal:** 각 일일 던전(#68)에 CP 게이트 무한 난이도 티어를 추가(높은 티어=높은 CP 요구·큰 보상). 세이브 변경 없음.

**Architecture:** 티어 접근성은 CP 런타임 파생. 서버 `dungeon.ts`에 tier 공식 추가 + 클라 미러, `UDungeonService::TryRunDungeon`에 Tier 인자, 보상 = 기존 sqrt(CP/minCp) × tier 배수. 티어1 = #68 기존 동일(회귀 안전).

**설계 출처:** [`docs/superpowers/specs/2026-05-29-dungeon-tiers-design.md`](../specs/2026-05-29-dungeon-tiers-design.md)

---

## 계약 (양쪽 동일)

```
TIER_CP_FACTOR   = 2.0   // 티어당 CP 요구 배수
getTierCpRequirement(type, tier) = getMinimumCp(type) * TIER_CP_FACTOR^(tier-1)  // tier>=1, fround
getMaxAccessibleTier(type, cp)   = cp < getMinimumCp(type) ? 0 : floor(log(cp/minCp)/log(2)) + 1  // 최대 tier
tierRewardMultiplier(tier)       = max(1, tier)   // 선형, 티어1=×1
getDungeonReward(type, cp, tier):
  tier<1 또는 cp < getTierCpRequirement(type,tier) → {0,0,0}
  기존 getDungeonReward(type,cp) 값 × tierRewardMultiplier(tier)
기존 getDungeonReward(type, cp) 시그니처는 tier 기본 1 오버로드/디폴트로 호환 유지.
```

---

## Task 1: 서버 미러 + parity (backend)
**Files:** Modify `server/src/core/formulas/dungeon.ts` + `dungeon.test.ts`.
- [ ] 실패 테스트: `getTierCpRequirement`(tier1=minCp, tier2=2×minCp, tier3=4×minCp), `getMaxAccessibleTier`(cp<minCp→0, cp=minCp→1, cp=4×minCp→3 경계), `getDungeonReward(type,cp,tier)` = 기존×tier, tier 접근불가→0, **tier1 = 기존 getDungeonReward(type,cp) 동일**.
- [ ] 구현: 계약대로(`Math.fround`/`Math.log`). 기존 2-인자 호출 호환(tier 기본 1).
- [ ] `cd server; npm run lint && npm run test -- dungeon && npm run build` GREEN(**lint 필수**).
- [ ] 커밋 `feat: 던전 티어 서버 미러 (PR #75)`.

## Task 2: 클라 미러 + 서비스 + Automation (character)
**Files:** Modify 클라 던전 reward 계산부(`UDungeonService` / 던전 공식), `GameCore/DungeonService.h/.cpp`, `GameCore/IdleGameInstance.cpp/.h`, `Tests/DungeonServiceTests.cpp`.
- [ ] 클라 던전 보상 공식에 tier 반영(서버 dungeon.ts와 동일값). `GetTierCpRequirement(type,tier)`/`GetMaxAccessibleTier(type,cp)` 추가.
- [ ] `UDungeonService::TryRunDungeon(EDungeonType, int64 CombatPower, const FString& TodayUtc, int32 Tier)` — tier 접근 가능(CP≥요구치) + 입장 잔여 가드, 보상 × tier 배수. 기존 시그니처는 tier 기본 1 유지(회귀).
- [ ] `GameInstance::TryRunDungeon(EDungeonType Type, int32 Tier=1)` — CP 조회 후 서비스 호출. #74 심연 마스터리 로컬 보너스는 기존 적용 지점 유지(티어 배수 후 적용, 단일).
- [ ] Automation: tier 게이트(CP 미달 0)·보상 스케일·**tier1 회귀(#68 기존 동일)**·MaxAccessibleTier 경계·서버 parity 앵커.
- [ ] 커밋 `feat: 던전 티어 클라 + 서비스 (PR #75)`.

## Task 3: UI 티어 선택 (designer)
**Files:** 던전 패널 + ko/en CSV.
- [ ] 던전 패널에 타입별 티어 선택(해금/잠금 = `GetMaxAccessibleTier`, 요구 CP `GetTierCpRequirement`, 예상 보상). 선택 tier로 `TryRunDungeon(Type, Tier)`.
- [ ] ko/en 티어/요구CP/보상 키 + CsvIntegrity.
- [ ] 커밋 `feat: 던전 티어 선택 UI + ko/en (PR #75)`.

## Task 4: 밸런스 (balance)
**Files:** Create `docs/planning/dungeon-tiers-balance-note.md`.
- [ ] TIER_CP_FACTOR(2.0)/선형 보상 근거 + 타입별 티어1~5 CP요구/보상 예시 + 입장 3회/일 페이싱(무한 비폭주). balance-sim median 무변동 확인(던전은 일일 부가 — 코어 페이싱 비침범).
- [ ] 커밋 `docs: 던전 티어 밸런스 노트 (PR #75)`.

## Task 5: QA (qa)
**Files:** `Tests/DungeonServiceTests.cpp` 보강 + `dungeon.parity.test.ts` (있으면 보강/없으면 생성) + qa 노트.
- [ ] tier 게이트·보상 스케일·tier1 회귀·일일 입장 공유(티어 무관 차감)·MaxAccessibleTier 경계·parity(type×tier×CP 경계).
- [ ] 커밋 `test: 던전 티어 E2E/parity (PR #75)`.

---

## Self-Review
- 스펙 §3 공식 → Task 1·2 / 게이트·보상 → Task 2 / UI → Task 3 / balance → Task 4 / parity·회귀 → Task 1·5.
- 세이브 변경 없음(티어 CP 런타임 파생) — SaveVersion 14 유지.
- **회귀 안전**: tier1=#68 기존 보상 동일 + 기존 호출 기본 tier1. #74 심연 마스터리 단일 적용 유지(이중 금지 — #72/#74 교훈).
- Placeholder 없음. `getTierCpRequirement`/`getMaxAccessibleTier`/`tierRewardMultiplier` 명칭 일관.

## 워크플로우 v3 매핑
1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
