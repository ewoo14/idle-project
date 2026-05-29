# 마스터리 트랙별 로컬 보너스 (V1.5) 구현 계획

> **For agentic workers:** 본 저장소는 PM→Codex 5-team→Claude 리뷰→TM→머지 v3 워크플로우(각 Task→Codex 파트). 문서/주석/커밋 본문 **한글**, 식별자 영문.

**Goal:** PR #72 마스터리 각 트랙이 자기 시스템을 직접 심화하는 로컬 보너스(6종)를 추가한다.

**Architecture:** 중앙화 — `FMasteryFormula::GetLocalBonus(track, level)`(+서버 `localBonus` 미러)가 트랙별 값 산출, `UMasteryService::GetLocalBonus(track)`가 노출, 6개 소비 지점이 읽어 단일 적용. 세이브 변경 없음(트랙 XP는 #72 기존).

**설계 출처:** [`docs/superpowers/specs/2026-05-29-mastery-local-bonuses-design.md`](../specs/2026-05-29-mastery-local-bonuses-design.md)

---

## 계약 (양쪽 동일)

```
GetLocalBonus(track, level) → float  (트랙별 의미, 무한 체감 ln 곡선):
  Combat   : 0.01*ln(1+level)         // 처치 보상 +%
  Equipment: min(0.50, 0.01*ln(1+level)) // 강화 골드 비용 절감 (클램프 0.50)
  Abyss    : 0.01*ln(1+level)         // 던전 보상 +%
  Rune     : 0.01*ln(1+level)         // 룬 코어 효과 가산
  Beast    : 0.01*ln(1+level)         // 펫 보너스 +%
  Explore  : 0.01*ln(1+level)         // 퀘스트 보상 +%
  (level<=0 → 0; Equipment은 절감률 반환=비용×(1-절감))
fround 경계(서버), FMath::Loge(클라).
```

---

## Task 1: 서버 미러 + parity (backend)
**Files:** Modify `server/src/core/formulas/mastery.ts` + `mastery.test.ts`.
- [ ] 실패 테스트: `localBonus(track, level)` 6트랙 0레벨=0, 양수 단조 증가, Equipment 클램프 0.50 상한.
- [ ] 구현: 위 계약(`Math.fround`, `Math.log`). track은 #72 enum 정수(0~5) 또는 문자열 — 기존 mastery.ts 스타일 따름.
- [ ] `cd server; npm run lint && npm run test -- mastery && npm run build` GREEN(**lint 필수, #72 교훈**).
- [ ] 커밋 `feat: 마스터리 로컬 보너스 서버 미러 (PR #74)`.

## Task 2: 클라 공식 + 서비스 노출 + Automation (character)
**Files:** Modify `GameCore/MasteryFormula.h/.cpp`, `MasteryService.h/.cpp`, `Tests/MasteryTests.cpp`.
- [ ] `FMasteryFormula::GetLocalBonus(EMasteryTrack, int32 Level)` — 서버 동일값. Equipment 클램프.
- [ ] `UMasteryService::GetLocalBonus(EMasteryTrack)` = `GetLocalBonus(track, GetTrackLevel(track))` BlueprintPure.
- [ ] Automation: 6트랙 0레벨=0·단조 증가·Equipment 클램프·서버 parity 앵커(level 0/1/30/100).
- [ ] 커밋 `feat: FMasteryFormula 로컬 보너스 + 서비스 노출 (PR #74)`.

## Task 3: 6개 소비 지점 적용 (character)
**Files:** Modify `GameCore/IdleGameInstance.cpp`, `CharacterSystem/IdleCharacter.cpp`(룬 코어), `CharacterSystem/IdleMonster.cpp`(처치 보상).
- [ ] **전투**: 처치 골드·EXP 보상에 `×(1 + GetLocalBonus(Combat))` (IdleMonster 보상 계산 또는 reward 경로, 단일 1회).
- [ ] **장비**: 강화 골드 비용에 `×(1 - GetLocalBonus(Equipment))` (TryEnhanceEquipped 비용 차감 지점, 0 가드).
- [ ] **심연**: 던전 보상에 `×(1 + GetLocalBonus(Abyss))` (TryRunDungeon 보상).
- [ ] **룬**: RefreshDerivedStats 룬 코어 가산에 `+ GetLocalBonus(Rune)` (기존 룬 코어 곱 항에 합류, 별도 항).
- [ ] **야성**: 펫 보너스 적용에 `×(1 + GetLocalBonus(Beast))`.
- [ ] **탐험**: ClaimQuest 보상(골드/EXP)에 `×(1 + GetLocalBonus(Explore))`.
- [ ] **각 단일 적용 지점**(#72/#73 이중적용 교훈 — getter 중복 금지). 적용 후 필요 시 RefreshPlayerCharacterStats.
- [ ] 테스트: 각 트랙 레벨↑ 시 해당 시스템 산출 증가, 0레벨 회귀 없음.
- [ ] 커밋 `feat: 마스터리 로컬 보너스 6개 시스템 적용 (PR #74)`.

## Task 4: UI 표시 (designer)
**Files:** 숙련 패널 + ko/en CSV.
- [ ] 숙련 패널 각 트랙 행에 현재 로컬 보너스 표시(`GetLocalBonus` BlueprintPure) + ko/en 툴팁(트랙별 효과 설명). CsvIntegrity 정합.
- [ ] 커밋 `feat: 숙련 패널 로컬 보너스 표시 + ko/en (PR #74)`.

## Task 5: 밸런스 (balance)
**Files:** Create `docs/planning/mastery-local-bonuses-balance-note.md`.
- [ ] 6 계수 근거 + 강화 절감 클램프(0.50) + 초기/중반/엔드 예시 + 무한 비폭주. balance-sim median 무변동 확인.
- [ ] 커밋 `docs: 마스터리 로컬 보너스 밸런스 노트 (PR #74)`.

## Task 6: QA (qa)
**Files:** `Tests/MasteryTests.cpp` 보강 + `mastery.parity.test.ts` 보강 + qa 노트.
- [ ] 6 트랙 로컬 보너스 단조·클램프·각 시스템 실효 적용·회귀(0레벨)·전역(#72)과 독립. parity 6트랙.
- [ ] 커밋 `test: 마스터리 로컬 보너스 E2E/parity (PR #74)`.

---

## Self-Review
- 스펙 §2 6종 → Task 2(공식)·Task 3(적용) ✅ / §3 단일·비이중 → Task 3 ✅ / 미러·parity → Task 1·6 ✅ / UI → Task 4 ✅ / balance → Task 5 ✅.
- 세이브 변경 없음(트랙 XP #72 기존) — SaveVersion 14 유지.
- Placeholder 없음. `GetLocalBonus`/`localBonus` 명칭 일관. #72/#73 교훈(단일 적용·biome lint) 반영.

## 워크플로우 v3 매핑
| Task | 파트 | / 1→backend, 2·3→character(메인), 4→designer, 5→balance, 6→qa |
