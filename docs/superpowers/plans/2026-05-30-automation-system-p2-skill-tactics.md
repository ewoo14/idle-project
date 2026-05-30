# 통합 자동화 시스템 P2 (스킬 자동 전술) 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 스킬별 자동 발동 조건(Always/BossEliteOnly/HpBelow/MaintainBuff)+우선순위를 `AutomationPolicy`에 저장하고 `USkillComponent::TickSkills` 게이트로 통제한다.

**Architecture:** P1 골격(AutomationPolicy 세이브 + 서버 parity + 해금) 재사용. 결정은 순수 함수 `evaluateSkillRule`로 서버/클라 미러(TDD). `TickSkills`는 시그니처 churn을 피해 `bIsBossElite` 기본 인자만 추가하고 HP%/buffActive는 내부 계산. 규칙은 `SetAutoRules`로 주입(전투→GameInstance 의존성 역전).

**Tech Stack:** 서버 TypeScript+vitest. 클라 UE5 C++. 게이트 `tools/ci/ue-automation.ps1` + 서버 `npm test`/biome.

**스펙:** `docs/superpowers/specs/2026-05-30-automation-system-p2-skill-tactics-design.md`
**선행:** P1(PR #99 머지 `5f1fa38`). 브랜치 `feat/automation-system-p2`.

**P2 제외:** 규칙 슬롯 제한/효율 sink(P4), 자동 장비(P3), 자동 소비(P4).

---

## 파일 구조

| 파일 | 책임 | 신규/수정 |
|---|---|---|
| `server/src/core/formulas/automation.ts` | `evaluateSkillRule` 추가(parity) | 수정 |
| `server/src/core/formulas/automation.test.ts` | 위 테스트 | 수정 |
| `client/Source/IdleProject/GameCore/AutomationTypes.h` | `ESkillAutoCondition`+`FSkillAutoRule` | 수정 |
| `client/Source/IdleProject/GameCore/AutomationPolicyService.h/.cpp` | `EvaluateSkillRule` static + `SkillRules` 상태/접근자 | 수정 |
| `client/Source/IdleProject/GameCore/IdleSaveGame.h` | `SkillRules` 필드 + SaveVer 27 | 수정 |
| `client/Source/IdleProject/GameCore/IdleGameInstance.h/.cpp` | 저장/복원/마이그 + BP 규칙 접근자 + 플레이어 동기 | 수정 |
| `client/Source/IdleProject/CombatSystem/SkillComponent.h/.cpp` | `SetAutoRules`/`GetRuleFor`/TickSkills 게이트 | 수정 |
| `client/Source/IdleProject/CombatSystem/BattleAIComponent.cpp` | `bIsBossElite` 전달 | 수정 |
| `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp` | `EvaluateSkillRule` parity 회귀 | 수정 |
| `client/Source/IdleProject/Tests/CombatTests.cpp` | TickSkills 규칙 게이트 회귀 | 수정 |
| 다수 `Tests/*ServiceTests.cpp` 등 | SaveVer 26→27 stale 일괄 | 수정 |

---

## Task 1: 서버 parity — evaluateSkillRule

**Files:**
- Modify: `server/src/core/formulas/automation.ts`
- Test: `server/src/core/formulas/automation.test.ts`

- [ ] **Step 1: 실패 테스트 추가**

`automation.test.ts` 끝에 추가:

```ts
import { evaluateSkillRule, type SkillAutoCondition } from "./automation.js";

describe("evaluateSkillRule — 스킬 자동 전술 조건", () => {
  const ctx = (over: Partial<{ selfHpPct: number; isBossElite: boolean; buffActive: boolean }> = {}) => ({
    selfHpPct: 1, isBossElite: false, buffActive: false, ...over,
  });

  it("Always: 항상 발동", () => {
    expect(evaluateSkillRule("Always", 0.3, ctx())).toBe(true);
  });

  it("BossEliteOnly: 보스/엘리트일 때만", () => {
    expect(evaluateSkillRule("BossEliteOnly", 0.3, ctx({ isBossElite: true }))).toBe(true);
    expect(evaluateSkillRule("BossEliteOnly", 0.3, ctx({ isBossElite: false }))).toBe(false);
  });

  it("HpBelow: HP%가 임계 이하일 때만", () => {
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.3 }))).toBe(true);
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.29 }))).toBe(true);
    expect(evaluateSkillRule("HpBelow", 0.3, ctx({ selfHpPct: 0.31 }))).toBe(false);
  });

  it("HpBelow: 임계 [0,1] 클램프", () => {
    expect(evaluateSkillRule("HpBelow", 5, ctx({ selfHpPct: 1 }))).toBe(true);   // 클램프 1
    expect(evaluateSkillRule("HpBelow", -1, ctx({ selfHpPct: 0 }))).toBe(true);  // 클램프 0
  });

  it("MaintainBuff: 버프 비활성일 때만 발동", () => {
    expect(evaluateSkillRule("MaintainBuff", 0.3, ctx({ buffActive: false }))).toBe(true);
    expect(evaluateSkillRule("MaintainBuff", 0.3, ctx({ buffActive: true }))).toBe(false);
  });
});
```

- [ ] **Step 2: 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: FAIL — `evaluateSkillRule` is not exported

- [ ] **Step 3: 구현 추가**

`automation.ts` 끝에 추가:

```ts
// 스킬 자동 전술 조건. 클라 ESkillAutoCondition 와 1:1(이름 동일).
export type SkillAutoCondition =
  | "Always"
  | "BossEliteOnly"
  | "HpBelow"
  | "MaintainBuff";

export type SkillRuleContext = {
  selfHpPct: number;
  isBossElite: boolean;
  buffActive: boolean;
};

// 스킬 발동 규칙 평가(순수). 클라 EvaluateSkillRule 1:1 미러.
// hpThresholdPct/selfHpPct 는 [0,1] 클램프(회귀안전).
export function evaluateSkillRule(
  condition: SkillAutoCondition,
  hpThresholdPct: number,
  ctx: SkillRuleContext,
): boolean {
  switch (condition) {
    case "BossEliteOnly":
      return ctx.isBossElite;
    case "HpBelow":
      return clamp01(ctx.selfHpPct) <= clamp01(hpThresholdPct);
    case "MaintainBuff":
      return !ctx.buffActive;
    default:
      return true; // Always (및 미지정)
  }
}

function clamp01(v: number): number {
  if (!Number.isFinite(v)) return 0;
  return Math.min(1, Math.max(0, v));
}
```

- [ ] **Step 4: 통과 확인 + 전체 + lint**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: PASS

Run: `cd server; npm test`
Expected: 전체 GREEN

Run: `cd server; npx biome check . ../tools/balance-sim`
Expected: `No fixes applied` (clean). 위반 시 `npx biome check --write src/core/formulas/automation.ts` 후 재확인.

- [ ] **Step 5: 커밋**

```bash
git add server/src/core/formulas/automation.ts server/src/core/formulas/automation.test.ts
git commit -m "feat(server): 스킬 자동 전술 evaluateSkillRule parity"
```

---

## Task 2: 클라 타입 + EvaluateSkillRule static

**Files:**
- Modify: `client/Source/IdleProject/GameCore/AutomationTypes.h`
- Modify: `client/Source/IdleProject/GameCore/AutomationPolicyService.h` / `.cpp`
- Test: `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp`

- [ ] **Step 1: 타입 추가 (AutomationTypes.h)**

`AutomationTypes.h`의 `FProgressionDecision` 정의 다음에 추가:

```cpp
// 스킬 자동 전술 조건. 서버 automation.ts SkillAutoCondition 와 1:1.
UENUM(BlueprintType)
enum class ESkillAutoCondition : uint8
{
	Always,
	BossEliteOnly,
	HpBelow,
	MaintainBuff
};

// 스킬별 자동 발동 규칙. 미설정 스킬은 Always(기존 동작).
USTRUCT(BlueprintType)
struct FSkillAutoRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	FName SkillId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	ESkillAutoCondition Condition = ESkillAutoCondition::Always;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HpThresholdPct = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Automation")
	int32 Priority = 0;
};
```

- [ ] **Step 2: 서비스 선언 추가 (AutomationPolicyService.h)**

`UAutomationPolicyService` 의 static 블록(`EfficiencyUpgradeCost` 다음)에 추가:

```cpp
	// 스킬 발동 규칙 평가(서버 evaluateSkillRule 1:1). hpThreshold/selfHpPct [0,1] 클램프.
	static bool EvaluateSkillRule(
		ESkillAutoCondition Condition,
		float HpThresholdPct,
		float SelfHpPct,
		bool bIsBossElite,
		bool bBuffActive);
```

정책 상태 블록(`PushDeathThreshold` 접근자 다음)에 추가:

```cpp
	const TArray<FSkillAutoRule>& GetSkillRules() const { return SkillRules; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetSkillRule(const FSkillAutoRule& Rule); // 같은 SkillId 있으면 교체, 없으면 추가

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void ClearSkillRule(FName SkillId);

	void RestoreSkillRules(const TArray<FSkillAutoRule>& InRules);
```

private 멤버에 추가:

```cpp
	UPROPERTY()
	TArray<FSkillAutoRule> SkillRules;
```

- [ ] **Step 3: 구현 추가 (AutomationPolicyService.cpp)**

```cpp
bool UAutomationPolicyService::EvaluateSkillRule(
	ESkillAutoCondition Condition,
	float HpThresholdPct,
	float SelfHpPct,
	bool bIsBossElite,
	bool bBuffActive)
{
	switch (Condition)
	{
	case ESkillAutoCondition::BossEliteOnly:
		return bIsBossElite;
	case ESkillAutoCondition::HpBelow:
		return FMath::Clamp(SelfHpPct, 0.0f, 1.0f) <= FMath::Clamp(HpThresholdPct, 0.0f, 1.0f);
	case ESkillAutoCondition::MaintainBuff:
		return !bBuffActive;
	default:
		return true; // Always
	}
}

void UAutomationPolicyService::SetSkillRule(const FSkillAutoRule& Rule)
{
	for (FSkillAutoRule& Existing : SkillRules)
	{
		if (Existing.SkillId == Rule.SkillId)
		{
			Existing = Rule;
			return;
		}
	}
	SkillRules.Add(Rule);
}

void UAutomationPolicyService::ClearSkillRule(FName SkillId)
{
	SkillRules.RemoveAll([SkillId](const FSkillAutoRule& R) { return R.SkillId == SkillId; });
}

void UAutomationPolicyService::RestoreSkillRules(const TArray<FSkillAutoRule>& InRules)
{
	SkillRules = InRules;
}
```

- [ ] **Step 4: parity 회귀 테스트 추가**

`AutomationPolicyServiceTests.cpp`에 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationSkillRuleTest,
	"IdleProject.GameCore.Automation.SkillRule",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationSkillRuleTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	// Always
	TestTrue(TEXT("always"), S::EvaluateSkillRule(ESkillAutoCondition::Always, 0.3f, 1.0f, false, false));
	// BossEliteOnly
	TestTrue(TEXT("boss true"), S::EvaluateSkillRule(ESkillAutoCondition::BossEliteOnly, 0.3f, 1.0f, true, false));
	TestFalse(TEXT("boss false"), S::EvaluateSkillRule(ESkillAutoCondition::BossEliteOnly, 0.3f, 1.0f, false, false));
	// HpBelow
	TestTrue(TEXT("hp at threshold"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 0.3f, 0.3f, false, false));
	TestFalse(TEXT("hp above"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 0.3f, 0.31f, false, false));
	// HpBelow 클램프
	TestTrue(TEXT("hp clamp hi"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 5.0f, 1.0f, false, false));
	// MaintainBuff
	TestTrue(TEXT("buff inactive"), S::EvaluateSkillRule(ESkillAutoCondition::MaintainBuff, 0.3f, 1.0f, false, false));
	TestFalse(TEXT("buff active"), S::EvaluateSkillRule(ESkillAutoCondition::MaintainBuff, 0.3f, 1.0f, false, true));
	// SetSkillRule 교체
	UAutomationPolicyService* Svc = NewObject<UAutomationPolicyService>();
	FSkillAutoRule R; R.SkillId = FName(TEXT("warrior_slash")); R.Condition = ESkillAutoCondition::BossEliteOnly;
	Svc->SetSkillRule(R);
	TestEqual(TEXT("one rule"), Svc->GetSkillRules().Num(), 1);
	R.Condition = ESkillAutoCondition::HpBelow; Svc->SetSkillRule(R);
	TestEqual(TEXT("replace not add"), Svc->GetSkillRules().Num(), 1);
	TestEqual(TEXT("replaced cond"), (int32)Svc->GetSkillRules()[0].Condition, (int32)ESkillAutoCondition::HpBelow);
	Svc->ClearSkillRule(FName(TEXT("warrior_slash")));
	TestEqual(TEXT("cleared"), Svc->GetSkillRules().Num(), 0);
	return true;
}
```

- [ ] **Step 5: 빌드 + 좁은 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.Automation"`
Expected: 빌드 성공 + `IdleProject.GameCore.Automation.SkillRule` PASS (기존 Automation 회귀 없음)

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/GameCore/AutomationTypes.h client/Source/IdleProject/GameCore/AutomationPolicyService.h client/Source/IdleProject/GameCore/AutomationPolicyService.cpp client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp
git commit -m "feat(client): 스킬 자동 전술 타입+EvaluateSkillRule+규칙 CRUD+회귀"
```

---

## Task 3: 세이브 — SkillRules + SaveVer 27

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleSaveGame.h`
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` (저장/복원)

- [ ] **Step 1: 세이브 필드 + 버전**

`IdleSaveGame.h`에서 `int32 SaveVersion = 26;` → `27`로 변경(주석도 갱신):

```cpp
	// SaveVer 27: 자동화 스킬 규칙(SkillRules) 추가. <27 세이브는 빈 규칙(전부 Always) 마이그레이션.
	int32 SaveVersion = 27;
```

P1 자동화 필드(`AutomationPushDeathThreshold`) 다음에 추가:

```cpp
	// 자동화 스킬 자동 전술 규칙(P2). SaveVer 27+. 클라 세이브 권위.
	UPROPERTY()
	TArray<FSkillAutoRule> AutomationSkillRules;
```

- [ ] **Step 2: 저장 직렬화**

`IdleGameInstance.cpp` 저장 함수의 자동화 정책 직렬화 블록(`SaveGame->AutomationPushDeathThreshold = ...;` 다음)에 추가:

```cpp
		SaveGame->AutomationSkillRules = AutomationPolicyService->GetSkillRules();
```

- [ ] **Step 3: 복원 + 마이그레이션**

복원 함수의 자동화 RestoreState 블록 안, `>= 26` 분기 내부에 SkillRules 복원을 추가하고, `< 26` else는 빈 규칙 유지. 구체적으로 기존:

```cpp
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
			AutomationPolicyService->RestoreState(EProgressionMode::Advance, 1, true, 3);
		}
```

뒤(이 if/else 블록 다음)에 추가:

```cpp
		// 스킬 규칙은 SaveVer 27+ 에서만 존재. 미만이면 빈(전부 Always).
		AutomationPolicyService->RestoreSkillRules(
			SaveGame->SaveVersion >= 27 ? SaveGame->AutomationSkillRules : TArray<FSkillAutoRule>());
```

- [ ] **Step 4: 빌드 + 세이브 round-trip**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore"`
Expected: 빌드 성공. **SaveVer 27 단언 실패가 다수 나올 수 있음**(기존 26 기대 테스트). 이는 Task 4에서 일괄 갱신. 빌드 성공 자체는 확인.

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp
git commit -m "feat(client): 자동화 스킬 규칙 세이브 + SaveVer 27"
```

---

## Task 4: SaveVer 26→27 stale 단언 일괄 갱신

**Files (수정):** 아래 테스트 파일들의 "현재 세이브 버전" 단언.

> P1에서 25→26으로 갱신한 단언들을 27로 올린다. **각 파일에서 "현재/캡처/기본 세이브 버전"을 26으로 단언하는 곳만** 변경하고, 마이그레이션 SOURCE(과거 v7/v15 등 입력)는 건드리지 않는다. 메시지 문자열도 `V26`/`v26`/`(26)` → `V27`/`v27`/`(27)`.

- [ ] **Step 1: 대상 일괄 갱신**

`grep -rn "to be 26\|current version (26)\|V26\|v26\|version is current (26)\|version (26)" client/Source/IdleProject/Tests/` 로 후보를 찾고, "현재 버전" 단언을 27로 변경. P1에서 갱신된 동일 13개 파일이 대상:

`ConsumableTests.cpp`, `AttendanceServiceTests.cpp`, `DungeonServiceTests.cpp`, `MissionServiceTests.cpp`, `SaveSystemTests.cpp`(다수: SaveGameDefaults/legacy/consumable/mastery/weeklyBoss capture), `TitleServiceTests.cpp`, `TreasureBoxServiceTests.cpp`(+ 주석 `v26`/`<26`), `GuildTests.cpp`(SaveRoundTripV19 capture), `RarityMigrationTests.cpp`, `MasteryTests.cpp`, `RebirthPerkServiceTests.cpp`, `RuneServiceTests.cpp`, `RuneSetServiceTests.cpp`(2건).

각 변경은 `TestEqual(TEXT("...V26"), SaveGame->SaveVersion, 26)` → `27`. 정확한 위치는 메시지 문자열로 식별.

- [ ] **Step 2: 잔여 확인**

Run: `grep -rn "SaveVersion, .*26\|to be 26" client/Source/IdleProject/Tests/`
Expected: 현재-버전 단언에 26 잔존 0 (마이그레이션 SOURCE의 7/15 등은 무관, 유지).

- [ ] **Step 3: 커밋**

```bash
git add client/Source/IdleProject/Tests/
git commit -m "test(client): SaveVer 26→27 stale 단언 일괄 갱신 (P2 SaveVer 범프)"
```

---

## Task 5: SkillComponent — 규칙 게이트 + 우선순위

**Files:**
- Modify: `client/Source/IdleProject/CombatSystem/SkillComponent.h` / `.cpp`
- Test: `client/Source/IdleProject/Tests/CombatTests.cpp`

- [ ] **Step 1: 헤더 — SetAutoRules + 시그니처 기본 인자**

`SkillComponent.h` 상단 include에 추가:

```cpp
#include "GameCore/AutomationTypes.h"
```

`TickSkills` 선언을 기본 인자 추가로 변경:

```cpp
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets, bool bIsBossElite = false);

	// 자동화 정책의 스킬 규칙 주입(빈=전부 Always, 기존 동작).
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void SetAutoRules(const TArray<FSkillAutoRule>& InRules);
```

private에 추가:

```cpp
	UPROPERTY()
	TArray<FSkillAutoRule> AutoRules;

	// SkillId 의 규칙(없으면 Always 기본).
	FSkillAutoRule GetRuleFor(FName SkillId) const;

	// MaintainBuff 용: 해당 스킬 버프가 아직 활성인지(LastCast + BuffDuration).
	bool IsSkillBuffActive(const FSkillDefinition& Skill, float Now) const;
```

- [ ] **Step 2: 구현 — SetAutoRules/GetRuleFor/IsSkillBuffActive**

`SkillComponent.cpp`에 추가:

```cpp
void USkillComponent::SetAutoRules(const TArray<FSkillAutoRule>& InRules)
{
	AutoRules = InRules;
}

FSkillAutoRule USkillComponent::GetRuleFor(FName SkillId) const
{
	for (const FSkillAutoRule& Rule : AutoRules)
	{
		if (Rule.SkillId == SkillId)
		{
			return Rule;
		}
	}
	return FSkillAutoRule{}; // 기본 Always
}

bool USkillComponent::IsSkillBuffActive(const FSkillDefinition& Skill, float Now) const
{
	if (Skill.BuffDuration <= 0.0f)
	{
		return false;
	}
	const float* LastCast = LastCastTimeBySkill.Find(Skill.SkillId);
	return LastCast && (Now - *LastCast) < Skill.BuffDuration;
}
```

- [ ] **Step 3: TickSkills 게이트 적용 (핵심)**

`TickSkills` 본문을 규칙 게이트 + 우선순위로 교체:

```cpp
void USkillComponent::TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets, bool bIsBossElite)
{
	UpdateTimedBuffs(Now);

	// 소유자 HP% (Combat 없으면 1.0). 클램프.
	float SelfHpPct = 1.0f;
	if (const UCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr)
	{
		const float MaxHp = FMath::Max(1.0f, Combat->MaxHp);
		SelfHpPct = FMath::Clamp(Combat->CurrentHp / MaxHp, 0.0f, 1.0f);
	}

	// 평가 순서: Priority 오름차순(동률 선언 순서 안정 유지). 궁극기 기본 우선.
	TArray<int32> Order;
	Order.Reserve(Skills.Num());
	for (int32 i = 0; i < Skills.Num(); ++i) { Order.Add(i); }
	Order.StableSort([this](int32 A, int32 B)
	{
		const int32 PrA = EffectivePriority(Skills[A]);
		const int32 PrB = EffectivePriority(Skills[B]);
		return PrA < PrB;
	});

	bool bUltimateFired = false;
	for (int32 Idx : Order)
	{
		const FSkillDefinition& Skill = Skills[Idx];
		if (Skill.Type == ESkillType::Passive)
		{
			continue;
		}
		const FSkillAutoRule Rule = GetRuleFor(Skill.SkillId);
		const bool bBuffActive = IsSkillBuffActive(Skill, Now);
		if (!UAutomationPolicyService::EvaluateSkillRule(Rule.Condition, Rule.HpThresholdPct, SelfHpPct, bIsBossElite, bBuffActive))
		{
			continue;
		}
		if (Skill.Type == ESkillType::Ultimate)
		{
			if (!bUltimateFired && IsUltimateReady())
			{
				ExecuteSkill(Skill, Now, Target, AoeTargets);
				bUltimateFired = true;
			}
			continue;
		}
		// Active
		if (IsReady(Skill.SkillId, Now))
		{
			ExecuteSkill(Skill, Now, Target, AoeTargets);
		}
	}
}

int32 USkillComponent::EffectivePriority(const FSkillDefinition& Skill) const
{
	// 궁극기는 기본 우선(-1), 규칙 Priority 가 명시되면 그 값 사용.
	const FSkillAutoRule Rule = GetRuleFor(Skill.SkillId);
	if (Rule.SkillId == Skill.SkillId)
	{
		return Rule.Priority;
	}
	return Skill.Type == ESkillType::Ultimate ? -1 : 0;
}
```

`SkillComponent.h` private에 `EffectivePriority` 선언 추가:

```cpp
	int32 EffectivePriority(const FSkillDefinition& Skill) const;
```

> 주: 기존 TickSkills 의 "궁극기 먼저 발동 후 return" 의미는 EffectivePriority(-1) + bUltimateFired 가드로 보존된다. 규칙 미설정 시 모든 Active 는 우선순위 0 동률 → 선언 순서 = 기존 동작. **기존 CombatTests(규칙 미설정, bIsBossElite 기본 false)는 동작 불변이어야 한다.**

- [ ] **Step 4: 기존 회귀 + 신규 게이트 테스트**

`CombatTests.cpp` 끝에 신규 테스트 추가(규칙 게이트 검증):

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillAutoRuleGateTest,
	"IdleProject.Combat.SkillAutoRuleGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillAutoRuleGateTest::RunTest(const FString& Parameters)
{
	// 액터 + Combat + Skill 구성(기존 CombatTests 헬퍼 패턴 사용 — 동일 파일 내 기존 셋업 참고).
	AActor* Caster = GetWorld() ? GetWorld()->SpawnActor<AActor>() : nullptr;
	if (!Caster) { AddError(TEXT("No world")); return false; }
	UCombatComponent* Combat = NewObject<UCombatComponent>(Caster);
	Combat->RegisterComponent();
	Combat->InitializeCombat(100.0f, 50.0f, 0.0f, 1.0f);
	USkillComponent* Skills = NewObject<USkillComponent>(Caster);
	Skills->RegisterComponent();
	Skills->LoadDefaultWarriorSkills();

	AActor* Target = GetWorld()->SpawnActor<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	TargetCombat->RegisterComponent();
	TargetCombat->InitializeCombat(100000.0f, 1.0f, 0.0f, 1.0f);

	// 첫 액티브 스킬 id 확보.
	FName FirstActive = NAME_None;
	for (const FSkillDefinition& S : Skills->GetSkillsForTest()) // 주: 노출 게터 없으면 아래 대안 사용
	{
		if (S.Type == ESkillType::Active) { FirstActive = S.SkillId; break; }
	}
	TestTrue(TEXT("has active skill"), FirstActive != NAME_None);

	// BossEliteOnly 규칙 → 일반 전투(bIsBossElite=false)에서 억제.
	TArray<FSkillAutoRule> Rules;
	FSkillAutoRule Rule; Rule.SkillId = FirstActive; Rule.Condition = ESkillAutoCondition::BossEliteOnly;
	Rules.Add(Rule);
	Skills->SetAutoRules(Rules);

	const float HpBefore = TargetCombat->CurrentHp;
	Skills->TickSkills(100.0f, Target, TArray<AActor*>{ Target }, /*bIsBossElite=*/false);
	TestEqual(TEXT("boss-only skill suppressed in normal fight"), TargetCombat->CurrentHp, HpBefore);

	// 보스 전투면 발동(쿨다운 리셋 위해 Now 진행).
	Skills->TickSkills(200.0f, Target, TArray<AActor*>{ Target }, /*bIsBossElite=*/true);
	TestTrue(TEXT("boss-only skill fires vs boss"), TargetCombat->CurrentHp < HpBefore);

	return true;
}
```

> 주: `GetSkillsForTest()` 가 없으면, 이 테스트는 첫 액티브 스킬 id 를 `LoadDefaultWarriorSkills` 후 알려진 상수(예: `warrior_slash`)로 직접 지정하라. CombatTests.cpp 내 기존 테스트에서 사용하는 스킬 id 명을 grep 으로 확인해 사용한다(예: `grep -n "warrior_" client/Source/IdleProject/CombatSystem/SkillComponent.cpp`). 헬퍼/스폰 패턴도 같은 파일의 기존 IMPLEMENT_SIMPLE_AUTOMATION_TEST 케이스를 따른다.

- [ ] **Step 5: 빌드 + Combat Automation 전체**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.Combat"`
Expected: 빌드 성공 + 기존 Combat 테스트 전부 GREEN(규칙 미설정 동작 불변) + `SkillAutoRuleGate` PASS

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/CombatSystem/SkillComponent.h client/Source/IdleProject/CombatSystem/SkillComponent.cpp client/Source/IdleProject/Tests/CombatTests.cpp
git commit -m "feat(client): TickSkills 규칙 게이트+우선순위(미설정=기존 동작)"
```

---

## Task 6: BattleAI 컨텍스트 + 플레이어 규칙 동기 + HUD 진입점

**Files:**
- Modify: `client/Source/IdleProject/CombatSystem/BattleAIComponent.cpp`
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h` / `.cpp`

- [ ] **Step 1: BattleAI — bIsBossElite 전달**

`BattleAIComponent.cpp`의 `Skills->TickSkills(GetWorld()->GetTimeSeconds(), TargetActor, AoeTargets);`(약 161행)을 다음으로 변경:

```cpp
			const AIdleMonster* TargetMonster = Cast<AIdleMonster>(TargetActor);
			const bool bTargetBossElite = TargetMonster && (TargetMonster->IsBoss() || TargetMonster->IsElite());
			Skills->TickSkills(GetWorld()->GetTimeSeconds(), TargetActor, AoeTargets, bTargetBossElite);
```

`BattleAIComponent.cpp` 상단에 include 필요 시 추가(이미 IdleMonster 사용 중이면 생략):

```cpp
#include "CharacterSystem/IdleMonster.h"
```

(상단 include 목록 확인 후 없을 때만 추가.)

- [ ] **Step 2: GameInstance — BP 규칙 접근자 + 플레이어 동기**

`IdleGameInstance.h` public 에 추가:

```cpp
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationSkillRule(const FSkillAutoRule& Rule);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void ClearAutomationSkillRule(FName SkillId);

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	TArray<FSkillAutoRule> GetAutomationSkillRules() const;

	// 정책의 스킬 규칙을 지정 SkillComponent 에 주입(스킬 로드/규칙 변경 시).
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SyncSkillRulesTo(USkillComponent* SkillComp);
```

`IdleGameInstance.cpp`에 구현:

```cpp
void UIdleGameInstance::SetAutomationSkillRule(const FSkillAutoRule& Rule)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		AutomationPolicyService->SetSkillRule(Rule);
	}
}

void UIdleGameInstance::ClearAutomationSkillRule(FName SkillId)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService)
	{
		AutomationPolicyService->ClearSkillRule(SkillId);
	}
}

TArray<FSkillAutoRule> UIdleGameInstance::GetAutomationSkillRules() const
{
	const UAutomationPolicyService* Service = GetAutomationPolicyService();
	return Service ? Service->GetSkillRules() : TArray<FSkillAutoRule>();
}

void UIdleGameInstance::SyncSkillRulesTo(USkillComponent* SkillComp)
{
	EnsureAutomationPolicyService();
	if (SkillComp && AutomationPolicyService)
	{
		SkillComp->SetAutoRules(AutomationPolicyService->GetSkillRules());
	}
}
```

`IdleGameInstance.cpp` 상단 include에 필요 시 `#include "CombatSystem/SkillComponent.h"` 추가(없을 때만).

> 주: `GetAutomationPolicyService()`는 P1에서 추가된 lazy-ensure const 게터. `SyncSkillRulesTo` 호출 지점(플레이어 캐릭터 스킬 로드 직후)은 기존 코드에서 플레이어 SkillComponent 를 초기화/스킬 로드하는 곳을 찾아 1줄 호출 추가한다. 해당 지점이 모호하면 DONE_WITH_CONCERNS 로 보고(플레이어 SkillComponent 초기화 위치 file:line 명시). BP 호출 진입점은 위 함수들로 이미 노출되므로, 동기 호출 누락은 HUD/BP 에서 보완 가능(블로킹 아님).

- [ ] **Step 3: 빌드 + 전체 Automation(HUD 포함)**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject"`
Expected: 표준 jumbo 빌드 성공(ODR 0) + 전체 Automation GREEN(`IdleProject.Combat`/`IdleProject.GameCore.Automation`/`IdleProject.UI.HUD`/SaveSystem v27 포함)

- [ ] **Step 4: 커밋**

```bash
git add client/Source/IdleProject/CombatSystem/BattleAIComponent.cpp client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp
git commit -m "feat(client): BattleAI 보스/엘리트 컨텍스트 + 스킬 규칙 BP 진입점/동기"
```

---

## Task 7: 최종 게이트 — 전체 회귀 + lint + SaveVer 점검

**Files:** 검증 전용(stale 발견 시 해당 파일만).

- [ ] **Step 1: 서버 전체 + lint**

Run: `cd server; npm test`
Expected: 전체 GREEN (evaluateSkillRule 포함)

Run: `cd server; npx biome check . ../tools/balance-sim`
Expected: clean

- [ ] **Step 2: UE 표준 jumbo + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1`
Expected: Result Fail 0, EXIT 0. SaveVer 27 단언 GREEN

- [ ] **Step 3: SaveVer stale 점검**

Run: `grep -rn "SaveVersion, .*26\|to be 26\|current version (26)" client/Source/IdleProject/Tests/`
Expected: 현재-버전 단언 26 잔존 0 (마이그 SOURCE 제외)

- [ ] **Step 4: 최종 커밋(보정 있었던 경우만)**

```bash
git add -A
git commit -m "chore: 자동화 P2 SaveVer 27 stale 점검/보정"
```

---

## Self-Review (작성자 점검)

**스펙 커버리지:**
- §3 데이터(ESkillAutoCondition/FSkillAutoRule/SkillRules) → Task 2,3 ✅
- §4 parity evaluateSkillRule → Task 1,2 ✅
- §5 TickSkills 게이트(내부 HP%/buffActive/bIsBossElite 기본인자/우선순위/궁극기 보존) → Task 5,6 ✅
- §6 해금(SkillTactics ch3, P1 isFeatureUnlocked 재사용 — HUD 게이트에서 사용) → Task 6 BP 진입점(해금 판정은 P1 `IsAutomationFeatureUnlocked(SkillTactics)` 재사용) ✅
- §7 가드(클램프/폴백 Always/데드락 없음) → Task 1,2,5 ✅
- §8 테스트/게이트(parity·게이트 회귀·SaveVer27·jumbo·biome) → Task 1~7 ✅

**P2 제외(스펙 §2):** 슬롯 제한/효율 sink/자동장비/소비 → 미포함(의도적) ✅

**플레이스홀더:** "주:" 는 기존 코드의 정확한 위치/스킬 id 확인 안내(실코드 블록 제공). TODO/TBD 없음. 단 Task 5 Step 4 의 `GetSkillsForTest()`는 대안(상수 id) 명시 — 구현자가 실제 스킬 id 로 확정.

**타입 일관성:** `ESkillAutoCondition`/`FSkillAutoRule`/`evaluateSkillRule`↔`EvaluateSkillRule`/`SetAutoRules`/`GetRuleFor`/`IsSkillBuffActive`/`EffectivePriority`/`SetSkillRule`/`ClearSkillRule`/`RestoreSkillRules`/`AutomationSkillRules` — 서버/클라/세이브 명칭 정합.

**SaveVer 27 부작용:** Task 4가 stale 단언 일괄 갱신(P1 26→ 경험). Task 3 Step 4에서 빌드만 확인, 테스트 GREEN은 Task 4 후.
