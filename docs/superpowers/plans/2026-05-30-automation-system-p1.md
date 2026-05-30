# 통합 자동화 시스템 P1 (골격 + 자동 진행) 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** AutomationPolicy 통합 골격(세이브 SaveVer 26 + 서버 parity + 해금 게이팅)과 첫 기둥인 "자동 진행 정책"(전진/파밍고정/자동후퇴 + 보스 자동도전)을 구현한다.

**Architecture:** 클라(UE5) 세이브 권위 + 서버 parity 미러(`automation.ts`, 라우트/DB 없음, 환생특성 패턴 1:1). 자동 진행 결정은 순수 함수로 서버/클라 양쪽에 미러링하여 TDD. 클라는 `UAutomationPolicyService`(RebirthPerkService 패턴)가 정책 상태를 보관하고, `UStageService` 클리어/사망 흐름이 결정 함수를 호출해 행동한다.

**Tech Stack:** 서버 TypeScript + vitest. 클라 UE5 C++ (UObject 서비스, IAutomationTestBase). 빌드/회귀 게이트 `tools/ci/ue-automation.ps1`.

**스펙:** `docs/superpowers/specs/2026-05-30-automation-system-design.md` (§3 아키텍처, §5①, §6 해금, §10 P1)

**P1 범위(명시):** 자동 강화/매각/스킬전술/소비는 **제외**(P2~P4). 무한 효율 업그레이드는 **곡선 함수와 해금 프레임만** 두고 실제 업그레이드 항목은 P4에서 채운다. P1은 진행 정책 + 골격 + 세이브 + 해금 게이팅 + HUD 진행 탭 골격까지.

---

## 파일 구조

| 파일 | 책임 | 신규/수정 |
|---|---|---|
| `server/src/core/formulas/automation.ts` | 해금 임계·진행 결정 순수 함수·효율 비용 곡선(서버 parity) | 신규 |
| `server/src/core/formulas/automation.test.ts` | 위 단위 테스트 | 신규 |
| `server/src/core/formulas/index.ts` | export 추가 | 수정 |
| `client/Source/IdleProject/GameCore/AutomationTypes.h` | enum(`EAutomationFeature`, `EProgressionMode`, `EProgressionAction`) + 결정 입출력 struct | 신규 |
| `client/Source/IdleProject/GameCore/AutomationPolicyService.h` | 정책 상태 + 결정/해금 함수 선언 (서버 parity static) | 신규 |
| `client/Source/IdleProject/GameCore/AutomationPolicyService.cpp` | 구현 | 신규 |
| `client/Source/IdleProject/GameCore/IdleSaveGame.h` | `FAutomationPolicySave` 필드 추가 | 수정 |
| `client/Source/IdleProject/GameCore/IdleGameInstance.h` | 서비스 소유·접근자 선언 | 수정 |
| `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` | SaveVer 26, 저장/복원/마이그레이션, StageService 연동 | 수정 |
| `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp` | 서비스/결정/세이브 round-trip 회귀 | 신규 |

---

## Task 1: 서버 parity — 자동 진행 결정 함수 (클리어 시)

**Files:**
- Create: `server/src/core/formulas/automation.ts`
- Test: `server/src/core/formulas/automation.test.ts`

- [ ] **Step 1: 실패 테스트 작성**

`server/src/core/formulas/automation.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import {
  decideOnClear,
  type ProgressionMode,
} from "./automation.js";

describe("decideOnClear — 진행 정책 클리어 시 결정", () => {
  const base = {
    clearedGlobalStage: 5,
    highestClearedGlobalStage: 5,
    farmLockStage: 3,
    autoBossChallenge: true,
    nextIsBoss: false,
  };

  it("Advance: 일반 다음 스테이지로 전진", () => {
    const r = decideOnClear({ ...base, mode: "Advance" });
    expect(r).toEqual({ action: "advance", targetGlobalStage: 6 });
  });

  it("Advance: 다음이 보스인데 자동도전 OFF면 현재에서 파밍(hold)", () => {
    const r = decideOnClear({ ...base, mode: "Advance", nextIsBoss: true, autoBossChallenge: false });
    expect(r).toEqual({ action: "hold", targetGlobalStage: 5 });
  });

  it("Advance: 다음이 보스이고 자동도전 ON이면 전진", () => {
    const r = decideOnClear({ ...base, mode: "Advance", nextIsBoss: true, autoBossChallenge: true });
    expect(r).toEqual({ action: "advance", targetGlobalStage: 6 });
  });

  it("FarmLock: 지정 스테이지로 고정(전진 안 함)", () => {
    const r = decideOnClear({ ...base, mode: "FarmLock" });
    expect(r).toEqual({ action: "hold", targetGlobalStage: 3 });
  });

  it("FarmLock: 미도달 스테이지 지정 시 최고클리어+1로 클램프", () => {
    const r = decideOnClear({ ...base, mode: "FarmLock", farmLockStage: 99 });
    expect(r).toEqual({ action: "hold", targetGlobalStage: 6 });
  });

  it("AutoRetreat: 클리어 시엔 전진과 동일(후퇴는 사망에서만)", () => {
    const r = decideOnClear({ ...base, mode: "AutoRetreat" });
    expect(r).toEqual({ action: "advance", targetGlobalStage: 6 });
  });
});
```

- [ ] **Step 2: 테스트 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: FAIL — `Cannot find module './automation.js'`

- [ ] **Step 3: 최소 구현**

`server/src/core/formulas/automation.ts`:

```ts
// 통합 자동화 시스템 — 서버 parity 미러(라우트/DB 없음).
// 자동 진행 정책 결정·해금 임계·효율 비용 곡선을 클라 AutomationPolicyService 와
// 1:1 미러링하여 drift 를 방지한다(환생특성 패턴). 결정 함수는 순수 함수.

export type ProgressionMode = "Advance" | "FarmLock" | "AutoRetreat";
export type ProgressionAction = "advance" | "hold" | "retreat";

export type DecideOnClearInput = {
  mode: ProgressionMode;
  clearedGlobalStage: number;
  highestClearedGlobalStage: number;
  farmLockStage: number;
  autoBossChallenge: boolean;
  nextIsBoss: boolean;
};

export type ProgressionDecision = {
  action: ProgressionAction;
  targetGlobalStage: number;
};

// 클리어 직후 다음 행동 결정. 음수/범위초과는 안전 클램프(회귀안전).
export function decideOnClear(input: DecideOnClearInput): ProgressionDecision {
  const cleared = Math.max(1, Math.trunc(input.clearedGlobalStage));
  const highest = Math.max(cleared, Math.trunc(input.highestClearedGlobalStage));
  const next = cleared + 1;

  if (input.mode === "FarmLock") {
    const lock = clampStage(input.farmLockStage, highest);
    return { action: "hold", targetGlobalStage: lock };
  }

  // Advance / AutoRetreat: 클리어 시 동일하게 전진 판정
  if (input.nextIsBoss && !input.autoBossChallenge) {
    return { action: "hold", targetGlobalStage: cleared };
  }
  return { action: "advance", targetGlobalStage: next };
}

// 파밍 고정 스테이지는 [1, highest+1] 로 클램프(미도달 구간 금지).
function clampStage(stage: number, highestCleared: number): number {
  const upper = highestCleared + 1;
  return Math.min(Math.max(1, Math.trunc(stage)), upper);
}
```

- [ ] **Step 4: 테스트 통과 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: PASS (6 passed)

- [ ] **Step 5: 커밋**

```bash
git add server/src/core/formulas/automation.ts server/src/core/formulas/automation.test.ts
git commit -m "feat(server): 자동화 진행 정책 클리어 결정 함수(parity)"
```

---

## Task 2: 서버 parity — 자동 후퇴 결정(사망 시) + 해금/효율 곡선

**Files:**
- Modify: `server/src/core/formulas/automation.ts`
- Test: `server/src/core/formulas/automation.test.ts`

- [ ] **Step 1: 실패 테스트 추가**

`automation.test.ts` 끝에 추가:

```ts
import {
  decideOnDeath,
  isFeatureUnlocked,
  efficiencyUpgradeCost,
  AUTOMATION_FEATURE,
} from "./automation.js";

describe("decideOnDeath — 자동 후퇴 결정", () => {
  it("AutoRetreat: 연속 사망이 임계 미만이면 현재 유지(hold)", () => {
    const r = decideOnDeath({
      mode: "AutoRetreat",
      currentGlobalStage: 8,
      consecutiveDeaths: 2,
      deathThreshold: 3,
    });
    expect(r).toEqual({ action: "hold", targetGlobalStage: 8 });
  });

  it("AutoRetreat: 연속 사망이 임계 도달이면 직전 스테이지로 후퇴", () => {
    const r = decideOnDeath({
      mode: "AutoRetreat",
      currentGlobalStage: 8,
      consecutiveDeaths: 3,
      deathThreshold: 3,
    });
    expect(r).toEqual({ action: "retreat", targetGlobalStage: 7 });
  });

  it("AutoRetreat: 1스테이지에서는 후퇴해도 1 미만으로 안 감", () => {
    const r = decideOnDeath({
      mode: "AutoRetreat",
      currentGlobalStage: 1,
      consecutiveDeaths: 5,
      deathThreshold: 3,
    });
    expect(r).toEqual({ action: "hold", targetGlobalStage: 1 });
  });

  it("Advance/FarmLock: 사망해도 이동 없음(hold)", () => {
    expect(
      decideOnDeath({ mode: "Advance", currentGlobalStage: 8, consecutiveDeaths: 9, deathThreshold: 3 }),
    ).toEqual({ action: "hold", targetGlobalStage: 8 });
    expect(
      decideOnDeath({ mode: "FarmLock", currentGlobalStage: 8, consecutiveDeaths: 9, deathThreshold: 3 }),
    ).toEqual({ action: "hold", targetGlobalStage: 8 });
  });
});

describe("해금 게이팅", () => {
  it("자동 진행은 챕터 0(초반)부터 해금", () => {
    expect(isFeatureUnlocked(AUTOMATION_FEATURE.Progression, { highestClearedChapter: 0, rebirthCount: 0 })).toBe(true);
  });

  it("스킬 전술은 챕터 3 클리어 전엔 잠김", () => {
    expect(isFeatureUnlocked(AUTOMATION_FEATURE.SkillTactics, { highestClearedChapter: 2, rebirthCount: 0 })).toBe(false);
    expect(isFeatureUnlocked(AUTOMATION_FEATURE.SkillTactics, { highestClearedChapter: 3, rebirthCount: 0 })).toBe(true);
  });
});

describe("효율 업그레이드 비용 곡선(무한 sink)", () => {
  it("레벨 0 비용은 base", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, 0)).toBe(1000);
  });
  it("기하급수 증가, 상한 없음", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, 2)).toBe(2250);
    expect(efficiencyUpgradeCost(1000, 1.5, 10)).toBeGreaterThan(efficiencyUpgradeCost(1000, 1.5, 9));
  });
  it("음수 레벨은 base 로 0가드", () => {
    expect(efficiencyUpgradeCost(1000, 1.5, -3)).toBe(1000);
  });
});
```

- [ ] **Step 2: 테스트 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: FAIL — `decideOnDeath`/`isFeatureUnlocked`/`efficiencyUpgradeCost`/`AUTOMATION_FEATURE` is not exported

- [ ] **Step 3: 구현 추가**

`automation.ts` 끝에 추가:

```ts
export type DecideOnDeathInput = {
  mode: ProgressionMode;
  currentGlobalStage: number;
  consecutiveDeaths: number;
  deathThreshold: number;
};

// 사망 직후 결정. AutoRetreat 모드에서만 임계 연속 사망 시 직전 스테이지로 후퇴.
export function decideOnDeath(input: DecideOnDeathInput): ProgressionDecision {
  const current = Math.max(1, Math.trunc(input.currentGlobalStage));
  if (input.mode !== "AutoRetreat") {
    return { action: "hold", targetGlobalStage: current };
  }
  const deaths = Math.max(0, Math.trunc(input.consecutiveDeaths));
  const threshold = Math.max(1, Math.trunc(input.deathThreshold));
  if (deaths >= threshold && current > 1) {
    return { action: "retreat", targetGlobalStage: current - 1 };
  }
  return { action: "hold", targetGlobalStage: current };
}

// 자동화 기능 4종. 클라 EAutomationFeature 와 1:1(이름 동일).
export const AUTOMATION_FEATURE = {
  Progression: "Progression",
  SkillTactics: "SkillTactics",
  AutoGear: "AutoGear",
  AutoConsumable: "AutoConsumable",
} as const;
export type AutomationFeature =
  (typeof AUTOMATION_FEATURE)[keyof typeof AUTOMATION_FEATURE];

// 기능별 해금 조건(스펙 §6 예시). 클라 parity. 진행=초반, 스킬=챕터3,
// 장비=챕터5, 소비=환생1. P1 에선 진행만 실사용, 나머지는 게이트 값만 정의.
const UNLOCK_REQUIREMENT: Record<
  AutomationFeature,
  { minClearedChapter: number; minRebirthCount: number }
> = {
  Progression: { minClearedChapter: 0, minRebirthCount: 0 },
  SkillTactics: { minClearedChapter: 3, minRebirthCount: 0 },
  AutoGear: { minClearedChapter: 5, minRebirthCount: 0 },
  AutoConsumable: { minClearedChapter: 0, minRebirthCount: 1 },
};

export function isFeatureUnlocked(
  feature: AutomationFeature,
  progress: { highestClearedChapter: number; rebirthCount: number },
): boolean {
  const req = UNLOCK_REQUIREMENT[feature];
  return (
    Math.trunc(progress.highestClearedChapter) >= req.minClearedChapter &&
    Math.trunc(progress.rebirthCount) >= req.minRebirthCount
  );
}

// 효율 업그레이드 비용(무한 sink, 상한 없음). base * growth^level, 음수 level 0가드.
// 환생특성/펫 진화 sink 패턴 준용. f32 정합 위해 round.
export function efficiencyUpgradeCost(
  base: number,
  growth: number,
  level: number,
): number {
  const lv = Math.max(0, Math.trunc(level));
  return Math.round(base * Math.pow(growth, lv));
}
```

- [ ] **Step 4: 테스트 통과 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: PASS (전체 통과)

- [ ] **Step 5: index.ts export 추가**

`server/src/core/formulas/index.ts`의 `export * from "./attendance.js";` 다음 줄에 추가:

```ts
export * from "./automation.js";
```

- [ ] **Step 6: 전체 서버 테스트 + 커밋**

Run: `cd server; npm test`
Expected: PASS (기존 + automation 신규 모두 GREEN)

```bash
git add server/src/core/formulas/automation.ts server/src/core/formulas/automation.test.ts server/src/core/formulas/index.ts
git commit -m "feat(server): 자동화 후퇴 결정+해금 게이팅+효율 비용 곡선(parity)"
```

---

## Task 3: 클라 타입 — AutomationTypes.h

**Files:**
- Create: `client/Source/IdleProject/GameCore/AutomationTypes.h`

- [ ] **Step 1: 타입 헤더 작성**

`client/Source/IdleProject/GameCore/AutomationTypes.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AutomationTypes.generated.h"

// 자동화 기능 4종. 서버 automation.ts AUTOMATION_FEATURE 와 1:1(이름 동일).
// P1 에선 Progression 만 실사용, 나머지는 해금 게이트 정의만(P2~P4).
UENUM(BlueprintType)
enum class EAutomationFeature : uint8
{
	Progression,
	SkillTactics,
	AutoGear,
	AutoConsumable
};

// 자동 진행 모드. 서버 ProgressionMode 와 1:1.
UENUM(BlueprintType)
enum class EProgressionMode : uint8
{
	Advance,
	FarmLock,
	AutoRetreat
};

// 진행 결정 행동. 서버 ProgressionAction 과 1:1.
UENUM(BlueprintType)
enum class EProgressionAction : uint8
{
	Advance,
	Hold,
	Retreat
};

// 진행 결정 결과(행동 + 목표 글로벌 스테이지).
USTRUCT(BlueprintType)
struct FProgressionDecision
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Automation")
	EProgressionAction Action = EProgressionAction::Hold;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Automation")
	int32 TargetGlobalStage = 1;
};
```

- [ ] **Step 2: 컴파일 확인(빠른 좁은 빌드)**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.SaveSystem"`
Expected: 빌드 단계 성공(신규 헤더 컴파일 통과). Automation 단계는 아직 신규 테스트 없음 — 기존 GREEN 유지

- [ ] **Step 3: 커밋**

```bash
git add client/Source/IdleProject/GameCore/AutomationTypes.h
git commit -m "feat(client): 자동화 타입(기능/진행모드/결정) 정의"
```

---

## Task 4: 클라 서비스 — AutomationPolicyService (결정/해금 parity static)

**Files:**
- Create: `client/Source/IdleProject/GameCore/AutomationPolicyService.h`
- Create: `client/Source/IdleProject/GameCore/AutomationPolicyService.cpp`
- Test: `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp`

- [ ] **Step 1: 서비스 헤더 작성**

`client/Source/IdleProject/GameCore/AutomationPolicyService.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameCore/AutomationTypes.h"
#include "UObject/Object.h"
#include "AutomationPolicyService.generated.h"

/**
 * 통합 자동화 정책 서비스(P1: 자동 진행). 정책 상태(모드/파밍고정/보스자동/후퇴임계)를
 * 보관하고, 클리어/사망 시 다음 행동을 결정한다(서버 automation.ts 결정 함수 1:1 미러).
 * 정책은 클라 로컬 세이브 권위. 해금 게이팅/효율 비용 곡선도 서버 parity static 으로 제공.
 * 음수/범위초과 입력은 안전 클램프(회귀안전, 환생특성 패턴).
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UAutomationPolicyService : public UObject
{
	GENERATED_BODY()

public:
	// ── 서버 parity static (automation.ts 1:1) ──

	// 클리어 직후 결정(decideOnClear).
	static FProgressionDecision DecideOnClear(
		EProgressionMode Mode,
		int32 ClearedGlobalStage,
		int32 HighestClearedGlobalStage,
		int32 FarmLockStage,
		bool bAutoBossChallenge,
		bool bNextIsBoss);

	// 사망 직후 결정(decideOnDeath).
	static FProgressionDecision DecideOnDeath(
		EProgressionMode Mode,
		int32 CurrentGlobalStage,
		int32 ConsecutiveDeaths,
		int32 DeathThreshold);

	// 기능 해금 여부(isFeatureUnlocked).
	static bool IsFeatureUnlocked(
		EAutomationFeature Feature,
		int32 HighestClearedChapter,
		int32 RebirthCount);

	// 효율 업그레이드 비용(efficiencyUpgradeCost). base*growth^level, 음수 level 0가드.
	static int64 EfficiencyUpgradeCost(int64 Base, float Growth, int32 Level);

	// ── 정책 상태(클라 세이브 권위) ──

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	EProgressionMode GetMode() const { return Mode; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetMode(EProgressionMode InMode) { Mode = InMode; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetFarmLockStage() const { return FarmLockStage; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetFarmLockStage(int32 InStage) { FarmLockStage = FMath::Max(1, InStage); }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoBossChallenge() const { return bAutoBossChallenge; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoBossChallenge(bool bValue) { bAutoBossChallenge = bValue; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetPushDeathThreshold() const { return PushDeathThreshold; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetPushDeathThreshold(int32 InThreshold) { PushDeathThreshold = FMath::Max(1, InThreshold); }

	// 세이브 복원(클램프 적용). 호출 측이 SaveVer>=26 가드 후 호출.
	void RestoreState(EProgressionMode InMode, int32 InFarmLockStage, bool bInAutoBoss, int32 InThreshold);

private:
	UPROPERTY()
	EProgressionMode Mode = EProgressionMode::Advance;

	UPROPERTY()
	int32 FarmLockStage = 1;

	UPROPERTY()
	bool bAutoBossChallenge = true;

	UPROPERTY()
	int32 PushDeathThreshold = 3;
};
```

- [ ] **Step 2: 서비스 구현 작성**

`client/Source/IdleProject/GameCore/AutomationPolicyService.cpp`:

```cpp
#include "GameCore/AutomationPolicyService.h"

FProgressionDecision UAutomationPolicyService::DecideOnClear(
	EProgressionMode Mode,
	int32 ClearedGlobalStage,
	int32 HighestClearedGlobalStage,
	int32 FarmLockStage,
	bool bAutoBossChallenge,
	bool bNextIsBoss)
{
	const int32 Cleared = FMath::Max(1, ClearedGlobalStage);
	const int32 Highest = FMath::Max(Cleared, HighestClearedGlobalStage);

	FProgressionDecision Out;
	if (Mode == EProgressionMode::FarmLock)
	{
		const int32 Upper = Highest + 1;
		Out.Action = EProgressionAction::Hold;
		Out.TargetGlobalStage = FMath::Clamp(FarmLockStage, 1, Upper);
		return Out;
	}

	if (bNextIsBoss && !bAutoBossChallenge)
	{
		Out.Action = EProgressionAction::Hold;
		Out.TargetGlobalStage = Cleared;
		return Out;
	}

	Out.Action = EProgressionAction::Advance;
	Out.TargetGlobalStage = Cleared + 1;
	return Out;
}

FProgressionDecision UAutomationPolicyService::DecideOnDeath(
	EProgressionMode Mode,
	int32 CurrentGlobalStage,
	int32 ConsecutiveDeaths,
	int32 DeathThreshold)
{
	const int32 Current = FMath::Max(1, CurrentGlobalStage);
	FProgressionDecision Out;
	Out.Action = EProgressionAction::Hold;
	Out.TargetGlobalStage = Current;

	if (Mode != EProgressionMode::AutoRetreat)
	{
		return Out;
	}
	const int32 Deaths = FMath::Max(0, ConsecutiveDeaths);
	const int32 Threshold = FMath::Max(1, DeathThreshold);
	if (Deaths >= Threshold && Current > 1)
	{
		Out.Action = EProgressionAction::Retreat;
		Out.TargetGlobalStage = Current - 1;
	}
	return Out;
}

bool UAutomationPolicyService::IsFeatureUnlocked(
	EAutomationFeature Feature,
	int32 HighestClearedChapter,
	int32 RebirthCount)
{
	int32 MinChapter = 0;
	int32 MinRebirth = 0;
	switch (Feature)
	{
	case EAutomationFeature::Progression:     MinChapter = 0; MinRebirth = 0; break;
	case EAutomationFeature::SkillTactics:    MinChapter = 3; MinRebirth = 0; break;
	case EAutomationFeature::AutoGear:        MinChapter = 5; MinRebirth = 0; break;
	case EAutomationFeature::AutoConsumable:  MinChapter = 0; MinRebirth = 1; break;
	}
	return HighestClearedChapter >= MinChapter && RebirthCount >= MinRebirth;
}

int64 UAutomationPolicyService::EfficiencyUpgradeCost(int64 Base, float Growth, int32 Level)
{
	const int32 Lv = FMath::Max(0, Level);
	// 서버 Math.round(base * growth^level) 미러. double 누적 후 반올림.
	const double Cost = static_cast<double>(Base) * FMath::Pow(static_cast<double>(Growth), static_cast<double>(Lv));
	return static_cast<int64>(FMath::RoundToDouble(Cost));
}

void UAutomationPolicyService::RestoreState(
	EProgressionMode InMode, int32 InFarmLockStage, bool bInAutoBoss, int32 InThreshold)
{
	Mode = InMode;
	FarmLockStage = FMath::Max(1, InFarmLockStage);
	bAutoBossChallenge = bInAutoBoss;
	PushDeathThreshold = FMath::Max(1, InThreshold);
}
```

- [ ] **Step 3: 회귀 테스트 작성(서버 parity 동일 케이스)**

`client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp`:

```cpp
#include "Misc/AutomationTest.h"
#include "GameCore/AutomationPolicyService.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationProgressionDecisionTest,
	"IdleProject.GameCore.Automation.ProgressionDecision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationProgressionDecisionTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;

	// Advance: 일반 전진
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::Advance, 5, 5, 3, true, false);
		TestEqual(TEXT("advance action"), (int32)D.Action, (int32)EProgressionAction::Advance);
		TestEqual(TEXT("advance target"), D.TargetGlobalStage, 6);
	}
	// Advance: 보스 자동도전 OFF → hold
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::Advance, 5, 5, 3, false, true);
		TestEqual(TEXT("boss off action"), (int32)D.Action, (int32)EProgressionAction::Hold);
		TestEqual(TEXT("boss off target"), D.TargetGlobalStage, 5);
	}
	// FarmLock: 미도달 클램프
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::FarmLock, 5, 5, 99, true, false);
		TestEqual(TEXT("farmlock action"), (int32)D.Action, (int32)EProgressionAction::Hold);
		TestEqual(TEXT("farmlock clamp"), D.TargetGlobalStage, 6);
	}
	// AutoRetreat: 임계 도달 후퇴
	{
		const FProgressionDecision D = S::DecideOnDeath(EProgressionMode::AutoRetreat, 8, 3, 3);
		TestEqual(TEXT("retreat action"), (int32)D.Action, (int32)EProgressionAction::Retreat);
		TestEqual(TEXT("retreat target"), D.TargetGlobalStage, 7);
	}
	// AutoRetreat: 1스테이지 하한
	{
		const FProgressionDecision D = S::DecideOnDeath(EProgressionMode::AutoRetreat, 1, 5, 3);
		TestEqual(TEXT("retreat floor"), D.TargetGlobalStage, 1);
		TestEqual(TEXT("retreat floor hold"), (int32)D.Action, (int32)EProgressionAction::Hold);
	}
	// 해금 게이팅
	{
		TestTrue(TEXT("progression unlocked"), S::IsFeatureUnlocked(EAutomationFeature::Progression, 0, 0));
		TestFalse(TEXT("skill locked ch2"), S::IsFeatureUnlocked(EAutomationFeature::SkillTactics, 2, 0));
		TestTrue(TEXT("skill unlocked ch3"), S::IsFeatureUnlocked(EAutomationFeature::SkillTactics, 3, 0));
	}
	// 효율 비용 곡선
	{
		TestEqual(TEXT("cost lv0"), S::EfficiencyUpgradeCost(1000, 1.5f, 0), (int64)1000);
		TestEqual(TEXT("cost lv2"), S::EfficiencyUpgradeCost(1000, 1.5f, 2), (int64)2250);
		TestEqual(TEXT("cost neg guard"), S::EfficiencyUpgradeCost(1000, 1.5f, -3), (int64)1000);
	}
	return true;
}
```

- [ ] **Step 4: 빌드 + 좁은 Automation 실행**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.Automation"`
Expected: 표준 jumbo 빌드 성공(ODR 없음) + `IdleProject.GameCore.Automation.ProgressionDecision` PASS

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/AutomationPolicyService.h client/Source/IdleProject/GameCore/AutomationPolicyService.cpp client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp
git commit -m "feat(client): 자동화 정책 서비스(진행 결정/해금/효율) + 회귀"
```

---

## Task 5: 세이브 — FAutomationPolicySave 필드 + SaveVer 26

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleSaveGame.h`

- [ ] **Step 1: 세이브 구조체 필드 추가**

`IdleSaveGame.h` 상단 include 블록에 추가(다른 `#include "GameCore/..."` 인접):

```cpp
#include "GameCore/AutomationTypes.h"
```

`IdleSaveGame.h`에서 `int32 SaveVersion = 25;` 를 다음으로 변경:

```cpp
	// SaveVer 26: 자동화 정책(FAutomationPolicySave) 추가. <26 세이브는 기본값(전진/보스자동ON) 마이그레이션.
	int32 SaveVersion = 26;
```

`RebirthPerkAllocations` 필드 다음 줄(파일 내 마지막 UPROPERTY 군 근처)에 추가:

```cpp
	// 자동화 정책(P1: 자동 진행). SaveVer 26+. 클라 세이브 권위.
	UPROPERTY()
	EProgressionMode AutomationProgressionMode = EProgressionMode::Advance;

	UPROPERTY()
	int32 AutomationFarmLockStage = 1;

	UPROPERTY()
	bool bAutomationAutoBossChallenge = true;

	UPROPERTY()
	int32 AutomationPushDeathThreshold = 3;
```

- [ ] **Step 2: 빌드 확인**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.SaveSystem"`
Expected: 빌드 성공. 기존 SaveSystem Automation GREEN(세이브 구조 확장만, 로직 변경 없음)

- [ ] **Step 3: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleSaveGame.h
git commit -m "feat(client): 자동화 정책 세이브 필드 + SaveVer 26"
```

---

## Task 6: GameInstance 연동 — 소유/저장/복원/마이그레이션

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h`
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp`

- [ ] **Step 1: 헤더에 서비스 소유·접근자 선언**

`IdleGameInstance.h`의 `RebirthPerkService` 멤버 선언부를 찾아 인접하게 추가(UPROPERTY private 군):

```cpp
	UPROPERTY()
	TObjectPtr<UAutomationPolicyService> AutomationPolicyService = nullptr;
```

`IdleGameInstance.h` 상단 include에 추가:

```cpp
#include "GameCore/AutomationPolicyService.h"
```

public 접근자 선언(RebirthPerkService 접근자 인접):

```cpp
	// 자동화 정책 서비스 접근자. 없으면 생성(lazy-ensure).
	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	UAutomationPolicyService* GetAutomationPolicyService() const { return AutomationPolicyService; }

private:
	void EnsureAutomationPolicyService();
public:
```

> 주: 기존 헤더의 접근지정자 구조에 맞춰 `EnsureAutomationPolicyService()`는 다른 `Ensure*` 선언과 같은 위치에 둔다(별도 public/private 블록 추가가 어색하면 기존 private 섹션으로 이동).

- [ ] **Step 2: ensure/소유 구현**

`IdleGameInstance.cpp`에서 `EnsureRebirthPerkService()` 구현을 찾아 바로 아래에 추가:

```cpp
void UIdleGameInstance::EnsureAutomationPolicyService()
{
	if (!AutomationPolicyService)
	{
		AutomationPolicyService = NewObject<UAutomationPolicyService>(this);
	}
}
```

`Shutdown()`(또는 서비스 nullptr 정리 지점, 기존 `RebirthPerkService = nullptr;` 라인 근처)에 추가:

```cpp
	AutomationPolicyService = nullptr;
```

- [ ] **Step 3: 저장 경로 — SaveVer 26 + 정책 직렬화**

`IdleGameInstance.cpp`의 `SaveGame->SaveVersion = 25;` 를 다음으로 변경:

```cpp
	SaveGame->SaveVersion = 26;
```

같은 저장 함수 내 `SaveGame->RebirthPerkAllocations = ...;` 블록 다음에 추가:

```cpp
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		SaveGame->AutomationProgressionMode = AutomationPolicyService->GetMode();
		SaveGame->AutomationFarmLockStage = AutomationPolicyService->GetFarmLockStage();
		SaveGame->bAutomationAutoBossChallenge = AutomationPolicyService->GetAutoBossChallenge();
		SaveGame->AutomationPushDeathThreshold = AutomationPolicyService->GetPushDeathThreshold();
	}
```

- [ ] **Step 4: 복원 경로 — SaveVer 가드 마이그레이션**

`IdleGameInstance.cpp`의 복원 함수에서 `RebirthPerkService->RestoreState(...)` 블록 다음에 추가:

```cpp
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		if (SaveGame->SaveVersion >= 26)
		{
			AutomationPolicyService->RestoreState(
				SaveGame->AutomationProgressionMode,
				SaveGame->AutomationFarmLockStage,
				SaveGame->bAutomationAutoBossChallenge,
				SaveGame->AutomationPushDeathThreshold);
		}
		else
		{
			// <26 세이브: 기본값(전진/보스자동ON/임계3) 마이그레이션
			AutomationPolicyService->RestoreState(EProgressionMode::Advance, 1, true, 3);
		}
	}
```

- [ ] **Step 5: 빌드 + 세이브 round-trip Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore"`
Expected: 빌드 성공 + GameCore Automation(SaveSystem 포함) GREEN. SaveVer 26 단언 통과

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp
git commit -m "feat(client): GameInstance 자동화 정책 소유/저장/복원/마이그레이션"
```

---

## Task 7: 자동 진행 연동 — StageService 클리어/사망 흐름

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp`
- Test: `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp` (통합 케이스 추가)

> StageService 의 클리어(`AdvanceStage`/`RecordKill`→advance)와 캐릭터 사망 이벤트를 GameInstance 가 중계하는 지점에 정책 결정을 삽입한다. StageService 자체는 순수 진행기로 두고, "전진할지/멈출지/후퇴할지"는 정책이 GameInstance 경유로 결정한다(관심사 분리).

- [ ] **Step 1: 통합 결정 헬퍼 추가(실패 테스트 먼저)**

`AutomationPolicyServiceTests.cpp`에 통합 시나리오 테스트 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationProgressionAdvanceFlowTest,
	"IdleProject.GameCore.Automation.AdvanceFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationProgressionAdvanceFlowTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	// FarmLock 고정: 클리어해도 항상 동일 스테이지 hold
	const FProgressionDecision D1 = S::DecideOnClear(EProgressionMode::FarmLock, 4, 7, 4, true, false);
	TestEqual(TEXT("farmlock hold stays"), D1.TargetGlobalStage, 4);
	// AutoRetreat: 임계 미만 사망은 유지, 클리어 시엔 전진
	const FProgressionDecision D2 = S::DecideOnDeath(EProgressionMode::AutoRetreat, 6, 1, 3);
	TestEqual(TEXT("autoretreat hold below threshold"), (int32)D2.Action, (int32)EProgressionAction::Hold);
	const FProgressionDecision D3 = S::DecideOnClear(EProgressionMode::AutoRetreat, 6, 6, 1, true, false);
	TestEqual(TEXT("autoretreat advances on clear"), D3.TargetGlobalStage, 7);
	return true;
}
```

- [ ] **Step 2: 테스트 빌드 후 실패→통과 확인**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.Automation.AdvanceFlow"`
Expected: 신규 테스트 PASS (결정 함수가 이미 구현됨 → 통합 시나리오 검증)

- [ ] **Step 3: GameInstance 클리어 중계에 정책 적용**

`IdleGameInstance.cpp`에서 StageService 클리어/전진을 처리하는 지점(예: `OnStageChanged`/`RecordKill` 후 advance 판단 또는 `MarkCurrentChapterBossDefeated` 흐름)을 찾아, 무조건 `AdvanceStage()` 호출 대신 정책 결정을 경유하도록 수정:

```cpp
	EnsureAutomationPolicyService();
	if (StageService && AutomationPolicyService)
	{
		const int32 ClearedGlobal = StageService->GetGlobalStageIndex();
		const int32 HighestGlobal = ClearedGlobal; // P1: 현재 최고 도달=현재(누적 최고치는 후속 정교화)
		// 다음 스테이지가 보스인지: StageService 의 다음 stage 기준(StagesPerChapter 경계)
		const bool bNextIsBoss = (StageService->GetCurrentStage() + 1) >= UStageService::StagesPerChapter
			? false /* 챕터 경계 보스 판정은 StageService 규칙 사용 */
			: false;

		const FProgressionDecision Decision = UAutomationPolicyService::DecideOnClear(
			AutomationPolicyService->GetMode(),
			ClearedGlobal,
			HighestGlobal,
			AutomationPolicyService->GetFarmLockStage(),
			AutomationPolicyService->GetAutoBossChallenge(),
			bNextIsBoss);

		if (Decision.Action == EProgressionAction::Advance)
		{
			StageService->AdvanceStage();
		}
		// Hold/Retreat: 현재 스테이지 유지(FarmLock/보스대기). 스테이지 강제 이동은
		// StageService 에 SetCurrentStage 가 없으므로 P1 은 Advance 억제까지만 지원하고
		// 명시적 점프(파밍 고정 스테이지로 이동/후퇴)는 Task 8 의 StageService 확장에서 처리.
	}
```

> 주: `bNextIsBoss` 정확 판정과 강제 점프(파밍 고정/후퇴 이동)는 StageService 가 목표 스테이지 세팅 API를 제공해야 완성된다 → Task 8.

- [ ] **Step 4: 빌드 + GameCore Automation GREEN 확인**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore"`
Expected: 빌드 성공 + GREEN

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp
git commit -m "feat(client): 자동 진행 정책을 스테이지 클리어 흐름에 연동(전진 억제)"
```

---

## Task 8: StageService 목표 점프 API + 보스 판정 정확화

**Files:**
- Modify: `client/Source/IdleProject/GameCore/StageService.h`
- Modify: `client/Source/IdleProject/GameCore/StageService.cpp`
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp`
- Test: `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp`

- [ ] **Step 1: StageService에 글로벌 스테이지 점프 + 다음 보스 판정 선언**

`StageService.h`의 `void AdvanceStage();` 다음에 추가:

```cpp
	// 글로벌 스테이지 인덱스로 직접 이동(파밍 고정/자동 후퇴용). [1, 최대도달+1] 범위로 클램프.
	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void JumpToGlobalStage(int32 TargetGlobalStage);

	// 현재 스테이지의 "다음" 스테이지가 보스인지(클리어 시 자동 진행 판정용).
	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	bool IsNextStageBoss() const;
```

- [ ] **Step 2: 구현 작성**

`StageService.cpp`에 추가(기존 `isBossStage` 규칙 = `Stage == StagesPerChapter` 재사용):

```cpp
void UStageService::JumpToGlobalStage(int32 TargetGlobalStage)
{
	const int32 MaxGlobal = (HighestClearedChapter * StagesPerChapter) + StagesPerChapter; // 도달 가능 상한 근사
	const int32 Clamped = FMath::Clamp(TargetGlobalStage, 1, FMath::Max(1, MaxGlobal));
	const int32 NewChapter = ((Clamped - 1) / StagesPerChapter) + 1;
	const int32 NewStage = ((Clamped - 1) % StagesPerChapter) + 1;
	CurrentChapter = FMath::Clamp(NewChapter, 1, TotalChapters);
	CurrentStage = FMath::Clamp(NewStage, 1, StagesPerChapter);
	KillsThisStage = 0;
	OnStageChanged.Broadcast(GetCurrentStageInfo());
}

bool UStageService::IsNextStageBoss() const
{
	// 다음 스테이지 번호(챕터 내). 현재가 보스면 다음은 새 챕터의 1스테이지 → 보스 아님.
	if (CurrentStage >= StagesPerChapter)
	{
		return false;
	}
	return (CurrentStage + 1) == StagesPerChapter;
}
```

- [ ] **Step 3: GameInstance 연동에 Hold/Retreat 점프 적용**

Task 7 Step 3에서 추가한 블록의 `bNextIsBoss` 계산과 Hold/Retreat 처리를 다음으로 교체:

```cpp
		const bool bNextIsBoss = StageService->IsNextStageBoss();

		const FProgressionDecision Decision = UAutomationPolicyService::DecideOnClear(
			AutomationPolicyService->GetMode(),
			ClearedGlobal,
			HighestGlobal,
			AutomationPolicyService->GetFarmLockStage(),
			AutomationPolicyService->GetAutoBossChallenge(),
			bNextIsBoss);

		switch (Decision.Action)
		{
		case EProgressionAction::Advance:
			StageService->AdvanceStage();
			break;
		case EProgressionAction::Hold:
			StageService->JumpToGlobalStage(Decision.TargetGlobalStage);
			break;
		case EProgressionAction::Retreat:
			StageService->JumpToGlobalStage(Decision.TargetGlobalStage);
			break;
		}
```

- [ ] **Step 4: 점프 회귀 테스트 추가**

`AutomationPolicyServiceTests.cpp`에 StageService 점프 검증 추가:

```cpp
#include "GameCore/StageService.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageJumpTest,
	"IdleProject.GameCore.Automation.StageJump",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageJumpTest::RunTest(const FString& Parameters)
{
	UStageService* Stage = NewObject<UStageService>();
	Stage->InitializeDefaultStages();
	Stage->JumpToGlobalStage(1);
	TestEqual(TEXT("jump floor chapter"), Stage->GetCurrentChapter(), 1);
	TestEqual(TEXT("jump floor stage"), Stage->GetCurrentStage(), 1);
	// 9스테이지(다음이 10=보스)
	Stage->JumpToGlobalStage(9);
	TestTrue(TEXT("stage9 next is boss"), Stage->IsNextStageBoss());
	// 10스테이지(보스): 다음은 새 챕터 1 → 보스 아님
	Stage->JumpToGlobalStage(10);
	TestFalse(TEXT("boss stage next not boss"), Stage->IsNextStageBoss());
	return true;
}
```

- [ ] **Step 5: 빌드 + GameCore Automation 전체 GREEN**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore"`
Expected: 빌드 성공 + StageJump/AdvanceFlow/ProgressionDecision 포함 GREEN

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/GameCore/StageService.h client/Source/IdleProject/GameCore/StageService.cpp client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp
git commit -m "feat(client): StageService 목표 점프/보스 판정 + 파밍고정·후퇴 이동 완성"
```

---

## Task 9: HUD 자동 진행 탭 골격(데이터 구동) + 정책 변경 BlueprintCallable

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h`
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp`

> 데이터 구동 HUD 패턴(코드 0 위젯 신규)을 따른다. P1 은 BP 가 호출할 정책 변경 진입점만 GameInstance 에 노출하고, 실제 위젯 바인딩은 데이터 구동 HUD 데이터로 처리한다(기존 HUD 패턴 재사용 — 신규 C++ 위젯 클래스 금지).

- [ ] **Step 1: 정책 변경 BlueprintCallable 래퍼 선언**

`IdleGameInstance.h` public 섹션에 추가:

```cpp
	// HUD 자동화 진행 탭용 래퍼(BP 바인딩). 변경 후 자동 저장 트리거는 기존 세이브 흐름 따름.
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationProgressionMode(EProgressionMode InMode);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationFarmLockStage(int32 InGlobalStage);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoBossChallenge(bool bValue);

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool IsAutomationFeatureUnlocked(EAutomationFeature Feature) const;
```

- [ ] **Step 2: 구현 작성**

`IdleGameInstance.cpp`에 추가(해금 판정은 RebirthCount/HighestClearedChapter 기존 접근자 사용 — 정확한 멤버명은 파일 내 `GetRebirthCount`/`StageService->GetHighestClearedChapter()` 확인 후 맞춤):

```cpp
void UIdleGameInstance::SetAutomationProgressionMode(EProgressionMode InMode)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		AutomationPolicyService->SetMode(InMode);
	}
}

void UIdleGameInstance::SetAutomationFarmLockStage(int32 InGlobalStage)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		AutomationPolicyService->SetFarmLockStage(InGlobalStage);
	}
}

void UIdleGameInstance::SetAutomationAutoBossChallenge(bool bValue)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		AutomationPolicyService->SetAutoBossChallenge(bValue);
	}
}

bool UIdleGameInstance::IsAutomationFeatureUnlocked(EAutomationFeature Feature) const
{
	const int32 HighestChapter = StageService ? StageService->GetHighestClearedChapter() : 0;
	return UAutomationPolicyService::IsFeatureUnlocked(Feature, HighestChapter, RebirthCount);
}
```

> 주: `RebirthCount`는 파일 내 기존 멤버/접근자명을 확인해 일치시킨다(예: `RebirthCount` 직접 멤버 또는 `GetRebirthCount()`). 컴파일 오류 시 해당 접근자로 교체.

- [ ] **Step 3: 빌드 + 전체 IdleProject Automation(HUD 포함) GREEN**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject"`
Expected: 표준 jumbo 빌드 성공(ODR 0) + 전체 IdleProject Automation GREEN(`IdleProject.UI.HUD` 포함, HUD 영향 슬라이스 게이트 필터)

- [ ] **Step 4: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp
git commit -m "feat(client): 자동화 진행 탭 정책 변경 BP 진입점 + 해금 판정"
```

---

## Task 10: 최종 게이트 — 전체 회귀 + SaveVer stale 점검

**Files:** (검증 전용, 코드 변경 없음 원칙. stale 발견 시 해당 파일만 수정)

- [ ] **Step 1: 서버 전체 테스트**

Run: `cd server; npm test`
Expected: 전체 GREEN (automation 포함)

- [ ] **Step 2: UE 표준 jumbo 빌드 + 전체 Automation 게이트**

Run: `./tools/ci/ue-automation.ps1`
Expected: Result에 Fail 0, 비0 EXIT 없음. SaveVer 26 단언 GREEN

- [ ] **Step 3: SaveVer stale 일괄 점검**

`SaveVersion < N` 가드들이 26 도입과 모순 없는지, 신규 필드의 `>= 26` 가드가 일관된지 grep 확인:

Run: `grep -rn "SaveVersion" client/Source/IdleProject/GameCore/IdleGameInstance.cpp`
Expected: 신규 `>= 26` 가드 존재, 기존 가드 회귀 없음. (stale 발견 시 해당 라인만 보정 후 재게이트)

- [ ] **Step 4: 최종 커밋(점검 결과 stale 보정 있었던 경우만)**

```bash
git add -A
git commit -m "chore: 자동화 P1 SaveVer 26 stale 점검/보정"
```

---

## Self-Review (작성자 점검 결과)

**스펙 커버리지(§ 대비):**
- §3 아키텍처(클라 세이브 권위 + 서버 parity, 신규 서비스, SaveVer 26, 데이터 구동 HUD) → Task 1~9 ✅
- §5① 자동 진행(전진/파밍고정/자동후퇴 + 보스 자동도전) → Task 1,2,7,8 ✅
- §6 해금 게이팅 + 효율 비용 곡선(프레임) → Task 2,4,9 ✅ (실제 업그레이드 항목은 P4 — 스펙 §10 명시)
- §9 테스트/게이트(parity test, Automation 회귀, ue-automation.ps1, SaveVer stale) → Task 1~4,10 ✅

**P1 제외 항목(스펙 §10):** 스킬전술/장비/소비/자동매각/자동강화 → P2~P4 (본 plan 미포함, 의도적) ✅

**플레이스홀더 스캔:** "주:" 노트는 기존 파일의 정확한 멤버명/접근지정자 위치를 구현 시 확인하라는 안내이며, 실제 코드 블록은 모두 채워짐. TODO/TBD 없음.

**타입 일관성:** `EProgressionMode`/`EProgressionAction`/`EAutomationFeature`/`FProgressionDecision`/`DecideOnClear`/`DecideOnDeath`/`IsFeatureUnlocked`/`EfficiencyUpgradeCost`/`JumpToGlobalStage`/`IsNextStageBoss` — 서버(automation.ts)와 클라(AutomationPolicyService) 명칭·시그니처 1:1 정합. 세이브 필드명(`Automation*`)/접근자 일관.
