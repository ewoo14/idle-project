# 통합 자동화 시스템 P4 (자동 버프 유지 + 무한 효율 업그레이드) 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`) syntax.

**Goal:** 자동 버프 유지(만료 시 보유 소비 자동 재사용)와 무한 효율 업그레이드(자동 매각가 보너스)를 추가해 자동화 4기둥을 완결한다.

**Architecture:** P1~P3 골격 재사용. 매각가 보너스/비용은 순수 함수 parity(서버/클라 미러). 자동 버프 유지는 GameInstance 헬퍼를 `ScheduleRespawn`(처치 틱)에서 호출. 매각가 곱은 GameInstance 매각 지점에서 적용(InventoryComponent는 정책 비의존).

**Tech Stack:** 서버 TS+vitest+biome. 클라 UE5 C++. 게이트 `tools/ci/ue-automation.ps1`.

**스펙:** `docs/superpowers/specs/2026-05-30-automation-system-p4-auto-consumable-design.md`
**선행:** P1(#99)+P2(#100)+P3(#101). 브랜치 `feat/automation-system-p4`.

**⚠️ 두 교훈(P2/P3):**
- **SaveVer 28→29 = 3곳**: ① `IdleSaveGame.h` 헤더 기본값 ② `IdleGameInstance.cpp CaptureToSave` writer(`SaveGame->SaveVersion = 28;`) ③ 전 테스트 단언.
- **C4458 멤버 셰도잉 금지**: 로컬 변수명을 클래스 멤버명(`Gold`/`Exp`/`Atk` 등)과 다르게(예: `GoldGained`, `Cost`).

**P4 제외:** HP 회복 자동 포션(회복 아이템 부재), 버프 마스크, 자동 강화, 다중 효율 트랙.

---

## 파일 구조

| 파일 | 책임 | 신규/수정 |
|---|---|---|
| `server/src/core/formulas/automation.ts` | `sellValueMultiplier` 추가 | 수정 |
| `server/src/core/formulas/automation.test.ts` | 테스트 | 수정 |
| `client/Source/IdleProject/GameCore/AutomationPolicyService.h/.cpp` | 2 정책 필드 + `GetSellValueMultiplier`/`SellUpgradeNextCost` + Restore 확장 | 수정 |
| `client/Source/IdleProject/GameCore/IdleSaveGame.h` | 2 필드 + SaveVer 29 | 수정 |
| `client/Source/IdleProject/GameCore/IdleGameInstance.h/.cpp` | MaintainBuffsIfEnabled/UpgradeSellValue/매각가 곱/BP | 수정 |
| `client/Source/IdleProject/IdleProjectGameModeBase.cpp` | ScheduleRespawn 자동 버프 후크 | 수정 |
| `client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp` | 효율 parity + GameInstance 회귀 | 수정 |
| 다수 `Tests/*ServiceTests.cpp` | SaveVer 28→29 stale 일괄 | 수정 |

---

## Task 1: 서버 parity — sellValueMultiplier

**Files:** Modify `automation.ts` + `automation.test.ts`.

- [ ] **Step 1: 실패 테스트 추가**

`automation.test.ts` 끝에:

```ts
import { sellValueMultiplier } from "./automation.js";

describe("sellValueMultiplier — 자동 매각가 효율 보너스", () => {
  it("레벨 0 = ×1", () => {
    expect(sellValueMultiplier(0)).toBe(1);
  });
  it("레벨당 +0.02", () => {
    expect(sellValueMultiplier(1)).toBeCloseTo(1.02, 10);
    expect(sellValueMultiplier(10)).toBeCloseTo(1.2, 10);
  });
  it("음수 0가드", () => {
    expect(sellValueMultiplier(-5)).toBe(1);
  });
});
```

- [ ] **Step 2: 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts`
Expected: FAIL — `sellValueMultiplier` 미export

- [ ] **Step 3: 구현 추가**

`automation.ts` 끝에:

```ts
// 자동 매각가 효율 보너스(P4). 레벨당 +2%, 무한. 음수 0가드.
// 클라 UAutomationPolicyService::GetSellValueMultiplier 1:1 미러.
export function sellValueMultiplier(level: number): number {
  return 1 + 0.02 * Math.max(0, Math.trunc(level));
}
```

- [ ] **Step 4: 통과 + 전체 + lint**

Run: `cd server; npx vitest run src/core/formulas/automation.test.ts` → PASS
Run: `cd server; npm test` → GREEN
Run: `cd server; npx biome check . ../tools/balance-sim` → clean(위반 시 `--write` 후 재확인)

- [ ] **Step 5: 커밋**

```bash
git add server/src/core/formulas/automation.ts server/src/core/formulas/automation.test.ts
git commit -m "feat(server): 자동 매각가 효율 보너스 sellValueMultiplier parity"
```

---

## Task 2: 클라 정책 — 효율 보너스/비용 + 2필드

**Files:** Modify `AutomationPolicyService.h/.cpp`; Test `AutomationPolicyServiceTests.cpp`.

- [ ] **Step 1: 헤더 — static + 상태**

`AutomationPolicyService.h` static 블록에 추가:

```cpp
	// 자동 매각가 효율 보너스(서버 sellValueMultiplier 1:1). 1 + 0.02×max(0,level).
	static float GetSellValueMultiplier(int32 Level);

	// 매각가 업그레이드 다음 비용(P1 EfficiencyUpgradeCost 재사용, base 50000 × 1.5^level).
	static int64 SellUpgradeNextCost(int32 Level);
```

정책 상태 접근자(기존 gear 접근자 다음):

```cpp
	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoMaintainBuff() const { return bAutoMaintainBuff; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoMaintainBuff(bool b) { bAutoMaintainBuff = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int32 GetSellValueUpgradeLevel() const { return SellValueUpgradeLevel; }
	void SetSellValueUpgradeLevel(int32 L) { SellValueUpgradeLevel = FMath::Max(0, L); }

	void RestoreConsumablePolicy(bool bInMaintain, int32 InSellLevel);
```

private 멤버:

```cpp
	UPROPERTY()
	bool bAutoMaintainBuff = false;
	UPROPERTY()
	int32 SellValueUpgradeLevel = 0;
```

- [ ] **Step 2: 구현**

`AutomationPolicyService.cpp`:

```cpp
float UAutomationPolicyService::GetSellValueMultiplier(int32 Level)
{
	return 1.0f + 0.02f * static_cast<float>(FMath::Max(0, Level));
}

int64 UAutomationPolicyService::SellUpgradeNextCost(int32 Level)
{
	// base 50000, growth 1.5, P1 EfficiencyUpgradeCost 재사용(기하급수 무한).
	return EfficiencyUpgradeCost(50000, 1.5f, Level);
}

void UAutomationPolicyService::RestoreConsumablePolicy(bool bInMaintain, int32 InSellLevel)
{
	bAutoMaintainBuff = bInMaintain;
	SellValueUpgradeLevel = FMath::Max(0, InSellLevel);
}
```

- [ ] **Step 3: parity 회귀 테스트**

`AutomationPolicyServiceTests.cpp`에 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationSellUpgradeTest,
	"IdleProject.GameCore.Automation.SellUpgrade",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationSellUpgradeTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	TestEqual(TEXT("mult lv0"), S::GetSellValueMultiplier(0), 1.0f);
	TestTrue(TEXT("mult lv10"), FMath::IsNearlyEqual(S::GetSellValueMultiplier(10), 1.2f, 1e-4f));
	TestEqual(TEXT("mult neg guard"), S::GetSellValueMultiplier(-5), 1.0f);
	TestEqual(TEXT("cost lv0 base"), S::SellUpgradeNextCost(0), (int64)50000);
	TestTrue(TEXT("cost grows"), S::SellUpgradeNextCost(5) > S::SellUpgradeNextCost(4));
	// 상태 setter 클램프
	UAutomationPolicyService* Svc = NewObject<UAutomationPolicyService>();
	Svc->SetSellValueUpgradeLevel(-3);
	TestEqual(TEXT("level clamp"), Svc->GetSellValueUpgradeLevel(), 0);
	return true;
}
```

- [ ] **Step 4: 빌드 + 좁은 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.Automation"`
Expected: 빌드 성공 + SellUpgrade PASS

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/AutomationPolicyService.h client/Source/IdleProject/GameCore/AutomationPolicyService.cpp client/Source/IdleProject/Tests/AutomationPolicyServiceTests.cpp
git commit -m "feat(client): 자동 매각가 효율 보너스/비용 + 소비 정책 필드 + 회귀"
```

---

## Task 3: 세이브 SaveVer 29 (3곳) + 직렬화/복원

**Files:** Modify `IdleSaveGame.h`, `IdleGameInstance.cpp`.

- [ ] **Step 1: 헤더 + 2필드 + 버전**

`IdleSaveGame.h`: `int32 SaveVersion = 28;` → `29`(주석 갱신). P3 필드 다음:

```cpp
	// 자동 소비/효율 정책(P4). SaveVer 29+.
	UPROPERTY()
	bool bAutomationAutoMaintainBuff = false;
	UPROPERTY()
	int32 AutomationSellValueUpgradeLevel = 0;
```

- [ ] **Step 2: CaptureToSave writer (필수)**

`IdleGameInstance.cpp CaptureToSave`의 `SaveGame->SaveVersion = 28;` → `29`. **(P2/P3 교훈 — 이 writer 라인)**

- [ ] **Step 3: 직렬화 + 복원**

저장(P3 gear 정책 직렬화 다음):

```cpp
		SaveGame->bAutomationAutoMaintainBuff = AutomationPolicyService->GetAutoMaintainBuff();
		SaveGame->AutomationSellValueUpgradeLevel = AutomationPolicyService->GetSellValueUpgradeLevel();
```

복원(P3 `RestoreGearPolicy` 호출 다음):

```cpp
		if (SaveGame->SaveVersion >= 29)
		{
			AutomationPolicyService->RestoreConsumablePolicy(
				SaveGame->bAutomationAutoMaintainBuff,
				SaveGame->AutomationSellValueUpgradeLevel);
		}
		else
		{
			AutomationPolicyService->RestoreConsumablePolicy(false, 0);
		}
```

- [ ] **Step 4: 빌드 확인**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.SaveSystem"`
Expected: **빌드 성공**(SaveVer 29 단언은 Task 5 후 GREEN). 컴파일 0 오류.

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp
git commit -m "feat(client): 자동 소비/효율 정책 세이브 + SaveVer 29(3곳)"
```

---

## Task 4: GameInstance — 자동 버프 유지 + 효율 업그레이드 + 매각가 곱

**Files:** Modify `IdleGameInstance.h/.cpp`, `IdleProjectGameModeBase.cpp`.

- [ ] **Step 1: 헤더 선언**

`IdleGameInstance.h` public:

```cpp
	// 자동 버프 유지: ON 시 만료된 버프를 보유분으로 자동 재사용(처치 틱에서 호출).
	void MaintainBuffsIfEnabled();

	// 자동 매각가 효율 업그레이드(골드 충분 시 레벨++). 성공 여부.
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	bool UpgradeSellValue();

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	int64 GetSellValueUpgradeCost() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoMaintainBuff(bool bValue);
```

- [ ] **Step 2: 구현**

`IdleGameInstance.cpp`:

```cpp
void UIdleGameInstance::MaintainBuffsIfEnabled()
{
	EnsureAutomationPolicyService();
	EnsureBuffService();
	if (!AutomationPolicyService || !BuffService || !AutomationPolicyService->GetAutoMaintainBuff())
	{
		return;
	}
	const int64 Now = GetCurrentUnixSeconds();
	static const EConsumableType Types[] = {
		EConsumableType::AttackTonic, EConsumableType::GuardTonic, EConsumableType::AllStatElixir,
		EConsumableType::FortuneScroll, EConsumableType::GoldFeast, EConsumableType::WisdomBooster };
	bool bUsedAny = false;
	for (const EConsumableType Type : Types)
	{
		if (!BuffService->IsBuffActive(Type, Now) && BuffService->GetTotalCount(Type) > 0)
		{
			if (BuffService->UseConsumable(Type, Now))
			{
				bUsedAny = true;
			}
		}
	}
	if (bUsedAny)
	{
		RefreshPlayerCharacterStats();
	}
}

int64 UIdleGameInstance::GetSellValueUpgradeCost() const
{
	const UAutomationPolicyService* Service = GetAutomationPolicyService();
	const int32 Level = Service ? Service->GetSellValueUpgradeLevel() : 0;
	return UAutomationPolicyService::SellUpgradeNextCost(Level);
}

bool UIdleGameInstance::UpgradeSellValue()
{
	EnsureAutomationPolicyService();
	if (!AutomationPolicyService)
	{
		return false;
	}
	const int32 Level = AutomationPolicyService->GetSellValueUpgradeLevel();
	const int64 UpgradeCost = UAutomationPolicyService::SellUpgradeNextCost(Level);
	if (Gold < UpgradeCost)
	{
		return false;
	}
	AddGold(-UpgradeCost);
	AutomationPolicyService->SetSellValueUpgradeLevel(Level + 1);
	return true;
}

void UIdleGameInstance::SetAutomationAutoMaintainBuff(bool bValue)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService) { AutomationPolicyService->SetAutoMaintainBuff(bValue); }
}
```

> 주: `Gold`/`AddGold`는 기존 멤버/메서드. C4458 회피 위해 로컬은 `UpgradeCost` 사용(멤버 아님). `BuffService->UseConsumable(Type, Now)` 단일 등급 오버로드(보유 등급 자동 선택) 존재 확인(BuffService.h:37). 없으면 `GetActiveGrade`/보유 등급 조회 후 2-인자 오버로드 사용.

- [ ] **Step 3: 매각가 곱 적용 (P3 매각 지점 수정)**

`SellInventoryItem`에서 베이스 결과에 곱 적용:

```cpp
int64 UIdleGameInstance::SellInventoryItem(int32 ItemIndex)
{
	UInventoryComponent* Inv = FindPlayerInventory();
	if (!Inv) { return 0; }
	EnsureAutomationPolicyService();
	const int64 BaseGold = Inv->SellItem(ItemIndex);
	if (BaseGold <= 0) { return 0; }
	const float Mult = AutomationPolicyService
		? UAutomationPolicyService::GetSellValueMultiplier(AutomationPolicyService->GetSellValueUpgradeLevel())
		: 1.0f;
	const int64 GoldGained = static_cast<int64>(FMath::RoundToDouble(static_cast<double>(BaseGold) * Mult));
	AddGold(GoldGained);
	RefreshPlayerCharacterStats();
	return GoldGained;
}
```

`HandleDroppedEquipment` 자동 매각 골드에도 곱:

```cpp
		const int64 BaseSell = FItemSellFormula::ComputeSellValue(Item.Rarity, Item.EnhanceLevel);
		const float Mult = UAutomationPolicyService::GetSellValueMultiplier(AutomationPolicyService->GetSellValueUpgradeLevel());
		AddGold(static_cast<int64>(FMath::RoundToDouble(static_cast<double>(BaseSell) * Mult)));
		return false;
```
(기존 `AddGold(FItemSellFormula::ComputeSellValue(...))` 한 줄을 위 3줄로 교체.)

- [ ] **Step 4: ScheduleRespawn 후크**

`IdleProjectGameModeBase.cpp ScheduleRespawn`의 `GameInstance->RecordMonsterKilled();` 다음에 추가:

```cpp
		GameInstance->MaintainBuffsIfEnabled();
```

- [ ] **Step 5: 빌드 + GameCore Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore"`
Expected: 빌드 성공(SaveVer 단언은 Task 5 후). 컴파일 0(C4458 없음).

- [ ] **Step 6: 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/IdleProjectGameModeBase.cpp
git commit -m "feat(client): 자동 버프 유지 + 매각가 효율 업그레이드 + 처치 틱 후크"
```

---

## Task 5: SaveVer 28→29 stale 단언 일괄 갱신

**Files:** 동일 테스트 파일들의 "현재 세이브 버전" 단언.

- [ ] **Step 1: 일괄 갱신**

`grep -rn "to be 28\|current version (28)\|V28\|v28\|version (28)" client/Source/IdleProject/Tests/`로 현재-버전 단언을 28→29(+라벨). 마이그 SOURCE(v7/v15 등) 유지. 동일 13개 파일(P3 목록과 동일).

- [ ] **Step 2: 잔여 확인**

Run: `grep -rn "SaveVersion, .*28\|to be 28\|current version (28)" client/Source/IdleProject/Tests/`
Expected: 현재-버전 28 잔존 0.

- [ ] **Step 3: 커밋**

```bash
git add client/Source/IdleProject/Tests/
git commit -m "test(client): SaveVer 28→29 stale 단언 일괄 갱신 (P4 SaveVer 범프)"
```

---

## Task 6: GameInstance 자동 버프/효율 회귀 테스트 + HUD 진입점

**Files:** Modify `AutomationPolicyServiceTests.cpp`(또는 신규 GameInstance 테스트).

- [ ] **Step 1: GameInstance 회귀 테스트**

`AutomationPolicyServiceTests.cpp`에 추가(NewObject GameInstance 직접 테스트):

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationConsumablePolicyTest,
	"IdleProject.GameCore.Automation.ConsumablePolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationConsumablePolicyTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GI = NewObject<UIdleGameInstance>();
	GI->InitializeServicesForTests(); // 주: 기존 테스트 초기화 헬퍼명 확인(없으면 Ensure* 경유 BP API 사용)

	// 효율 업그레이드: 골드 충분 시 레벨++ & 골드 차감.
	GI->AddGold(100000);
	const int64 Cost = GI->GetSellValueUpgradeCost(); // 50000
	const bool bUp = GI->UpgradeSellValue();
	TestTrue(TEXT("upgrade succeeds"), bUp);
	TestEqual(TEXT("level 1"), GI->GetAutomationPolicyService()->GetSellValueUpgradeLevel(), 1);

	// 골드 부족 시 거부.
	GI->SetGoldForTest(0); // 주: 골드 0 세팅 헬퍼/AddGold 음수로 대체
	TestFalse(TEXT("upgrade refused when poor"), GI->UpgradeSellValue());
	return true;
}
```

> 주: `InitializeServicesForTests()`/`SetGoldForTest()`는 예시명. 기존 GameInstance 테스트(예: `RuneServiceTests`/`MissionServiceTests`)가 GameInstance를 어떻게 초기화·골드 세팅하는지 grep으로 확인해 동일 패턴 사용. 헬퍼가 없으면 `AddGold` 양수/음수로 골드를 조정하고, 서비스는 Ensure* 경유 BP API(`UpgradeSellValue`/`GetSellValueUpgradeCost`/`GetAutomationPolicyService`)만으로 검증한다. 자동 버프 유지(MaintainBuffsIfEnabled)는 BuffService에 AddConsumable 후 호출→IsBuffActive 검증으로 추가 가능(소비 추가 API `AddConsumable` 존재).

- [ ] **Step 2: HUD 진입점 확인**

자동 소비/효율 해금은 P1 `IsAutomationFeatureUnlocked(EAutomationFeature::AutoConsumable)`(환생 1회)로 판정. 토글(`SetAutomationAutoMaintainBuff`)·효율(`UpgradeSellValue`/`GetSellValueUpgradeCost`)은 Task 4에서 BP 노출. 데이터 구동 HUD가 바인딩(신규 C++ 위젯 0). UI.csv 라벨 필요 시 소비/효율 탭 라벨만 추가.

- [ ] **Step 3: 빌드 + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject"`
Expected: 표준 jumbo 빌드 성공(ODR 0) + 전체 Automation GREEN(`IdleProject.GameCore.Automation`/`Consumable`/`UI.HUD`/SaveSystem v29 포함)

- [ ] **Step 4: 커밋**

```bash
git add client/Source/IdleProject
git commit -m "feat(client): 자동 소비/효율 회귀 + HUD 진입점(AutoConsumable 환생1 해금)"
```

---

## Task 7: 최종 게이트

- [ ] **Step 1: 서버 전체 + lint**

Run: `cd server; npm test` → GREEN
Run: `cd server; npx biome check . ../tools/balance-sim` → clean

- [ ] **Step 2: UE 표준 jumbo + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1`
Expected: Fail 0, EXIT 0. SaveVer 29 GREEN

- [ ] **Step 3: SaveVer 정합 점검(3곳)**

Run: `grep -rn "SaveVersion = 2" client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/GameCore/IdleSaveGame.h`
Expected: writer = 29, 헤더 기본값 = 29.

Run: `grep -rn "to be 28\|current version (28)" client/Source/IdleProject/Tests/`
Expected: 0

- [ ] **Step 4: 최종 커밋(보정 시만)**

```bash
git add -A
git commit -m "chore: 자동화 P4 SaveVer 29 정합 점검"
```

---

## Self-Review (작성자 점검)

**스펙 커버리지:**
- §4 parity(sellValueMultiplier/비용 P1 재사용) → Task 1,2 ✅
- §5① 자동 버프 유지(MaintainBuffsIfEnabled + ScheduleRespawn 후크) → Task 4 ✅
- §5② 효율 업그레이드(UpgradeSellValue/매각가 곱 수동·자동) → Task 2,4 ✅
- §6 세이브 SaveVer 29(3곳) + 해금 AutoConsumable 환생1 + HUD → Task 3,5,6 ✅
- §7 가드(보유0/활성 무동작·골드부족 거부·곱 [1,∞)·구버전 OFF) → Task 2,4 ✅
- §8 테스트/게이트(parity·회귀·SaveVer29·jumbo·biome) → Task 1~7 ✅

**P4 제외:** HP 포션/버프 마스크/자동 강화/다중 효율 트랙 → 미포함(의도적) ✅

**플레이스홀더:** "주:" 는 기존 API명(BuffService UseConsumable 오버로드, GameInstance 테스트 초기화/골드 헬퍼) 확인 안내(실코드 제공). TODO/TBD 없음.

**타입 일관성:** `sellValueMultiplier`↔`GetSellValueMultiplier`/`SellUpgradeNextCost`/`UpgradeSellValue`/`GetSellValueUpgradeCost`/`MaintainBuffsIfEnabled`/`RestoreConsumablePolicy`/`bAutomationAutoMaintainBuff`·`AutomationSellValueUpgradeLevel` 정합. 로컬 `UpgradeCost`/`GoldGained`/`BaseGold`/`BaseSell`(멤버 셰도잉 회피).

**SaveVer 29 3곳 + C4458:** Task 3(헤더+writer)+Task 5(테스트)+Task 7 점검. 로컬 변수명 멤버 회피 명시.
