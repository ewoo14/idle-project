# 소비 아이템 + 임시 버프 구현 계획

> **For agentic workers:** REQUIRED SUB-SKILL: superpowers:subagent-driven-development 또는 executing-plans. 단 본 저장소는 PM→Codex 5-team→Claude 리뷰→TM→머지 v3 워크플로우를 따른다(각 Task→Codex 파트 매핑). 문서/주석/커밋 본문 **한글**, 식별자/`feat:` 영문.

**Goal:** 장비와 별개인 소비 아이템(6종)을 신설하고, 사용 시 일정 시간 능력치·획득량 임시 버프를 부여한다.

**Architecture:** 순수 공식(`FConsumableFormula` ↔ `consumable.ts`) + `UBuffService`(GameInstance 소유, 타입별 보유 수량 + 활성 버프 종료 유닉스타임). 스탯 버프는 `RefreshDerivedStats`의 기존 곱(transcend×tower×achievement×mastery)에 타입별 별도 항으로 합류, 경제 버프는 골드/EXP/드롭 경로(마스터리·펫과 동일 지점). 세이브 v13→v14.

**Tech Stack:** UE5 5.7.4 C++ + Automation / Node 22 TS + Vitest.

**설계 출처:** [`docs/superpowers/specs/2026-05-29-consumables-design.md`](../specs/2026-05-29-consumables-design.md)

---

## 계약 (양쪽 동일)

```
EConsumableType: AttackTonic=0, GuardTonic=1, AllStatElixir=2,
                 FortuneScroll=3, GoldFeast=4, WisdomBooster=5  (총 6)

V1 고정 효과/지속 (FConsumableFormula / consumable.ts):
  AttackTonic   : PhysAtk·MagicAtk ×(1+0.30), 1800초
  GuardTonic    : Hp·PhysDef·MagicDef ×(1+0.30), 1800초
  AllStatElixir : 전 코어 ×(1+0.20), 1800초
  FortuneScroll : DropRateAdd +0.30, 1800초
  GoldFeast     : Gold ×(1+0.50), 1800초
  WisdomBooster : Exp  ×(1+0.50), 1800초
  GetBuffPercent(type) / GetBuffDurationSec(type) — 단일 출처. fround 경계.

스택: 동일 타입 사용 시 EndTime = Now + Duration(연장/갱신), 다른 타입 동시.
활성 판정: Now < EndTime.
```

---

## Task 1: 서버 공식 미러 (backend)

**Files:** Create `server/src/core/formulas/consumable.ts` + `consumable.test.ts`; Modify `index.ts`.

- [ ] 실패 테스트: 6타입 `getBuffPercent`/`getBuffDurationSec` 고정값, 미지정 타입 0/0 가드.
- [ ] 구현: 위 표 상수 매핑(`Math.fround`), `export * from "./consumable.js"`.
- [ ] `cd server; npm run lint && npm run test -- consumable && npm run build` GREEN.
- [ ] 커밋 `feat: 소비 아이템 공식 서버 미러 (PR #73)`.

> **주의(#72 교훈): 반드시 `npm run lint`(biome) 통과 확인** — test/build만 보지 말 것.

---

## Task 2: 클라 공식 `FConsumableFormula` + Automation (character)

**Files:** Create `client/Source/IdleProject/GameCore/ConsumableTypes.h`(`EConsumableType`, `FConsumableSaveEntry{uint8 Type; int32 Count; int64 BuffEndUnixSec;}`), `ConsumableFormula.h/.cpp`; Create `Tests/ConsumableTests.cpp`.

- [ ] 실패 Automation: `GetBuffPercent`/`GetBuffDurationSec` 6타입 기대값(`transcend` 미러 스타일 `Math.fround` 경계 일치), 범위 외 0 가드.
- [ ] 구현: 서버 `consumable.ts`와 동일 상수.
- [ ] `IdleProject.Consumable.Formula` GREEN.
- [ ] 커밋 `feat: FConsumableFormula 클라 공식 + Automation (PR #73)`.

---

## Task 3: `UBuffService` — 보유 수량 + 활성 버프 (character)

**Files:** Create `GameCore/BuffService.h/.cpp`; Modify `Tests/ConsumableTests.cpp`.

API:
```
void Initialize();
int32 GetCount(EConsumableType) const;
void AddConsumable(EConsumableType, int32 Amount);
bool UseConsumable(EConsumableType, int64 NowUnixSec);   // Count>0이면 -1 + EndTime=Now+Duration, true
bool IsBuffActive(EConsumableType, int64 NowUnixSec) const;
int64 GetBuffRemainingSec(EConsumableType, int64 NowUnixSec) const;
float GetBuffStatMultiplier(EConsumableType, int64 NowUnixSec) const;  // 활성 시 1+percent, 아니면 1
float GetGoldBuffPct(int64 Now) const; float GetExpBuffPct(int64 Now) const; float GetDropBuffAdd(int64 Now) const;
TArray<FConsumableSaveEntry> ExportSave() const; void ImportSave(const TArray<FConsumableSaveEntry>&);
DECLARE ... FOnBuffActivated(EConsumableType, int64 EndUnixSec);
```
- [ ] 실패 테스트: Add→Use(Count-1, 버프 활성), 만료(Now≥End → 비활성·곱 1.0), 동일 타입 재사용 연장, ExportSave/ImportSave 라운드트립.
- [ ] 구현 + `IdleProject.Consumable.Service` GREEN.
- [ ] 커밋 `feat: UBuffService 소비 보유/버프 (PR #73)`.

---

## Task 4: GameInstance 연결 + 수급/사용 + 능력치 합류 (character)

**Files:** Modify `GameCore/IdleGameInstance.h/.cpp`, `CharacterSystem/IdleCharacter.cpp`.

- [ ] GameInstance: `UBuffService* BuffService` 보유 + `GetBuffService()` + `EnsureBuffService()`. `Init`에서 Ensure. `GetCurrentUnixSeconds()` 재사용.
- [ ] 사용 API: `UFUNCTION TryUseConsumable(EConsumableType)` → `BuffService->UseConsumable(type, GetCurrentUnixSeconds())` 성공 시 `RefreshPlayerCharacterStats()` + autosave.
- [ ] 수급 훅: 골드 상점 `TryBuyConsumable(EConsumableType)`(#38 곡선), 던전 보상/드롭/퀘스트 보상 경로에서 `BuffService->AddConsumable(...)`.
- [ ] 능력치 합류 — `IdleCharacter.cpp` RefreshDerivedStats (현재 `...×MasteryCoreMultiplier`):
  - `Now = GameInstance->GetCurrentUnixSeconds()` (BlueprintPure 헬퍼 노출 또는 GameInstance getter)
  - AttackTonic → `PhysAtk/MagicAtk *= GetBuffStatMultiplier(AttackTonic, Now)`
  - GuardTonic → `Hp/PhysDef/MagicDef *= GetBuffStatMultiplier(GuardTonic, Now)`
  - AllStatElixir → 전 코어 곱
  - **각 버프 단일 적용 지점**(이중 금지, #72 교훈).
- [ ] 경제: 골드 경로 `*(1+GetGoldBuffPct)`, `AddExp`에 `*(1+GetExpBuffPct)`(마스터리 EXP와 별도 항, getter 중복 금지), 드롭 확률 `+GetDropBuffAdd`.
- [ ] 테스트: 사용→스탯/CP↑·골드/EXP/드롭↑, 만료 후 원복, 미사용 회귀 없음.
- [ ] 커밋 `feat: GameInstance 소비 사용/수급 + 버프 능력치 합류 (PR #73)`.

---

## Task 5: 세이브 v13→v14 (character + backend)

**Files:** Modify `GameCore/IdleSaveGame.h`, `IdleGameInstance.cpp`(Capture/Apply), `GameCore/CloudSavePayloadMapper.cpp`, `Tests/SaveSystemTests.cpp`; `server/src/modules/save/save.schema.ts` + `save.test.ts`.

- [ ] `IdleSaveGame.h`: `SaveVersion=14`, `TArray<FConsumableSaveEntry> Consumables;`
- [ ] Capture: `Consumables = BuffService->ExportSave();` / Apply: `BuffService->ImportSave(SaveVersion>=14 ? Consumables : {})` (v13 누락=0).
- [ ] 클라우드 payload 선택 필드 + 서버 schema(`additionalProperties` 유지, 명시 필드 추가).
- [ ] 테스트: v14 라운드트립(보유+버프 종료시각 보존), **v13→v14 마이그레이션(누락=0)**, 만료 버프 로드 시 비활성.
- [ ] 커밋 `feat: 소비 세이브 v13→v14 + 서버 payload (PR #73)`.

> **주의(#72 교훈):** 환생/초월 리셋 시 소비 보유/버프 처리 정책 명시 — V1은 **리셋 비대상**(보유 유지). 주석 + 리셋 생존 테스트.

---

## Task 6: UI — 소비 인벤토리 + 활성 버프 바 (designer)

**Files:** 소비 패널 위젯 + 버프 바, ko/en CSV, CsvIntegrity.

- [ ] 소비 6종 보유 수량 + 사용 버튼(`TryUseConsumable`). 활성 버프 바(아이콘+`GetBuffRemainingSec` 카운트다운).
- [ ] ko/en 6타입 명칭 + 효과 툴팁 키, `CsvIntegrity` 정합.
- [ ] 커밋 `feat: 소비 인벤토리 + 버프 바 UI + ko/en (PR #73)`.

---

## Task 7: 밸런스 문서 (balance)

**Files:** Create `docs/planning/consumables-v1-balance-note.md`.

- [ ] 버프 %·지속·수급률·상점 가격 근거 + 영구 성장 대비 보조성(버프 미사용 페이싱 불변, balance-sim median 무변동 확인).
- [ ] 커밋 `docs: 소비 V1 밸런스 노트 (PR #73)`.

---

## Task 8: QA (qa)

**Files:** `Tests/ConsumableTests.cpp` 보강 + `consumable.parity.test.ts` + qa 노트.

- [ ] E2E: 사용→버프 활성→스탯/CP·골드/EXP/드롭↑→만료→원복. 스택(동일 연장/다른 동시). 세이브 v13→v14. 리셋 생존. 회귀(미사용 불변).
- [ ] parity: 6타입 %·지속 클라↔서버 일치.
- [ ] 커밋 `test: 소비 E2E/parity/회귀 (PR #73)`.

---

## Self-Review
- 스펙 §3 6종 → Task 2~4 ✅ / §4 적용·이중방지 → Task 4(단일 지점) ✅ / §5 수급·사용 → Task 4 ✅ / §6.1 세이브 v14 → Task 5 ✅ / UI → Task 6 ✅ / balance → Task 7 ✅ / parity·회귀 → Task 1·8 ✅.
- Placeholder 없음. 타입/메서드명(`EConsumableType`, `UBuffService::*`, `FConsumableFormula::*`, `consumable.ts`) Task 전반 일관.
- #72 교훈 반영: EXP/골드 버프 **단일 적용 지점**·getter 중복 금지·biome lint 선확인.

## 워크플로우 v3 매핑
| Task | 파트 |
| --- | --- |
| 1,5(서버) | backend |
| 2~5(클라) | character (메인) |
| 6 | designer |
| 7 | balance |
| 8 | qa |
