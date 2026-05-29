# 펫 진화 (별/Star) 구현 계획

> 스펙: [`2026-05-30-pet-evolution-design.md`](../specs/2026-05-30-pet-evolution-design.md). v3 디스패치, 현행 재검증. SaveVer 19→20.

**Goal:** 보유 펫 무한 별 진화(골드 기하 비용) → 장착 펫 보너스 배수. 레벨과 직교한 무한 성장 축.

**Architecture:** 서버 `petBonus.ts` parity 함수 + 클라 PetLevelFormula/PetService 별 맵·진화·배수 적용 + GameInstance 진입점 + 펫 패널 UI. SaveVer 19→20(PetStars).

## Task 1: backend (backend)
- [ ] `server/src/core/formulas/petBonus.ts` 확장(기존 패턴 모방): `getPetEvolveCost(star)` = `floor(PET_EVOLVE_BASE * PET_EVOLVE_GROWTH^star)`, `getPetStarMultiplier(star)` = `1 + PET_STAR_STEP * star`(Math.fround). 상수 export(클라 parity). 음수 star 가드(0).
- [ ] `petBonus.test.ts` 확장: 진화 비용 단조 증가·star0 기본, 별 배수 star0=1·선형 증가, 음수 가드. `cd server; npm run lint && npx vitest run src/core/formulas && npm run build` GREEN.
- [ ] 커밋 `feat: 펫 진화 비용·배수 backend parity (펫 심화)`.

## Task 2: client (character)
- [ ] `PetLevelFormula.{h,cpp}`(또는 PetService 내) 미러: `GetPetEvolveCost(int32 Star)→int64`, `GetPetStarMultiplier(int32 Star)→float`(서버 1:1). 음수 가드.
- [ ] `PetService`: `TMap<FString,int32> PetStars` + `GetPetStar(const FString&)`/`bool EvolvePet(const FString&)`(owned·star++; 골드 검증/차감은 GameInstance). `GetEquippedPetStatBonus`에 장착 펫 `GetPetStarMultiplier(GetPetStar(equipped))` 곱(최종 1곳, 레벨 배수와 분리 곱). `RestoreState` 오버로드에 PetStars 추가(누락=빈 맵).
- [ ] GameInstance: `EvolveEquippedPet()` 또는 `EvolvePet(id)` 진입점 — 골드(GetGold) `GetPetEvolveCost(현재 별)` 검증→차감→PetService::EvolvePet→RefreshDerivedStats→RequestAutosave. **골드 단일 차감**.
- [ ] SaveVer **19→20**: `IdleSaveGame.h` SaveVersion=20 + PetStars 직렬화(맵 → 배열 키/값 또는 기존 PetLevels 직렬화 방식 모방). IdleGameInstance Capture(=20)/Apply(`>=20` 가드, PetStars 복원). CloudSavePayloadMapper 정합. **전 세이브 테스트 단언 19→20 일괄 갱신**(SaveSystem/Consumable/Dungeon/Mastery/Rarity/Rune/RuneSet/Guild 등 grep, 레거시 입력 <20 유지).
- [ ] Automation(`PetServiceTests`/신규): 진화 비용 parity, 별 배수(star0=1/선형), 진화 시 골드 차감·star++·미보유/골드부족 거부, 별 배수 장착 보너스 반영(레벨과 직교), 세이브 v20 라운드트립. 익명 헬퍼 Pet~ prefix.
- [ ] 커밋 `feat: 펫 진화 PetService + SaveVer20 (펫 심화)`.

## Task 3: UI (designer)
- [ ] 펫 패널: 선택/장착 펫 **별 표시**(★N) + **진화 버튼**(비용·다음 별 효과 % 표시, 골드 부족 시 비활성). ko/en + CsvIntegrity. 표준 jumbo.
- [ ] 커밋 `feat: 펫 진화 UI (펫 심화)`.

## Task 4: balance/docs (balance)
- [ ] `docs/planning/pet-evolution-balance-note.md`: 진화 비용 곡선(BASE/GROWTH)·별 배수(STEP)·레벨과 직교·무한 sink·median 영향. SaveVer20.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신. 커밋 `docs: 펫 진화 밸런스 노트 (펫 심화)`.

## Task 5: qa (qa)
- [ ] E2E: 진화(골드 차감·별↑·확정 성공)→미보유/골드부족 거부→별 배수 CP/보너스 반영(레벨 직교)→세이브 v20→parity. 표준 jumbo + ue-automation.ps1 게이트. PR 스크린샷.
- [ ] 커밋 `test: 펫 진화 E2E (펫 심화)`.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: 서버 `getPetEvolveCost`/`getPetStarMultiplier` ↔ 클라 `GetPetEvolveCost`/`GetPetStarMultiplier` — TM cross-check.
- **이중 계산 가드(#72)**: 별 배수는 GetEquippedPetStatBonus 최종 1곳, 장착 펫만. 레벨 배수와 분리 곱(중복 금지).
- SaveVer 19→20 전 테스트 단언 갱신(stale 방지, #83 교훈). PetStars 누락=0성(회귀안전).
- jumbo ODR(Pet~ prefix), 골드 단일 차감.

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
