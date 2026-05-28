# 룬 도감 + 수집 보너스 구현 계획 — PR #62

> **실행 방식:** 워크플로우 v3 ([1]~[N])의 [2] Codex 작업 명세. Codex(character 메인 + backend/designer/balance/qa)가 인터페이스 계약·테스트 앵커를 그대로 구현하고 [3] Claude TM이 parity·정합·트리거 누락을 검증한다.

**목표:** 룬 타입×레어도 54셀 도감 + 셀/행/카테고리 영구 보너스로 메타 수집 벡터 추가.

**아키텍처:** `URuneService::AddRune` 단일 진입점에 unlock 훅 → `RuneCodex` 상태(54비트 또는 엔트리 배열) 갱신. `FRuneCodexFormula` 셀당 가산 + 행/카테고리 마일스톤. `RefreshDerivedStats` 코어 합산에 가산, `GetUtilCap` 조회에 확장 반영. 저장 `SaveVersion` 3→4.

**기술 스택:** UE5 C++ + TypeScript/vitest. `Math.fround` parity. 기존 RuneService/SaveGame 패턴 재사용.

---

## 파일 구조

### 생성 (client)
- `client/Source/IdleProject/RuneSystem/RuneCodexTypes.h` — `FRuneCodexEntry`, `FRuneCodexBonus`, `FRuneCodexCompletion`
- `client/Source/IdleProject/RuneSystem/RuneCodexFormula.h` / `.cpp` — `FRuneCodexFormula`
- `client/Source/IdleProject/Tests/RuneCodexFormulaTests.cpp`
- `client/Source/IdleProject/Tests/RuneCodexServiceTests.cpp`

### 수정 (client)
- `RuneSystem/RuneTypes.h` — (선택) `FRuneCodexEntry` 동일 파일 두면 합칠 수 있음
- `RuneSystem/RuneService.h` / `.cpp` — `OwnedCodex` 상태 + `AddRune` unlock 훅 + `GetCodexCoreBonus`/`GetCodexUtilCapBonus`/`GetCodexCompletion` + Capture/Restore
- `GameCore/IdleSaveGame.h` — `SaveVersion` 3→4 + 도감 필드
- `GameCore/IdleGameInstance.cpp` — Capture/Apply 연동(RuneService 위임)
- `CharacterSystem/IdleCharacter.cpp` `RefreshDerivedStats` — 코어 합산에 `GetCodexCoreBonus` 가산
- `RuneSystem/RuneFormula.cpp` — `GetUtilCap` 호출 시 도감 확장 반영(또는 RuneService에서 cap 조회 합성)
- `UI/IdleHUD.h` / `.cpp` — `FIdleHUDRuneCodexViewModel` + 9×6 도감 그리드
- `Content/Localization/Game/ko/Rune.csv` / `en/Rune.csv` — 도감 키 추가
- `Tests/RuneServiceTests.cpp` — 도감 트리거 관련 케이스 보강(필요시)

### 생성 (server)
- `server/src/core/formulas/runeCodex.ts`
- `server/src/core/formulas/runeCodex.test.ts`

### 수정 (server)
- `server/src/core/formulas/index.ts` — `export * from "./runeCodex.js"`

---

## 인터페이스 계약

### 1. RuneCodexTypes.h
```cpp
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexEntry
{
    GENERATED_BODY()
    UPROPERTY() ERuneType RuneType = ERuneType::None;
    UPROPERTY() EItemRarity Rarity = EItemRarity::None;
    UPROPERTY() bool bUnlocked = false;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexBonus
{
    GENERATED_BODY()
    UPROPERTY() float CoreStatAdd = 0.0f;   // 코어 5스탯에 동일 가산 (Σ cell + Σ row + 카테고리)
    UPROPERTY() float UtilCapExtension = 0.0f; // 유틸 캡 확장 (카테고리 보너스)
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexCompletion
{
    GENERATED_BODY()
    UPROPERTY() int32 UnlockedCells = 0; // 0..54
    UPROPERTY() int32 TotalCells = 54;
    UPROPERTY() TArray<bool> RowComplete; // 길이 6 (레어도별)
    UPROPERTY() bool bCoreCategoryComplete = false; // 코어 30셀 완성
    UPROPERTY() bool bUtilCategoryComplete = false; // 유틸 24셀 완성
};
```

### 2. RuneCodexFormula
```cpp
struct IDLEPROJECT_API FRuneCodexFormula
{
    static constexpr int32 TotalCells = 54;             // 9 * 6
    static constexpr int32 CoreCategoryCells = 30;      // 5 * 6
    static constexpr int32 UtilCategoryCells = 24;      // 4 * 6
    static constexpr float PerCellCoreBonus = 0.004f;   // 셀당 코어 +0.4%
    static constexpr float CoreCategoryBonus = 0.05f;   // 코어 완성 +5%
    static constexpr float UtilCategoryCapExtension = 0.10f; // 유틸 완성 캡 +10%

    static float GetRowCompletionBonus(EItemRarity Rarity); // Common 0.01 .. Mythic 0.12
    static FRuneCodexBonus ComputeBonus(const FRuneCodexCompletion& Completion);
};
```

### 3. URuneService (수정 — 도감 통합)
```cpp
// 헤더 추가:
void UnlockCodexCell(ERuneType Type, EItemRarity Rarity); // AddRune 내부 호출
FRuneCodexCompletion GetCodexCompletion() const;
FRuneCodexBonus GetCodexBonus() const; // = ComputeBonus(GetCodexCompletion())

// CaptureState / RestoreState 시그니처 확장:
void CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots, TArray<FRuneCodexEntry>& OutCodex) const;
void RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots, const TArray<FRuneCodexEntry>& InCodex);

// 멤버:
UPROPERTY() TArray<FRuneCodexEntry> OwnedCodex; // 54엔트리(전체 슬롯 사전 채우기) 또는 unlock된 것만(가변). 구현은 54 사전 채우기로 단순화.

// AddRune에서:
// 기존 sanitize+추가 직후 UnlockCodexCell(Sanitized.RuneType, Sanitized.Rarity) 호출
```

### 4. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 4;          // 3 → 4
UPROPERTY() TArray<FRuneCodexEntry> RuneCodex; // 또는 uint64 RuneCodexBitmap (54비트)
```
`#include "RuneSystem/RuneCodexTypes.h"`.

### 5. RefreshDerivedStats 통합
기존 룬 코어 합산:
```cpp
const FRuneCoreMultipliers M = Rune->GetEquippedCoreMultipliers();
const float CodexCore = Rune->GetCodexBonus().CoreStatAdd; // 신규
Derived.PhysAtk *= (M.PhysAtk + CodexCore);  // M.PhysAtk = 1 + Σ runeBonus, 가산
Derived.MagicAtk *= (M.MagicAtk + CodexCore);
Derived.PhysDef  *= (M.PhysDef  + CodexCore);
Derived.MagicDef *= (M.MagicDef + CodexCore);
Derived.Hp       *= (M.Hp       + CodexCore);
```
(또는 `M.<stat> + CodexCore`를 새 멤버로 묶기 — 시그니처 결정 시 Codex가 일관 적용)

### 6. 유틸 캡 확장
`URuneService::GetEquippedUtilValues`에서 캡 적용 시 `cap + GetCodexBonus().UtilCapExtension` (단, 일관성 위해 Util 캡은 type-wise이므로 카테고리 완성 시 4개 유틸 모두 확장). `FRuneFormula::GetUtilCap`은 순수 유지, RuneService가 합성.

### 7. 서버 runeCodex.ts (parity)
```ts
export const RUNE_CODEX_TOTAL_CELLS = 54;
export const RUNE_CODEX_CORE_CELLS = 30;
export const RUNE_CODEX_UTIL_CELLS = 24;
export const PER_CELL_CORE_BONUS = 0.004;
export const CORE_CATEGORY_BONUS = 0.05;
export const UTIL_CATEGORY_CAP_EXTENSION = 0.10;
export function getRowCompletionBonus(rarity: number): number; // 1..6, fround
export interface RuneCodexBonus { coreStatAdd: number; utilCapExtension: number; }
export interface RuneCodexCompletion { unlockedCells: number; rowComplete: boolean[]; coreCategoryComplete: boolean; utilCategoryComplete: boolean; }
export function computeRuneCodexBonus(c: RuneCodexCompletion): RuneCodexBonus; // 모든 합 fround
```

---

## 테스트 케이스

### 클라 Automation
- 빈 도감 → CoreStatAdd 0, UtilCapExtension 0 (회귀안전)
- AddRune 1회 → 해당 셀 unlock(`bUnlocked=true`), CoreStatAdd += 0.004
- 같은 룬 2회 AddRune → 중복 unlock no-op (셀 1개)
- 분해 후에도 unlock 유지(영구)
- 레어도 행 완성(9 타입 한 레어도) → row 보너스 가산(예 Mythic +0.12)
- 코어 카테고리 30셀 완성 → CoreStatAdd += 0.05
- 유틸 카테고리 24셀 완성 → UtilCapExtension += 0.10 → `GetEquippedUtilValues` 캡 확장 반영
- Capture→Restore: OwnedCodex 라운드트립
- v3→v4 마이그레이션: v3 세이브(도감 필드 없음) Apply → 빈 도감, 보너스 0
- 무효 도감 엔트리(None 타입/None 레어도) 필터 — 새니타이즈
- RefreshDerivedStats: 룬 미장착+도감 비어있음 → 기존 동일, 도감 코어 카테고리 완성 → 5스탯 동일 가산 확인

### 서버 vitest
- `getRowCompletionBonus(1)===0.01`, `(6)===0.12` 앵커
- `computeRuneCodexBonus` 빈 입력 → 0/0
- 완전 도감(54셀, 모든 row, 두 카테고리) → CoreStatAdd = 54*0.004 + Σrow + 0.05 (정확 합산 앵커), UtilCapExtension = 0.10
- `Math.fround` parity 클라 앵커와 일치

---

## Codex 작업 분배

| 파트 | 작업 |
| --- | --- |
| character (메인) | RuneCodexTypes/Formula + URuneService 도감 통합(AddRune 훅·Capture/Restore 확장) + 저장 v4 + RefreshDerivedStats 가산 + 유틸 캡 합성 + Tests |
| backend | runeCodex.ts 미러 + runeCodex.test.ts(앵커/parity) + index.ts export |
| designer | FIdleHUDRuneCodexViewModel + 룬 패널 내 9×6 그리드 + 완성도/마일스톤 표시 + Rune.csv ko/en 도감 키 확장 |
| balance | 셀/행/카테고리 보너스 수치 시뮬 + 페이싱(median) 영향 + 파워크리프 가드. tools/balance-sim 확장 |
| qa | 트리거 누락 점검(드롭/상점/복원 전 경로) + 분해 후 유지 + 마이그레이션 + 코어/유틸 카테고리 경계 |

---

## 워크플로우 v3 단계
- [1] ✅ 기획서·계획 + PR
- [2] Codex character 메인(+보조) → PM 산출 게시
- [3] Claude TM 종합 + 직접 fix (trigger 누락/parity/회귀안전)
- [4] Codex TM+fix → PM 산출 게시
- [5] Claude 검증 (로컬 서버 + UE Build/Automation + CI)
- [N] CI 그린 + PM 종합 소견 + squash merge

## Self-Review
- DoD 1~8 전부 매핑 ✅
- placeholder 없음 (밸런스는 의도적 위임 + 시작값 명시)
- 타입 일관성: `FRuneCodexEntry`/`FRuneCodexBonus`/`FRuneCodexCompletion`/`FRuneCodexFormula`/`runeCodex.ts` 전 섹션 일치
