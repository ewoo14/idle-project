# 장비 심화 (강화 리스크 + 잠재 레이어) 구현 계획

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 고단계 강화에 단계 하락 리스크·보호서·천장을 도입하고, 드롭 어픽스와 분리된 잠재 레이어를 큐브로 리롤하는 통합 장비 심화 슬라이스를 구현한다.

**Architecture:** 정착된 A안 패턴 — 서버 TypeScript 공식이 source of truth, UE5 C++가 동일 수치를 미러, 양쪽 패리티 테스트 + balance-sim 으로 검증. 게임플레이는 클라이언트 권위(형제 시스템과 동일), 서버는 공식 미러. 신규 아이템 필드는 `FItemInstance` 확장으로 세이브에 자동 직렬화, 신규 자원은 `UIdleSaveGame` 개별 필드. SaveVersion 11→12.

**Tech Stack:** UE5.7.4 C++ (AutomationSpec 테스트), Node.js 22 + TypeScript + vitest, `tools/balance-sim`.

> **스펙:** `docs/superpowers/specs/2026-05-28-equipment-depth-design.md`
> **언어 규칙:** 모든 문서/주석/커밋 본문은 한글. 코드 식별자·`feat:`/`docs:` prefix는 영문.

---

## 파일 구조 (생성/수정 맵)

### 서버 (source of truth)
- 수정: `server/src/core/formulas/enhance.ts` — 안전/위험 구간 판정, 천장, 보호 분기 추가
- 수정: `server/src/core/formulas/enhance.test.ts` — 신규 헬퍼 테스트
- 생성: `server/src/core/formulas/potential.ts` — 잠재 등급/줄/롤/큐브 공식
- 생성: `server/src/core/formulas/potential.test.ts`
- 수정: `server/src/core/formulas/drop.ts` — PowerScore(`computeItemPowerScore`)에 잠재 항 추가
- 수정: `server/src/core/formulas/drop.test.ts` — 잠재 포함 PowerScore 앵커
- 수정: `server/src/core/formulas/index.ts` — `potential.js` re-export

### 클라이언트 (미러 + 게임플레이)
- 수정: `client/Source/IdleProject/ItemSystem/ItemTypes.h` — `FItemInstance` 필드 + `EPotentialGrade`/`EPotentialStat`/`FPotentialLine` enum·struct, PowerScore 항
- 수정: `client/Source/IdleProject/ItemSystem/EnhanceFormula.h/.cpp` — 구간/천장/보호 헬퍼
- 생성: `client/Source/IdleProject/ItemSystem/PotentialFormula.h/.cpp` — 잠재 공식 미러
- 수정: `client/Source/IdleProject/ItemSystem/InventoryComponent.h/.cpp` — 강화 결과 적용(하락/천장/보호), 잠재 적용, 잠금 토글, 자동장착 잠금 존중
- 수정: `client/Source/IdleProject/GameCore/IdleGameInstance.h/.cpp` — `TryEnhanceEquipped` 리스크/보호 처리, `TryRerollPotential`, 자원 보유/차감, 세이브 v11→v12 마이그레이션
- 수정: `client/Source/IdleProject/GameCore/IdleSaveGame.h` — `SaveVersion=12`, 자원 필드(`ProtectionScrolls`/`ResetCubes`/`RankCubes`)
- 수정: `client/Source/IdleProject/GameCore/CloudSavePayloadMapper.cpp` — 신규 자원 페이로드 반영
- 수정: `client/Source/IdleProject/UI/IdleHUD.h/.cpp` — 강화 패널(구간/보호/천장), 잠재 패널(신규), 잠금 토글
- 수정: `client/Content/Localization/Game/ko/UI.csv` — 신규 한글 카피
- 테스트: `client/Source/IdleProject/Tests/EnhanceFormulaTests.cpp`(신규/확장), `PotentialFormulaTests.cpp`(신규), `InventoryTests.cpp`(확장), `SaveSystemTests.cpp`(마이그레이션)

### balance-sim & 문서
- 수정: `tools/balance-sim/` — 신규 강화 실패 모델 import 및 +50 도달 기대비용/시간 재산출
- 수정: `docs/planning/05-balance-philosophy.md`, `docs/planning/01-game-design.md`
- 생성: `docs/planning/slices/NN-equipment-depth.md` (슬라이스 문서, NN은 다음 PR 번호)

---

## Phase 0 — 데이터 모델 & 세이브 기반

### Task 0.1: `FItemInstance` 잠재/리스크/잠금 필드 + enum·struct 추가

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/ItemTypes.h`
- Test: `client/Source/IdleProject/Tests/PotentialFormulaTests.cpp` (다음 Task에서 사용할 타입 정의)

- [ ] **Step 1: `ItemTypes.h`에 잠재 enum/struct 추가** (`EUniqueTrait` 아래, `FItemInstance` 위)

```cpp
UENUM(BlueprintType)
enum class EPotentialGrade : uint8
{
	None = 0 UMETA(Hidden),
	Rare = 1 UMETA(DisplayName = "Rare"),
	Epic = 2 UMETA(DisplayName = "Epic"),
	Unique = 3 UMETA(DisplayName = "Unique"),
	Legendary = 4 UMETA(DisplayName = "Legendary")
};

UENUM(BlueprintType)
enum class EPotentialStat : uint8
{
	None = 0 UMETA(Hidden),
	PhysAtkPct = 1, MagicAtkPct = 2, AllStatPct = 3,
	CritRate = 4, CritDmg = 5, HpPct = 6, DefPct = 7,
	GoldGainPct = 8, DropRatePct = 9
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FPotentialLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EPotentialStat Stat = EPotentialStat::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	float Value = 0.0f;
};
```

- [ ] **Step 2: `FItemInstance`에 필드 추가** (`EnhanceLevel` 아래)

```cpp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	EPotentialGrade PotentialGrade = EPotentialGrade::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	FPotentialLine PotentialLine3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item", meta = (ClampMin = "0"))
	int32 EnhanceFailStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Item")
	bool bLocked = false;
```

- [ ] **Step 3: 컴파일 확인 (PowerScore 항은 Task 3.x에서 추가)**

Run: 클라이언트 빌드 (참조: `reference_ue_headless_verify` 메모리의 빌드 명령)
Expected: 컴파일 성공, 기존 PowerScore 테스트 영향 없음(신규 필드 기본값 0/None).

- [ ] **Step 4: Commit**

```bash
git add client/Source/IdleProject/ItemSystem/ItemTypes.h
git commit -m "feat(item): 잠재/강화 리스크/잠금 필드를 FItemInstance에 추가"
```

### Task 0.2: 세이브 자원 필드 + SaveVersion 12 마이그레이션

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleSaveGame.h:21` (SaveVersion), 신규 자원 필드
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` (마이그레이션 분기 ~373행, SaveVersion 설정)
- Test: `client/Source/IdleProject/Tests/SaveSystemTests.cpp`

- [ ] **Step 1: 실패 테스트 — v11 세이브 로드 시 신규 자원 기본값 0, v12로 승격**

`SaveSystemTests.cpp`에 AutomationSpec 추가:

```cpp
It("v11 세이브를 로드하면 신규 자원이 0이고 SaveVersion이 12로 승격된다", [this]()
{
    UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
    Save->SaveVersion = 11;
    Save->bHasSave = true;
    // 마이그레이션 진입점 호출(프로젝트 기존 마이그레이션 헬퍼와 동일 경로)
    UIdleGameInstance::MigrateSaveGame(Save); // 기존 마이그레이션 함수명에 맞춰 조정
    TestEqual(TEXT("ProtectionScrolls"), Save->ProtectionScrolls, (int64)0);
    TestEqual(TEXT("ResetCubes"), Save->ResetCubes, (int64)0);
    TestEqual(TEXT("RankCubes"), Save->RankCubes, (int64)0);
    TestEqual(TEXT("SaveVersion"), Save->SaveVersion, 12);
});
```

> 주의: `MigrateSaveGame`는 실제 프로젝트의 마이그레이션 진입 함수명으로 교체한다(현재 `IdleGameInstance.cpp` ~43, ~373행의 SaveVersion 처리 흐름 참조).

- [ ] **Step 2: 테스트 실행 → 실패 확인** (필드/버전 없음으로 컴파일 또는 단언 실패)

- [ ] **Step 3: `IdleSaveGame.h` 수정**

```cpp
	int32 SaveVersion = 12;   // 11 → 12
	// ... RuneEssence 아래에 추가:
	UPROPERTY()
	int64 ProtectionScrolls = 0;

	UPROPERTY()
	int64 ResetCubes = 0;

	UPROPERTY()
	int64 RankCubes = 0;
```

- [ ] **Step 4: `IdleGameInstance.cpp` 마이그레이션** — 로드 시 `SaveVersion < 12`면 신규 자원 0 보장 후 `SaveVersion = 12` 설정(기존 `< 7` 분기와 동일 패턴, 저장 시 `SaveGame->SaveVersion = 12`로 변경).

- [ ] **Step 5: 테스트 실행 → 통과 확인**

- [ ] **Step 6: Commit**

```bash
git add client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/Tests/SaveSystemTests.cpp
git commit -m "feat(save): 보호서/큐브 자원 필드 + SaveVersion 11→12 마이그레이션"
```

### Task 0.3: 클라우드 세이브 페이로드에 신규 자원 반영

**Files:**
- Modify: `client/Source/IdleProject/GameCore/CloudSavePayloadMapper.cpp`
- Test: `client/Source/IdleProject/Tests/SaveSystemTests.cpp`

- [ ] **Step 1: 실패 테스트** — 자원이 채워진 SaveGame → 페이로드 직렬화 → 역직렬화 시 자원 보존.

```cpp
It("클라우드 페이로드 왕복에서 보호서/큐브 자원이 보존된다", [this]()
{
    UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
    Save->ProtectionScrolls = 7; Save->ResetCubes = 3; Save->RankCubes = 1;
    const FString Payload = /* 기존 직렬화 함수 */;
    UIdleSaveGame* Round = /* 기존 역직렬화 함수 */;
    TestEqual(TEXT("보호서"), Round->ProtectionScrolls, (int64)7);
    TestEqual(TEXT("재설정"), Round->ResetCubes, (int64)3);
    TestEqual(TEXT("등급"), Round->RankCubes, (int64)1);
});
```

- [ ] **Step 2: 실행 → 실패 확인**
- [ ] **Step 3: `CloudSavePayloadMapper.cpp`에 세 자원 필드 매핑 추가** (기존 `RuneEssence` 매핑과 동일 위치/방식). `FItemInstance` 신규 필드는 구조체 직렬화에 자동 포함되는지 확인하고, 수동 매핑이면 잠재/FailStreak/bLocked도 추가.
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(save): 클라우드 페이로드에 장비 심화 자원/필드 매핑"
```

---

## Phase A — 강화 리스크 (서버 → 클라 미러 → 게임플레이 → UI)

### Task A.1: 서버 `enhance.ts` 리스크/천장/보호 공식

**Files:**
- Modify: `server/src/core/formulas/enhance.ts`
- Test: `server/src/core/formulas/enhance.test.ts`

- [ ] **Step 1: 실패 테스트 추가** (`enhance.test.ts` 하단)

```ts
import {
  ENHANCE_SAFE_MAX_LEVEL, ENHANCE_PITY_THRESHOLD, isRiskLevel, resolveEnhanceAttempt,
} from "./enhance.js";

describe("enhance risk model", () => {
  it("안전 구간은 +0~+9, 위험 구간은 +10~+49", () => {
    expect(ENHANCE_SAFE_MAX_LEVEL).toBe(9);
    expect(isRiskLevel(9)).toBe(false);
    expect(isRiskLevel(10)).toBe(true);
    expect(isRiskLevel(49)).toBe(true);
    expect(isRiskLevel(50)).toBe(false); // 최대치는 강화 불가
  });

  it("천장 임계는 12", () => { expect(ENHANCE_PITY_THRESHOLD).toBe(12); });

  it("성공 시 레벨+1, 연속실패 0 리셋", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 12, failStreak: 5, useProtection: false, hasProtection: false, roll: 0 });
    expect(o).toEqual({ success: true, pityForced: false, newLevel: 13, newFailStreak: 0, protectionConsumed: false, downgraded: false });
  });

  it("안전 구간 실패: 단계 유지, 연속실패 누적, 하락 없음", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 5, failStreak: 2, useProtection: false, hasProtection: false, roll: 0.999 });
    expect(o).toMatchObject({ success: false, newLevel: 5, newFailStreak: 3, downgraded: false, protectionConsumed: false });
  });

  it("위험 구간 실패(보호 없음): 단계 -1, 연속실패 0 리셋, 하락 true", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 20, failStreak: 4, useProtection: false, hasProtection: false, roll: 0.999 });
    expect(o).toMatchObject({ success: false, newLevel: 19, newFailStreak: 0, downgraded: true, protectionConsumed: false });
  });

  it("위험 구간 실패(보호 사용): 단계 유지, 연속실패 누적, 보호서 소모", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 20, failStreak: 4, useProtection: true, hasProtection: true, roll: 0.999 });
    expect(o).toMatchObject({ success: false, newLevel: 20, newFailStreak: 5, downgraded: false, protectionConsumed: true });
  });

  it("위험 구간 보호 토글 ON이지만 미보유면 하락", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 20, failStreak: 4, useProtection: true, hasProtection: false, roll: 0.999 });
    expect(o).toMatchObject({ success: false, newLevel: 19, newFailStreak: 0, downgraded: true, protectionConsumed: false });
  });

  it("천장: 보호 누적으로 연속실패 11→12 시 다음 시도 강제 성공", () => {
    const o = resolveEnhanceAttempt({ currentLevel: 20, failStreak: 11, useProtection: true, hasProtection: true, roll: 0.999 });
    expect(o).toMatchObject({ success: true, pityForced: true, newLevel: 21, newFailStreak: 0 });
  });
});
```

- [ ] **Step 2: 실행 → 실패 확인**

Run: `cd server; npm run test -- enhance`
Expected: FAIL ("resolveEnhanceAttempt is not a function" 등)

- [ ] **Step 3: `enhance.ts` 구현** (파일 하단에 추가)

```ts
export const ENHANCE_SAFE_MAX_LEVEL = 9;
export const ENHANCE_PITY_THRESHOLD = 12;

export function isRiskLevel(currentLevel: number): boolean {
  return currentLevel > ENHANCE_SAFE_MAX_LEVEL && currentLevel < MAX_ENHANCE_LEVEL;
}

export interface EnhanceAttemptInput {
  currentLevel: number;
  failStreak: number;
  useProtection: boolean;
  hasProtection: boolean;
  roll: number; // [0,1)
}

export interface EnhanceAttemptOutcome {
  success: boolean;
  pityForced: boolean;
  newLevel: number;
  newFailStreak: number;
  protectionConsumed: boolean;
  downgraded: boolean;
}

export function resolveEnhanceAttempt(input: EnhanceAttemptInput): EnhanceAttemptOutcome {
  const { currentLevel } = input;
  const risk = isRiskLevel(currentLevel);
  const rate = getEnhanceSuccessRate(currentLevel);
  const pityForced = risk && input.failStreak + 1 >= ENHANCE_PITY_THRESHOLD;
  const success = pityForced || rollEnhanceSuccess(rate, () => input.roll);

  if (success) {
    return { success: true, pityForced, newLevel: currentLevel + 1, newFailStreak: 0, protectionConsumed: false, downgraded: false };
  }
  if (!risk) {
    return { success: false, pityForced: false, newLevel: currentLevel, newFailStreak: input.failStreak + 1, protectionConsumed: false, downgraded: false };
  }
  const protect = input.useProtection && input.hasProtection;
  if (protect) {
    return { success: false, pityForced: false, newLevel: currentLevel, newFailStreak: input.failStreak + 1, protectionConsumed: true, downgraded: false };
  }
  return { success: false, pityForced: false, newLevel: Math.max(0, currentLevel - 1), newFailStreak: 0, protectionConsumed: false, downgraded: true };
}
```

- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(server): 강화 리스크/천장/보호 공식(resolveEnhanceAttempt)"
```

### Task A.2: 클라이언트 `FEnhanceFormula` 리스크 미러

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/EnhanceFormula.h/.cpp`
- Test: `client/Source/IdleProject/Tests/EnhanceFormulaTests.cpp`

- [ ] **Step 1: 실패 테스트** — `enhance.test.ts`와 **동일 앵커**로 AutomationSpec 작성(안전/위험 경계, 성공, 안전실패, 위험실패±보호, 천장). 결과 비교용 struct:

```cpp
// EnhanceFormula.h
struct FEnhanceAttemptInput { int32 CurrentLevel; int32 FailStreak; bool bUseProtection; bool bHasProtection; float Roll; };
struct FEnhanceAttemptOutcome { bool bSuccess; bool bPityForced; int32 NewLevel; int32 NewFailStreak; bool bProtectionConsumed; bool bDowngraded; };
```

테스트 예:
```cpp
It("위험 구간 실패(보호 없음)는 -1단계로 하락하고 연속실패를 리셋한다", [this]()
{
    const FEnhanceAttemptOutcome O = FEnhanceFormula::ResolveAttempt({20, 4, false, false, 0.999f});
    TestFalse(TEXT("성공"), O.bSuccess);
    TestEqual(TEXT("레벨"), O.NewLevel, 19);
    TestEqual(TEXT("연속실패"), O.NewFailStreak, 0);
    TestTrue(TEXT("하락"), O.bDowngraded);
});
```

- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: `EnhanceFormula.h/.cpp` 구현** — `static constexpr int32 SafeMaxLevel = 9; static constexpr int32 PityThreshold = 12;`, `static bool IsRiskLevel(int32)`, `static FEnhanceAttemptOutcome ResolveAttempt(const FEnhanceAttemptInput&)`. 로직은 A.1 TS와 1:1(성공률은 기존 `GetEnhanceSuccessRate`, roll은 입력값과 비교).
- [ ] **Step 4: 실행 → 통과** (서버 앵커와 동일 수치 확인)
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): FEnhanceFormula 리스크/천장/보호 미러 + 패리티 테스트"
```

### Task A.3: `InventoryComponent` 강화 결과 적용

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/InventoryComponent.h/.cpp`
- Test: `client/Source/IdleProject/Tests/InventoryTests.cpp`

> 현재 `EnhanceEquippedItem(EItemSlot)`는 성공-only +1만 적용(슬라이스 #33 참조). 결과 구조를 받아 하락/유지/연속실패까지 반영하도록 변이 API를 확장한다.

- [ ] **Step 1: 실패 테스트** — 장착 아이템에 `FEnhanceAttemptOutcome` 적용 시 `EnhanceLevel`/`EnhanceFailStreak`이 결과대로 갱신되는지(성공+1·리셋, 위험실패 -1·리셋, 보호실패 유지·누적).

```cpp
It("위험 구간 보호 실패 적용 시 레벨 유지·연속실패 누적", [this]()
{
    // 장착 아이템 EnhanceLevel=20, FailStreak=4 세팅 후
    Inventory->ApplyEnhanceOutcome(EItemSlot::Weapon, {false,false,20,5,true,false});
    const FItemInstance Eq = /* 장착 조회 */;
    TestEqual(TEXT("레벨"), Eq.EnhanceLevel, 20);
    TestEqual(TEXT("연속실패"), Eq.EnhanceFailStreak, 5);
});
```

- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — `void ApplyEnhanceOutcome(EItemSlot, const FEnhanceAttemptOutcome&)`: 장착 아이템의 `EnhanceLevel=Outcome.NewLevel`, `EnhanceFailStreak=Outcome.NewFailStreak` 설정 + 장착 변경 델리게이트 브로드캐스트. 기존 `EnhanceEquippedItem`은 내부적으로 이 경로를 쓰도록 정리(중복 제거, DRY).
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): InventoryComponent 강화 결과(하락/천장/보호) 적용 API"
```

### Task A.4: `IdleGameInstance::TryEnhanceEquipped` 리스크/보호 처리

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h/.cpp`
- Test: `client/Source/IdleProject/Tests/InventoryTests.cpp` (또는 게임인스턴스 테스트 파일)

> 현재 흐름(#33): 장착 확인 → 비용 계산 → 골드 게이트 → 1회 차감 → 성공률 RNG → 성공 시 +1 → `RecordGearEnhanced()` → `FEnhanceAttemptResult` 반환 + `OnEnhanceResult` 브로드캐스트.

- [ ] **Step 1: 실패 테스트** — 위험 구간에서 보호 토글 ON + 보호서 보유 시: 실패해도 레벨 유지 + 보호서 1 차감 + 결과에 `bProtected=true`. 보호서 미보유 시 토글 ON이어도 하락.

- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현**
  - `TryEnhanceEquipped(EItemSlot Slot, bool bUseProtection)` 시그니처로 확장(기존 호출부는 `bUseProtection=false` 기본).
  - 골드 게이트 후, 장착 아이템의 `EnhanceLevel`/`EnhanceFailStreak` 읽어 `FEnhanceFormula::ResolveAttempt` 호출(결정적 RNG 시드 경로 유지, roll = `Stream.GetFraction()`).
  - `Outcome.bProtectionConsumed`면 `ProtectionScrolls -= 1`(보유>0 선확인 — 미보유면 `useProtection=false`로 호출).
  - `Inventory->ApplyEnhanceOutcome(Slot, Outcome)`.
  - `RecordGearEnhanced()` 유지(시도 기준 퀘스트 진행).
  - 결과 구조 `FEnhanceAttemptResult`에 `bDowngraded`, `bPityForced`, `bProtected`, `NewFailStreak` 필드 추가, `OnEnhanceResult` 브로드캐스트.
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: 자원 보유/차감 getter** — `int64 GetProtectionScrolls() const`, `void AddProtectionScrolls(int64)` 등(기존 `GetGold/AddGold` 패턴). 자원 수급 경로(드롭/상점/던전 보상)는 Task A.6에서 연결.
- [ ] **Step 6: Commit**

```bash
git commit -am "feat(game): TryEnhanceEquipped 리스크/보호/천장 + 보호서 차감"
```

### Task A.5: 강화 HUD 패널 — 구간/보호/천장 표시

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h/.cpp` (`BuildEnhancePanelViewModel`, `TryEnhanceFromHitBox`)
- Modify: `client/Content/Localization/Game/ko/UI.csv`
- Test: `client/Source/IdleProject/Tests/` HUD 뷰모델 테스트

- [ ] **Step 1: 실패 테스트** — `BuildEnhancePanelViewModel`이 구간(안전/위험), 위험 경고 표시, 보호 토글 가용(보유 수), 천장 진행도(X/12), 성공률을 뷰모델에 담는지.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — 뷰모델에 `bRiskZone`, `bDowngradeWarning`, `int64 ProtectionScrolls`, `bProtectionEnabled`, `int32 FailStreak`, `int32 PityThreshold`, `float SuccessRate` 추가. 보호 토글 히트박스 + `TryEnhanceFromHitBox`가 `bUseProtection` 전달. 한글 카피(UI.csv): "안전 구간", "위험: 실패 시 -1단계", "보호서 사용 (보유 {0})", "천장 {0}/{1}".
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(ui): 강화 패널 구간/보호/천장 표시 + 한글 카피"
```

### Task A.6: 보호서 수급 경로 연결

**Files:**
- Modify: 드롭(`DropFormula`/`drop.ts`) 또는 던전/보스 보상, 골드 상점(`shop.ts`/`FShopFormula`) 중 스펙 §4 경로
- Test: 해당 보상/상점 테스트

- [ ] **Step 1: 실패 테스트** — 골드 상점에 보호서 교환 항목 추가 시, 골드 차감 + 보호서 +1(결정적). (드롭 저확률은 별도 테스트.)
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — 골드 상점 교환 항목(보호서) 추가, 가격은 PR #38 `300 * RewardMultiplier` 곡선과 정합되게 1차 시안 설정. 드롭 저확률·던전/보스 보상은 기존 보상 테이블에 보호서 라인 추가(수치는 balance-sim에서 검증).
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(economy): 보호서 수급(상점 교환/드롭/보상) 연결"
```

---

## Phase B — 잠재 레이어 (서버 → 클라 미러 → 적용/PowerScore → 큐브 → UI)

### Task B.1: 서버 `potential.ts` 공식

**Files:**
- Create: `server/src/core/formulas/potential.ts`
- Create: `server/src/core/formulas/potential.test.ts`
- Modify: `server/src/core/formulas/index.ts`

- [ ] **Step 1: 실패 테스트** (`potential.test.ts`)

```ts
import { describe, expect, it } from "vitest";
import {
  POTENTIAL_GRADES, getMaxPotentialGrade, getPotentialLineCount,
  getPotentialRollRange, rollPotentialLines, applyResetCube, applyRankCube,
  RANK_CUBE_UPGRADE_CHANCE,
} from "./potential.js";

describe("potential gating", () => {
  it("아이템 등급별 잠재 등급 상한", () => {
    expect(getMaxPotentialGrade("Common")).toBe("None");
    expect(getMaxPotentialGrade("Rare")).toBe("Epic");
    expect(getMaxPotentialGrade("Epic")).toBe("Unique");
    expect(getMaxPotentialGrade("Unique")).toBe("Legendary");
    expect(getMaxPotentialGrade("Legendary")).toBe("Legendary");
    expect(getMaxPotentialGrade("Mythic")).toBe("Legendary");
  });
  it("잠재 등급별 줄 수", () => {
    expect(getPotentialLineCount("None")).toBe(0);
    expect(getPotentialLineCount("Rare")).toBe(1);
    expect(getPotentialLineCount("Epic")).toBe(2);
    expect(getPotentialLineCount("Unique")).toBe(3);
    expect(getPotentialLineCount("Legendary")).toBe(3);
  });
});

describe("potential rolls (deterministic)", () => {
  const seq = (values: number[]) => { let i = 0; return () => values[i++ % values.length]; };
  it("rollPotentialLines는 등급별 줄 수만큼 중복 없는 옵션을 뽑는다", () => {
    const lines = rollPotentialLines("Epic", seq([0, 0.5, 0, 0.5]));
    expect(lines).toHaveLength(2);
    expect(new Set(lines.map((l) => l.stat)).size).toBe(2);
  });
  it("재설정 큐브는 등급을 유지하고 줄만 다시 굴린다", () => {
    const lines = applyResetCube("Unique", seq([0, 0.5, 0, 0.5, 0, 0.5]));
    expect(lines).toHaveLength(3);
  });
  it("등급 큐브는 상승 롤<확률이면 한 단계 상승(상한 내), 줄 재롤", () => {
    const up = applyRankCube("Epic", "Unique", RANK_CUBE_UPGRADE_CHANCE - 0.001, seq([0, 0.5, 0, 0.5, 0, 0.5]));
    expect(up.grade).toBe("Unique");
    expect(up.lines).toHaveLength(3);
  });
  it("등급 큐브는 상한에서 더 오르지 않는다", () => {
    const up = applyRankCube("Unique", "Unique", 0, seq([0, 0.5, 0, 0.5, 0, 0.5]));
    expect(up.grade).toBe("Unique");
  });
  it("등급 큐브 상승 실패 시 등급 유지·줄 재롤", () => {
    const up = applyRankCube("Epic", "Legendary", RANK_CUBE_UPGRADE_CHANCE + 0.1, seq([0, 0.5, 0, 0.5]));
    expect(up.grade).toBe("Epic");
  });
});
```

- [ ] **Step 2: 실행 → 실패** (`cd server; npm run test -- potential`)
- [ ] **Step 3: `potential.ts` 구현**

```ts
import type { EnhanceItemRarity } from "./enhance.js";

export const POTENTIAL_GRADES = ["None", "Rare", "Epic", "Unique", "Legendary"] as const;
export type PotentialGrade = (typeof POTENTIAL_GRADES)[number];

export const POTENTIAL_STATS = [
  "PhysAtkPct", "MagicAtkPct", "AllStatPct", "CritRate", "CritDmg",
  "HpPct", "DefPct", "GoldGainPct", "DropRatePct",
] as const;
export type PotentialStat = (typeof POTENTIAL_STATS)[number];

export const RANK_CUBE_UPGRADE_CHANCE = 0.08;

export interface PotentialLine { stat: PotentialStat | "None"; value: number; }
export type PotentialRng = () => number;

export function getMaxPotentialGrade(itemRarity: EnhanceItemRarity): PotentialGrade {
  switch (itemRarity) {
    case "Rare": return "Epic";
    case "Epic": return "Unique";
    case "Unique": return "Legendary";
    case "Legendary":
    case "Transcendent":
    case "Mythic": return "Legendary";
    default: return "None"; // Common, None
  }
}

export function getPotentialLineCount(grade: PotentialGrade): number {
  switch (grade) {
    case "Rare": return 1;
    case "Epic": return 2;
    case "Unique":
    case "Legendary": return 3;
    default: return 0;
  }
}

// 등급별 [min,max] 롤 폭 (1차 시안 — balance-sim으로 보정)
const GRADE_SCALE: Record<PotentialGrade, number> = {
  None: 0, Rare: 1, Epic: 1.6, Unique: 2.4, Legendary: 3.4,
};
const STAT_BASE: Record<PotentialStat, { min: number; max: number }> = {
  PhysAtkPct: { min: 0.02, max: 0.05 }, MagicAtkPct: { min: 0.02, max: 0.05 },
  AllStatPct: { min: 0.01, max: 0.03 }, CritRate: { min: 0.01, max: 0.03 },
  CritDmg: { min: 0.02, max: 0.06 }, HpPct: { min: 0.02, max: 0.06 },
  DefPct: { min: 0.02, max: 0.05 }, GoldGainPct: { min: 0.03, max: 0.08 },
  DropRatePct: { min: 0.02, max: 0.05 },
};

export function getPotentialRollRange(stat: PotentialStat, grade: PotentialGrade): { min: number; max: number } {
  const s = GRADE_SCALE[grade]; const b = STAT_BASE[stat];
  return { min: Math.fround(b.min * s), max: Math.fround(b.max * s) };
}

function pickUniqueStats(count: number, rng: PotentialRng): PotentialStat[] {
  const pool = [...POTENTIAL_STATS];
  // Fisher-Yates with injected rng (drop.ts shuffle와 동일 방식)
  for (let i = pool.length - 1; i > 0; i--) {
    const j = Math.floor(rng() * (i + 1));
    [pool[i], pool[j]] = [pool[j], pool[i]];
  }
  return pool.slice(0, count);
}

export function rollPotentialLines(grade: PotentialGrade, rng: PotentialRng): PotentialLine[] {
  const count = getPotentialLineCount(grade);
  if (count <= 0) return [];
  const stats = pickUniqueStats(count, rng);
  return stats.map((stat) => {
    const { min, max } = getPotentialRollRange(stat, grade);
    const value = Math.fround(min + rng() * (max - min));
    return { stat, value };
  });
}

export function applyResetCube(grade: PotentialGrade, rng: PotentialRng): PotentialLine[] {
  return rollPotentialLines(grade, rng);
}

function nextGrade(grade: PotentialGrade): PotentialGrade {
  const i = POTENTIAL_GRADES.indexOf(grade);
  return POTENTIAL_GRADES[Math.min(i + 1, POTENTIAL_GRADES.length - 1)];
}

export function applyRankCube(
  currentGrade: PotentialGrade, itemRarity: EnhanceItemRarity,
  upgradeRoll: number, rng: PotentialRng,
): { grade: PotentialGrade; lines: PotentialLine[] } {
  const cap = getMaxPotentialGrade(itemRarity);
  const capIdx = POTENTIAL_GRADES.indexOf(cap);
  const canUpgrade = POTENTIAL_GRADES.indexOf(currentGrade) < capIdx;
  const grade = canUpgrade && upgradeRoll < RANK_CUBE_UPGRADE_CHANCE ? nextGrade(currentGrade) : currentGrade;
  return { grade, lines: rollPotentialLines(grade, rng) };
}
```

- [ ] **Step 4: `index.ts`에 `export * from "./potential.js";`** 추가, 실행 → 통과
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(server): 잠재 공식(등급 게이팅/줄 롤/큐브) + 테스트"
```

### Task B.2: 클라이언트 `PotentialFormula` 미러

**Files:**
- Create: `client/Source/IdleProject/ItemSystem/PotentialFormula.h/.cpp`
- Test: `client/Source/IdleProject/Tests/PotentialFormulaTests.cpp`

- [ ] **Step 1: 실패 테스트** — B.1과 **동일 앵커**: `GetMaxPotentialGrade`, `GetPotentialLineCount`, `GetPotentialRollRange`(float fround 동일), `RollPotentialLines`(주입 `FRandomStream`로 결정적), `ApplyRankCube` 상승/상한/실패.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — B.1 TS와 1:1. `static EPotentialGrade GetMaxPotentialGrade(EItemRarity)`, `static int32 GetPotentialLineCount(EPotentialGrade)`, `static TArray<FPotentialLine> RollPotentialLines(EPotentialGrade, FRandomStream&)`, `static FApplyRankCubeResult ApplyRankCube(EPotentialGrade Current, EItemRarity, float UpgradeRoll, FRandomStream&)`. 셔플은 `drop.ts`/`DropFormula` 셔플과 동일 방식. fround 매칭 위해 float 산술 사용.
- [ ] **Step 4: 실행 → 통과** (서버 앵커와 동일)
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): PotentialFormula 미러 + 패리티 테스트"
```

### Task B.3: 잠재 → DeriveStats 반영

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/InventoryComponent.cpp` (`ComputeEquipmentBonus`), 서버 `equipment.ts` 미러
- Test: `InventoryTests.cpp`, `server/src/core/formulas/equipment.test.ts`

- [ ] **Step 1: 실패 테스트** — 잠재 줄(% 승수)이 장착 합산 시 파생 스탯에 반영되는지(유니크 trait %승수 경로와 동일). 예: `PhysAtkPct +0.05` 잠재 → 최종 PhysAtk 5% 증가.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — 장착 아이템 잠재 줄을 합산해 `DeriveStats` 직전 % 승수로 적용(유니크 trait의 core-stat 승수 경로 재사용/확장). 클라 `ComputeEquipmentBonus` + 서버 `equipment.ts` 동일.
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): 잠재 줄을 파생 스탯 % 승수로 반영(클라/서버)"
```

### Task B.4: PowerScore에 잠재 가중 합산

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/ItemTypes.h` (`FItemPowerScore::Compute`), `server/src/core/formulas/drop.ts` (`computeItemPowerScore`)
- Test: `client/Source/IdleProject/Tests/InventoryTests.cpp`(ItemPowerScore), `server/src/core/formulas/drop.test.ts`

- [ ] **Step 1: 실패 테스트** — 잠재 줄이 있는 아이템의 PowerScore가 잠재 항만큼 증가하고, **클라/서버 동일 값**. 잠재 없는 레거시 아이템은 기존 값 유지.

```ts
// drop.test.ts
it("잠재 줄은 PowerScore에 가중 합산되고 레거시 아이템은 불변", () => {
  expect(computeItemPowerScore(legacyItem)).toBe(prevAnchor);
  expect(computeItemPowerScore(itemWithPotential)).toBe(prevAnchor + expectedPotentialScore);
});
```

- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — PowerScore에 잠재 항 추가(어픽스 가중 방식과 동일 근사): 예) `+ PotentialPhysAtkPct * 1000 + PotentialCritRate * 1000 + ...` 등 1차 시안 가중치를 **양쪽 동일**하게. `(1 + EnhanceLevel*0.1)` 곱 안에 포함. 가중치 표는 docs에 기록.
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): PowerScore 잠재 가중 합산(클라/서버 동일)"
```

### Task B.5: 잠재 부여(드롭) + 큐브 리롤 게임플레이

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/InventoryComponent.cpp` (잠재 적용 변이), `IdleGameInstance.h/.cpp` (`TryRerollPotential`)
- Test: `InventoryTests.cpp`

- [ ] **Step 1: 실패 테스트** — `TryRerollPotential(EItemSlot, EPotentialCubeType)`: 재설정 큐브 보유>0 시 차감 + 줄 재롤(등급 유지); 등급 큐브 보유>0 시 차감 + `ApplyRankCube` 결과 적용; 보유 0이면 시도 안 함.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현**
  - 드롭 시 잠재 부여: Rare+ 아이템에 초기 잠재 등급/줄 부여(드롭 생성 경로 `DropFormula`/`drop.ts`에 추가 — 초기 등급은 아이템 등급 상한 이하 최소 등급, 1차 시안). 어픽스 롤과 별개 RNG.
  - `EPotentialCubeType { Reset, Rank }`.
  - `TryRerollPotential`: 큐브 보유 선확인 → 차감 → `FRandomStream` 시드 → `applyResetCube`/`applyRankCube` → `Inventory` 잠재 갱신 → 결과 델리게이트(미리보기/적용 UI용).
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(game): 잠재 드롭 부여 + 큐브 리롤(TryRerollPotential)"
```

### Task B.6: 큐브 수급 경로

**Files:**
- Modify: 골드 상점/던전/보스 보상 (스펙 §4)
- Test: 해당 테스트

- [ ] **Step 1: 실패 테스트** — 골드 상점 교환에 재설정/등급 큐브 항목(가격 1차 시안), 골드 차감 + 큐브 +1.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — 재설정 큐브(저가, 드롭+상점), 등급 큐브(고가, 던전/보스+상점). PR #38 곡선 정합.
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(economy): 재설정/등급 큐브 수급 연결"
```

### Task B.7: 잠재 HUD 패널 (미리보기 → 적용/유지)

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h/.cpp`, `client/Content/Localization/Game/ko/UI.csv`
- Test: HUD 뷰모델 테스트

- [ ] **Step 1: 실패 테스트** — `BuildPotentialPanelViewModel`: 현재 잠재 등급/줄, 큐브 보유 수, 보류 중 롤 결과, 적용/유지 버튼 상태.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — 뷰모델 + 히트박스(재설정/등급 큐브 사용 → 보류 롤 생성 → 적용/유지). 등급별 색상, 한글 카피(UI.csv): "잠재 등급", "재설정 큐브 ({0})", "등급 큐브 ({0})", "적용", "유지".
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(ui): 잠재 패널(미리보기/적용/유지) + 한글 카피"
```

### Task B.8: 아이템 잠금(🔒) + 자동장착 존중

**Files:**
- Modify: `client/Source/IdleProject/ItemSystem/InventoryComponent.cpp` (자동장착 비교), `IdleHUD.cpp` (잠금 토글)
- Test: `InventoryTests.cpp`

- [ ] **Step 1: 실패 테스트** — 잠긴(`bLocked=true`) 장착 아이템은 더 높은 PowerScore 드롭이 와도 자동장착이 교체하지 않는다. 잠금 토글 API.
- [ ] **Step 2: 실행 → 실패**
- [ ] **Step 3: 구현** — `void SetItemLocked(EItemSlot/인덱스, bool)`; 자동장착 비교 로직에서 현재 장착이 `bLocked`면 교체 스킵(밸런스 doc §부록 자동장착 흐름 참조).
- [ ] **Step 4: 실행 → 통과**
- [ ] **Step 5: Commit**

```bash
git commit -am "feat(item): 아이템 잠금 + 자동장착 잠금 존중"
```

---

## Phase C — balance-sim & 문서 동기화

### Task C.1: balance-sim 신규 강화 실패 모델 재실행

**Files:**
- Modify: `tools/balance-sim/` (강화 시뮬레이션이 `resolveEnhanceAttempt` import)
- Test: `tools/balance-sim/` 자체 테스트(있으면)

- [ ] **Step 1: 시뮬레이터가 `enhance.ts`의 `resolveEnhanceAttempt`(하락+보호+천장)를 사용하도록 강화 경로 수정.** 보호서 사용 정책(예: 위험 구간 항상 보호 사용 시나리오 / 미사용 시나리오 둘 다) 모델링.
- [ ] **Step 2: 실행** — `cd server; npm run balance:sim`(또는 프로젝트 실제 명령)
- [ ] **Step 3: 결과 기록** — 등급별 +50 도달 기대비용·기대시간(보호 사용/미사용), 첫 환생 5~10h 밴드 유지 확인. 결과를 `05-balance-philosophy.md`에 절로 추가.
- [ ] **Step 4: Commit**

```bash
git commit -am "chore(balance): 강화 리스크 모델 balance-sim 재실행 결과"
```

### Task C.2: 기획 문서 동기화

**Files:**
- Modify: `docs/planning/05-balance-philosophy.md` (강화 리스크/잠재 수치 절, balance-sim 결과)
- Modify: `docs/planning/01-game-design.md` (§4.3 강화 리스크, §4.4 잠재 재감정 갱신)
- Create: `docs/planning/slices/NN-equipment-depth.md`

- [ ] **Step 1: `05-balance-philosophy.md`** — "강화 리스크 모델"(안전/위험/보호/천장 12), "잠재 레이어"(등급 상한·줄 수·옵션 풀·롤 폭·PowerScore 가중치), 큐브/보호서 수급·가격 절 추가.
- [ ] **Step 2: `01-game-design.md`** — §4.3을 구현된 리스크 모델로 갱신(기존 +11~+15 원안 메모는 "구현: 안전 +0~9 / 위험 +10~50, 영구 파괴 없음"으로 정정), §4.4 잠재를 별도 레이어·큐브로 갱신.
- [ ] **Step 3: 슬라이스 문서** — 본 계획 요약 + 7파트 분담(BE/FE/Designer/QA/DevOps) 표.
- [ ] **Step 4: Commit**

```bash
git commit -am "docs: 장비 심화(강화 리스크/잠재) 기획 문서 동기화"
```

### Task C.3: QA 시나리오

**Files:**
- Create: `docs/qa/scenarios/` 신규 시나리오, `docs/qa/regression-checklist.md` 갱신

- [ ] **Step 1: 시나리오 작성(한글)** — (1) 보호서 미사용 위험 강화 → 하락 확인, (2) 보호서 사용 → 유지·보호서 차감, (3) 천장 12회 → 강제 성공, (4) 재설정/등급 큐브 → 줄 재롤·등급 상승, (5) 잠긴 아이템이 자동장착에서 생존, (6) v11 세이브 로드 → v12 마이그레이션·기본값.
- [ ] **Step 2: 회귀 체크리스트에 장비 심화 항목 추가**
- [ ] **Step 3: Commit**

```bash
git commit -am "docs(qa): 장비 심화 시나리오 + 회귀 체크리스트"
```

---

## 자가 검토 결과 (작성자 체크)

**1. 스펙 커버리지**
- 스펙 §2 강화 리스크 → Task A.1~A.6 ✓ (안전/위험, 보호서, 천장, 실효비용=C.1)
- 스펙 §3 잠재 레이어 → Task B.1~B.8 ✓ (게이팅, 줄 수, 옵션 풀, PowerScore, 잠금)
- 스펙 §4 경제 → Task A.6, B.6 ✓
- 스펙 §5 데이터/세이브/패리티 → Task 0.1~0.3, 각 Phase의 클라/서버 미러 ✓
- 스펙 §6 UI → Task A.5, B.7, B.8 ✓
- 스펙 §7 테스트/밸런스/문서 → 각 Task의 TDD + Phase C ✓
- 스펙 §8 미확정 수치 → 1차 시안으로 코드화 + C.1 balance-sim 보정 + docs 기록 ✓

**2. 플레이스홀더 스캔** — 서버 TS는 완전 코드. C++ 작업은 "TS와 1:1 미러 + 동일 앵커"로 명세(시그니처·struct·테스트 앵커 제공). 실제 .cpp 본문은 구현 시 기존 파일 패턴을 따른다(파일/심볼 경로 명시). 마이그레이션 함수명(`MigrateSaveGame`)은 실제 진입점으로 교체 표기.

**3. 타입 일관성** — `FEnhanceAttemptOutcome`/`EnhanceAttemptOutcome`, `resolveEnhanceAttempt`/`ResolveAttempt`, `applyResetCube`/`applyRankCube`/`ApplyRankCube`, `EPotentialCubeType{Reset,Rank}` 명칭 통일 확인. `RANK_CUBE_UPGRADE_CHANCE` 양쪽 동일 상수.

**남은 확정 사항(구현 중 PM 확정):** 잠재 롤 폭·PowerScore 가중치·큐브/보호서 가격·등급 큐브 상승 확률(8% 시안)·큐브 1종 vs 2종·미리보기 vs 즉시교체 — 모두 1차 시안으로 코드화했고 balance-sim/체감으로 보정.
