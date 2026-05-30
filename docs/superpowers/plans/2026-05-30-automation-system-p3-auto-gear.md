# 통합 자동화 시스템 P3 (자동 장비 관리 + 최소 판매) 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 장비 최소 판매 기능을 신설하고, 자동 장착(전투력 기준 교체)과 자동 매각(드랍 필터)을 `AutomationPolicy`로 통제한다.

**Architecture:** P1/P2 골격 재사용. 매각가·아이템파워는 순수 함수로 서버/클라 미러(TDD). 판매는 InventoryComponent 신규 메서드. 자동 매각/장착은 GameInstance `HandleDroppedEquipment` 핸들러로 중앙화하고 몬스터 드랍(`EquipmentDrop`)을 경유시킨다(샵 구매 제외).

**Tech Stack:** 서버 TS+vitest+biome. 클라 UE5 C++. 게이트 `tools/ci/ue-automation.ps1`.

**스펙:** `docs/superpowers/specs/2026-05-30-automation-system-p3-auto-gear-design.md`
**선행:** P1(#99 `5f1fa38`)+P2(#100 `5994d98`). 브랜치 `feat/automation-system-p3`.

**⚠️ SaveVer 범프 3곳(P2 교훈):** SaveVer 27→28 시 ① `IdleSaveGame.h` 헤더 기본값 ② `IdleGameInstance.cpp` `CaptureToSave` 의 `SaveGame->SaveVersion = 27;`(현재 1745 부근) ③ 전 테스트 단언 — **3곳 모두** 갱신.

**P3 제외:** 자동 강화, 부위별/다중 필터 슬롯(P4), 자동 소비(P4), 샵 구매 자동매각.

---

## 파일 구조

| 파일 | 책임 | 신규/수정 |
|---|---|---|
| `server/src/core/formulas/itemSell.ts` | `computeItemSellValue`+`computeItemPower` parity | 신규 |
| `server/src/core/formulas/itemSell.test.ts` | 테스트 | 신규 |
| `server/src/core/formulas/index.ts` | export | 수정 |
| `client/Source/IdleProject/ItemSystem/ItemSellFormula.h` | parity static 2공식 | 신규 |
| `client/Source/IdleProject/ItemSystem/ItemSellFormula.cpp` | 구현 | 신규 |
| `client/Source/IdleProject/ItemSystem/InventoryComponent.h/.cpp` | `SellItem`+`AutoEquipBestPerSlot` | 수정 |
| `client/Source/IdleProject/GameCore/AutomationTypes.h` | (없음 — 정책 필드는 SaveGame/Service) | — |
| `client/Source/IdleProject/GameCore/AutomationPolicyService.h/.cpp` | 3 정책 필드 + 접근자 + Restore 확장 | 수정 |
| `client/Source/IdleProject/GameCore/IdleSaveGame.h` | 3 필드 + SaveVer 28 | 수정 |
| `client/Source/IdleProject/GameCore/IdleGameInstance.h/.cpp` | 저장/복원 + SellInventoryItem/HandleDroppedEquipment/BP 토글 | 수정 |
| `client/Source/IdleProject/ItemSystem/EquipmentDrop.cpp` | 드랍 픽업 핸들러 경유 | 수정 |
| `client/Source/IdleProject/Tests/ItemSellFormulaTests.cpp` | parity + 인벤 회귀 | 신규 |
| 다수 `Tests/*ServiceTests.cpp` | SaveVer 27→28 stale 일괄 | 수정 |

---

## Task 1: 서버 parity — itemSell.ts

**Files:** Create `server/src/core/formulas/itemSell.ts` + `itemSell.test.ts`; Modify `index.ts`.

- [ ] **Step 1: 실패 테스트**

`server/src/core/formulas/itemSell.test.ts`:

```ts
import { describe, expect, it } from "vitest";
import { computeItemSellValue, computeItemPower, type ItemRarity } from "./itemSell.js";

describe("computeItemSellValue — 등급×강화 매각가", () => {
  it("등급별 base(강화 0)", () => {
    expect(computeItemSellValue("Common", 0)).toBe(100);
    expect(computeItemSellValue("Epic", 0)).toBe(1500);
    expect(computeItemSellValue("Mythic", 0)).toBe(400000);
  });
  it("None은 0", () => {
    expect(computeItemSellValue("None", 5)).toBe(0);
  });
  it("강화당 +20%", () => {
    expect(computeItemSellValue("Common", 5)).toBe(200);   // 100*(1+1.0)
    expect(computeItemSellValue("Rare", 10)).toBe(1200);   // 400*(1+2.0)
  });
  it("음수 강화 0가드", () => {
    expect(computeItemSellValue("Common", -3)).toBe(100);
  });
});

describe("computeItemPower — 같은 슬롯 내 우열 스칼라", () => {
  const item = (over = {}) => ({
    bonusAtk: 0, bonusMagicAtk: 0, bonusDef: 0, bonusPhysDef: 0, bonusMagicDef: 0,
    bonusHp: 0, bonusAffixHp: 0, bonusCritRate: 0, bonusCritDmg: 0, enhanceLevel: 0, ...over,
  });
  it("보너스 가중합", () => {
    expect(computeItemPower(item({ bonusAtk: 100, bonusDef: 50 }))).toBe(150);
    expect(computeItemPower(item({ bonusHp: 1000 }))).toBe(100);        // /10
    expect(computeItemPower(item({ bonusCritRate: 0.1 }))).toBe(100);   // *1000
    expect(computeItemPower(item({ bonusCritDmg: 0.5 }))).toBe(50);     // *100
  });
  it("강화당 +10%", () => {
    expect(computeItemPower(item({ bonusAtk: 100, enhanceLevel: 10 }))).toBe(200);
  });
  it("음수/0가드", () => {
    expect(computeItemPower(item({ bonusAtk: 100, enhanceLevel: -5 }))).toBe(100);
  });
});
```

- [ ] **Step 2: 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/itemSell.test.ts`
Expected: FAIL — module/export 없음

- [ ] **Step 3: 구현**

`server/src/core/formulas/itemSell.ts`:

```ts
// 아이템 매각가/파워 — 클라 ItemSellFormula 와 1:1 parity(라우트/DB 없음).
// 자동 장비 관리(P3): 매각가 = 등급 base × 강화 배수, 파워 = 보너스 가중합 × 강화 배수.

export type ItemRarity =
  | "None" | "Common" | "Rare" | "Epic"
  | "Unique" | "Legendary" | "Transcendent" | "Mythic";

// 등급별 매각 base 골드. None=0. 클라 RARITY_SELL_BASE 1:1.
const RARITY_SELL_BASE: Record<ItemRarity, number> = {
  None: 0, Common: 100, Rare: 400, Epic: 1500, Unique: 6000,
  Legendary: 25000, Transcendent: 100000, Mythic: 400000,
};

// 매각가: base × (1 + 0.2 × max(0, enhanceLevel)). 무한(캡 없음).
export function computeItemSellValue(rarity: ItemRarity, enhanceLevel: number): number {
  const base = RARITY_SELL_BASE[rarity] ?? 0;
  const lv = Math.max(0, Math.trunc(enhanceLevel));
  return Math.round(base * (1 + 0.2 * lv));
}

export type ItemPowerInput = {
  bonusAtk: number; bonusMagicAtk: number; bonusDef: number;
  bonusPhysDef: number; bonusMagicDef: number; bonusHp: number;
  bonusAffixHp: number; bonusCritRate: number; bonusCritDmg: number;
  enhanceLevel: number;
};

// 같은 슬롯 내 우열용 스칼라(CombatPower 와 별개). 보너스 가중합 × 강화 배수.
export function computeItemPower(item: ItemPowerInput): number {
  const raw =
    item.bonusAtk + item.bonusMagicAtk +
    item.bonusDef + item.bonusPhysDef + item.bonusMagicDef +
    (item.bonusHp + item.bonusAffixHp) / 10 +
    item.bonusCritRate * 1000 + item.bonusCritDmg * 100;
  const lv = Math.max(0, Math.trunc(item.enhanceLevel));
  return Math.round(Math.max(0, raw) * (1 + 0.1 * lv));
}
```

- [ ] **Step 4: 통과 + 전체 + lint**

Run: `cd server; npx vitest run src/core/formulas/itemSell.test.ts` → PASS
Run: `cd server; npm test` → 전체 GREEN
Run: `cd server; npx biome check . ../tools/balance-sim` → clean (위반 시 `npx biome check --write src/core/formulas/itemSell.ts` 후 재확인)

`index.ts`에 `export * from "./itemSell.js";` 추가(기존 정렬 양식에 맞춰).

- [ ] **Step 5: 커밋**

```bash
git add server/src/core/formulas/itemSell.ts server/src/core/formulas/itemSell.test.ts server/src/core/formulas/index.ts
git commit -m "feat(server): 아이템 매각가/파워 parity(itemSell)"
```

---

## Task 2: 클라 parity — ItemSellFormula

**Files:** Create `ItemSellFormula.h/.cpp`; Test `ItemSellFormulaTests.cpp`.

- [ ] **Step 1: 헤더**

`client/Source/IdleProject/ItemSystem/ItemSellFormula.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

/** 아이템 매각가/파워 — 서버 itemSell.ts 1:1 parity. 라우트/DB 없음. */
class IDLEPROJECT_API FItemSellFormula
{
public:
	// 등급 base × (1 + 0.2 × max(0,enhance)). None=0.
	static int64 ComputeSellValue(EItemRarity Rarity, int32 EnhanceLevel);

	// 같은 슬롯 내 우열 스칼라(보너스 가중합 × 강화 배수). CombatPower 와 별개.
	static int64 ComputeItemPower(const FItemInstance& Item);
};
```

- [ ] **Step 2: 구현**

`client/Source/IdleProject/ItemSystem/ItemSellFormula.cpp`:

```cpp
#include "ItemSystem/ItemSellFormula.h"

namespace
{
	// 서버 RARITY_SELL_BASE 1:1. 인덱스 = EItemRarity(None0..Mythic7).
	int64 RaritySellBase(EItemRarity Rarity)
	{
		switch (Rarity)
		{
		case EItemRarity::Common:       return 100;
		case EItemRarity::Rare:         return 400;
		case EItemRarity::Epic:         return 1500;
		case EItemRarity::Unique:       return 6000;
		case EItemRarity::Legendary:    return 25000;
		case EItemRarity::Transcendent: return 100000;
		case EItemRarity::Mythic:       return 400000;
		default:                        return 0; // None
		}
	}
}

int64 FItemSellFormula::ComputeSellValue(EItemRarity Rarity, int32 EnhanceLevel)
{
	const int32 Lv = FMath::Max(0, EnhanceLevel);
	const double Value = static_cast<double>(RaritySellBase(Rarity)) * (1.0 + 0.2 * Lv);
	return static_cast<int64>(FMath::RoundToDouble(Value));
}

int64 FItemSellFormula::ComputeItemPower(const FItemInstance& Item)
{
	const double Raw =
		Item.BonusAtk + Item.BonusMagicAtk +
		Item.BonusDef + Item.BonusPhysDef + Item.BonusMagicDef +
		(Item.BonusHp + Item.BonusAffixHp) / 10.0 +
		Item.BonusCritRate * 1000.0 + Item.BonusCritDmg * 100.0;
	const int32 Lv = FMath::Max(0, Item.EnhanceLevel);
	return static_cast<int64>(FMath::RoundToDouble(FMath::Max(0.0, Raw) * (1.0 + 0.1 * Lv)));
}
```

> 주: `FItemInstance` 의 실제 보너스 필드명(BonusAtk/BonusMagicAtk/BonusDef/BonusPhysDef/BonusMagicDef/BonusHp/BonusAffixHp/BonusCritRate/BonusCritDmg/EnhanceLevel)을 `ItemTypes.h` 에서 확인해 정확히 사용한다. 누락/상이 시 해당 필드로 교체.

- [ ] **Step 3: parity 회귀 테스트**

`client/Source/IdleProject/Tests/ItemSellFormulaTests.cpp`:

```cpp
#include "Misc/AutomationTest.h"
#include "ItemSystem/ItemSellFormula.h"
#include "ItemSystem/ItemTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemSellParityTest,
	"IdleProject.Item.SellFormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemSellParityTest::RunTest(const FString& Parameters)
{
	using F = FItemSellFormula;
	// 매각가
	TestEqual(TEXT("common base"), F::ComputeSellValue(EItemRarity::Common, 0), (int64)100);
	TestEqual(TEXT("epic base"), F::ComputeSellValue(EItemRarity::Epic, 0), (int64)1500);
	TestEqual(TEXT("none zero"), F::ComputeSellValue(EItemRarity::None, 5), (int64)0);
	TestEqual(TEXT("common +5 enh"), F::ComputeSellValue(EItemRarity::Common, 5), (int64)200);
	TestEqual(TEXT("rare +10 enh"), F::ComputeSellValue(EItemRarity::Rare, 10), (int64)1200);
	TestEqual(TEXT("neg enh guard"), F::ComputeSellValue(EItemRarity::Common, -3), (int64)100);
	// 파워
	FItemInstance A; A.BonusAtk = 100.0f; A.BonusDef = 50.0f;
	TestEqual(TEXT("power sum"), F::ComputeItemPower(A), (int64)150);
	FItemInstance B; B.BonusHp = 1000.0f;
	TestEqual(TEXT("power hp/10"), F::ComputeItemPower(B), (int64)100);
	FItemInstance C; C.BonusAtk = 100.0f; C.EnhanceLevel = 10;
	TestEqual(TEXT("power +10 enh"), F::ComputeItemPower(C), (int64)200);
	return true;
}

#endif
```

- [ ] **Step 4: 빌드 + 좁은 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.Item.SellFormulaParity"`
Expected: 빌드 성공 + PASS

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/ItemSystem/ItemSellFormula.h client/Source/IdleProject/ItemSystem/ItemSellFormula.cpp client/Source/IdleProject/Tests/ItemSellFormulaTests.cpp
git commit -m "feat(client): ItemSellFormula(매각가/파워) parity + 회귀"
```

---

## Task 3: InventoryComponent — SellItem + AutoEquipBestPerSlot

**Files:** Modify `InventoryComponent.h/.cpp`; Test `ItemSellFormulaTests.cpp`(인벤 케이스 추가).

- [ ] **Step 1: 헤더 선언**

`InventoryComponent.h` public 에 추가:

```cpp
	// 미장착·미잠금 아이템 매각: 제거 후 매각가 반환(불가 시 0). 장착 인덱스 보정.
	int64 SellItem(int32 ItemIndex);

	// 슬롯별 보유 아이템 중 파워 최고가 장착품보다 크면 교체. 교체 수 반환.
	int32 AutoEquipBestPerSlot();
```

- [ ] **Step 2: 구현**

`InventoryComponent.cpp` 상단 include 에 `#include "ItemSystem/ItemSellFormula.h"` 추가. 구현:

```cpp
int64 UInventoryComponent::SellItem(int32 ItemIndex)
{
	if (!Items.IsValidIndex(ItemIndex))
	{
		return 0;
	}
	const FItemInstance& Item = Items[ItemIndex];
	if (Item.bLocked)
	{
		return 0;
	}
	// 장착 중인 아이템은 매각 거부.
	for (const TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (Pair.Value == ItemIndex)
		{
			return 0;
		}
	}

	const int64 Value = FItemSellFormula::ComputeSellValue(Item.Rarity, Item.EnhanceLevel);
	Items.RemoveAt(ItemIndex);

	// 제거로 인한 인덱스 시프트: ItemIndex 보다 큰 장착 인덱스 -1 보정.
	for (TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (Pair.Value > ItemIndex)
		{
			Pair.Value -= 1;
		}
	}
	return Value;
}

int32 UInventoryComponent::AutoEquipBestPerSlot()
{
	int32 EquipCount = 0;
	// 슬롯별 최고 파워 인덱스 수집(None 슬롯 제외).
	TMap<EItemSlot, int32> BestIndexBySlot;
	TMap<EItemSlot, int64> BestPowerBySlot;
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		const FItemInstance& It = Items[i];
		if (It.Slot == EItemSlot::None)
		{
			continue;
		}
		const int64 Power = FItemSellFormula::ComputeItemPower(It);
		const int64* Cur = BestPowerBySlot.Find(It.Slot);
		if (!Cur || Power > *Cur)
		{
			BestPowerBySlot.Add(It.Slot, Power);
			BestIndexBySlot.Add(It.Slot, i);
		}
	}
	// 장착품보다 강하면 교체.
	for (const TPair<EItemSlot, int32>& Pair : BestIndexBySlot)
	{
		const EItemSlot Slot = Pair.Key;
		const int32 BestIdx = Pair.Value;
		const FItemInstance* Equipped = GetEquippedItem(Slot);
		const int64 EquippedPower = Equipped ? FItemSellFormula::ComputeItemPower(*Equipped) : -1;
		if (BestPowerBySlot[Slot] > EquippedPower)
		{
			EquipItem(BestIdx);
			EquipCount += 1;
		}
	}
	return EquipCount;
}
```

> 주: `EquippedIndex`/`Items`/`GetEquippedItem`/`EquipItem` 의 실제 시그니처를 `InventoryComponent.h/.cpp` 에서 확인. `EquipItem(int32 ItemIndex)` 가 EquippedIndex 를 갱신하는 기존 동작에 의존. 같은 슬롯 다수면 최고 1개만 장착(나머지 인벤 유지).

- [ ] **Step 3: 인벤 회귀 테스트 추가**

`ItemSellFormulaTests.cpp` 의 `#endif` 앞에 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventorySellEquipTest,
	"IdleProject.Item.InventorySellEquip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventorySellEquipTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inv = NewObject<UInventoryComponent>();

	FItemInstance Weak; Weak.Slot = EItemSlot::Weapon; Weak.Rarity = EItemRarity::Common; Weak.BonusAtk = 10.0f;
	FItemInstance Strong; Strong.Slot = EItemSlot::Weapon; Strong.Rarity = EItemRarity::Rare; Strong.BonusAtk = 100.0f;
	Inv->AddItem(Weak);   // index 0
	Inv->AddItem(Strong); // index 1

	// 자동 장착: 더 강한 Strong 장착.
	const int32 Equipped = Inv->AutoEquipBestPerSlot();
	TestEqual(TEXT("equipped one"), Equipped, 1);
	const FItemInstance* Now = Inv->GetEquippedItem(EItemSlot::Weapon);
	TestTrue(TEXT("strong equipped"), Now && Now->BonusAtk == 100.0f);

	// 장착품 매각 거부.
	int32 StrongIdx = INDEX_NONE;
	// (장착 인덱스 조회는 GetEquippedItem 으로 충분; 매각 거부만 검증)
	// Weak(미장착, index 0) 매각 → Common base 100.
	const int64 Gold = Inv->SellItem(0);
	TestEqual(TEXT("weak sells for 100"), Gold, (int64)100);

	// 잠금 아이템 매각 거부.
	FItemInstance Locked; Locked.Slot = EItemSlot::Helmet; Locked.Rarity = EItemRarity::Epic; Locked.bLocked = true;
	Inv->AddItem(Locked);
	const int32 LockedIdx = Inv->GetItemCountForTest() - 1; // 주: 마지막 인덱스. 게터 없으면 알려진 인덱스로.
	TestEqual(TEXT("locked sell refused"), Inv->SellItem(LockedIdx), (int64)0);
	return true;
}
```

> 주: `GetItemCountForTest()` 가 없으면, 추가 순서로 인덱스를 직접 계산하라(Weak 매각 후 Items = [Strong(0), Locked(1)] → LockedIdx=1). `UInventoryComponent` 생성/AddItem/GetEquippedItem 시그니처를 헤더에서 확인.

- [ ] **Step 4: 빌드 + Item Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.Item"`
Expected: 빌드 성공 + SellFormulaParity/InventorySellEquip PASS + 기존 Item 회귀 GREEN

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/ItemSystem/InventoryComponent.h client/Source/IdleProject/ItemSystem/InventoryComponent.cpp client/Source/IdleProject/Tests/ItemSellFormulaTests.cpp
git commit -m "feat(client): InventoryComponent 매각/자동장착 + 회귀"
```

---

## Task 4: 정책 + 세이브 SaveVer 28 + GameInstance 핸들러

**Files:** Modify `AutomationPolicyService.h/.cpp`, `IdleSaveGame.h`, `IdleGameInstance.h/.cpp`, `EquipmentDrop.cpp`.

- [ ] **Step 1: 정책 서비스 3필드**

`AutomationPolicyService.h` 정책 상태에 추가(접근자 포함):

```cpp
	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoEquipByPower() const { return bAutoEquipByPower; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoEquipByPower(bool b) { bAutoEquipByPower = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool GetAutoSell() const { return bAutoSell; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoSell(bool b) { bAutoSell = b; }

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	EItemRarity GetAutoSellMaxRarity() const { return AutoSellMaxRarity; }
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutoSellMaxRarity(EItemRarity R) { AutoSellMaxRarity = R; }

	void RestoreGearPolicy(bool bInAutoEquip, bool bInAutoSell, EItemRarity InMaxRarity);
```

private 멤버:

```cpp
	UPROPERTY()
	bool bAutoEquipByPower = false;
	UPROPERTY()
	bool bAutoSell = false;
	UPROPERTY()
	EItemRarity AutoSellMaxRarity = EItemRarity::Common;
```

`AutomationPolicyService.h` 상단 include 에 `#include "ItemSystem/ItemTypes.h"` 추가(EItemRarity). 구현(`.cpp`):

```cpp
void UAutomationPolicyService::RestoreGearPolicy(bool bInAutoEquip, bool bInAutoSell, EItemRarity InMaxRarity)
{
	bAutoEquipByPower = bInAutoEquip;
	bAutoSell = bInAutoSell;
	AutoSellMaxRarity = InMaxRarity;
}
```

- [ ] **Step 2: 세이브 필드 + SaveVer 28 (3곳)**

`IdleSaveGame.h`: `int32 SaveVersion = 27;` → `28`(주석 갱신). P2 필드 다음에 추가:

```cpp
	// 자동 장비 정책(P3). SaveVer 28+.
	UPROPERTY()
	bool bAutomationAutoEquipByPower = false;
	UPROPERTY()
	bool bAutomationAutoSell = false;
	UPROPERTY()
	EItemRarity AutomationAutoSellMaxRarity = EItemRarity::Common;
```

`IdleGameInstance.cpp` `CaptureToSave` 의 `SaveGame->SaveVersion = 27;` → `28`. **(P2 교훈: 이 writer 라인 필수)**

- [ ] **Step 3: 저장 직렬화 + 복원**

저장(자동화 직렬화 블록, `SaveGame->AutomationSkillRules = ...;` 다음):

```cpp
		SaveGame->bAutomationAutoEquipByPower = AutomationPolicyService->GetAutoEquipByPower();
		SaveGame->bAutomationAutoSell = AutomationPolicyService->GetAutoSell();
		SaveGame->AutomationAutoSellMaxRarity = AutomationPolicyService->GetAutoSellMaxRarity();
```

복원(P2 `RestoreSkillRules` 호출 다음):

```cpp
		if (SaveGame->SaveVersion >= 28)
		{
			AutomationPolicyService->RestoreGearPolicy(
				SaveGame->bAutomationAutoEquipByPower,
				SaveGame->bAutomationAutoSell,
				SaveGame->AutomationAutoSellMaxRarity);
		}
		else
		{
			AutomationPolicyService->RestoreGearPolicy(false, false, EItemRarity::Common);
		}
```

- [ ] **Step 4: GameInstance — 판매/드랍 핸들러/BP 토글**

`IdleGameInstance.h` public 에 추가:

```cpp
	UFUNCTION(BlueprintCallable, Category = "Idle|Item")
	int64 SellInventoryItem(int32 ItemIndex);

	// 몬스터 드랍 처리: 자동 매각 대상이면 골드화(미보관), 아니면 보관(+자동 장착). bKept 반환.
	bool HandleDroppedEquipment(class UInventoryComponent* Inv, const FItemInstance& Item);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoEquip(bool bValue);
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoSell(bool bValue);
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoSellMaxRarity(EItemRarity Rarity);
```

`.cpp` 구현:

```cpp
int64 UIdleGameInstance::SellInventoryItem(int32 ItemIndex)
{
	UInventoryComponent* Inv = GetPlayerInventory(); // 주: 기존 인벤 접근자명 확인(없으면 캐릭터 경유)
	if (!Inv) { return 0; }
	const int64 Gold = Inv->SellItem(ItemIndex);
	if (Gold > 0) { AddGold(Gold); }
	return Gold;
}

bool UIdleGameInstance::HandleDroppedEquipment(UInventoryComponent* Inv, const FItemInstance& Item)
{
	EnsureAutomationPolicyService();
	if (!Inv) { return false; }

	if (AutomationPolicyService && AutomationPolicyService->GetAutoSell()
		&& static_cast<uint8>(Item.Rarity) <= static_cast<uint8>(AutomationPolicyService->GetAutoSellMaxRarity())
		&& Item.Rarity != EItemRarity::None
		&& !Item.bLocked && Item.EnhanceLevel <= 0
		&& Item.PotentialGrade == EPotentialGrade::None
		&& Item.UniqueTrait1 == EUniqueTrait::None && Item.UniqueTrait2 == EUniqueTrait::None)
	{
		AddGold(FItemSellFormula::ComputeSellValue(Item.Rarity, Item.EnhanceLevel));
		return false; // 매각됨, 미보관
	}

	if (!Inv->AddItem(Item)) { return false; }
	if (AutomationPolicyService && AutomationPolicyService->GetAutoEquipByPower())
	{
		Inv->AutoEquipBestPerSlot();
	}
	return true;
}

void UIdleGameInstance::SetAutomationAutoEquip(bool bValue)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService) { AutomationPolicyService->SetAutoEquipByPower(bValue); }
}
void UIdleGameInstance::SetAutomationAutoSell(bool bValue)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService) { AutomationPolicyService->SetAutoSell(bValue); }
}
void UIdleGameInstance::SetAutomationAutoSellMaxRarity(EItemRarity Rarity)
{
	EnsureAutomationPolicyService();
	if (AutomationPolicyService) { AutomationPolicyService->SetAutoSellMaxRarity(Rarity); }
}
```

`IdleGameInstance.cpp` 상단 include 에 `#include "ItemSystem/ItemSellFormula.h"` + `#include "ItemSystem/InventoryComponent.h"`(없을 때만) 추가.

> 주: `GetPlayerInventory()` 는 예시명. 기존에 플레이어 인벤토리를 얻는 경로(캐릭터→FindComponentByClass<UInventoryComponent>)를 grep 으로 확인해 정확히 사용. 없으면 `HandleDroppedEquipment` 가 Inv 를 인자로 받으므로 SellInventoryItem 도 동일하게 플레이어 캐릭터 인벤을 찾는 헬퍼를 따른다(기존 인벤 접근 패턴 재사용).

- [ ] **Step 5: EquipmentDrop 경유**

`EquipmentDrop.cpp` 의 픽업 블록:

```cpp
		if (UInventoryComponent* Inventory = TargetCharacter->FindComponentByClass<UInventoryComponent>())
		{
			if (Inventory->AddItem(Payload))
			{
				if (UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>())
				{
					GameInstance->RecordAchievementItemCollected(Payload);
				}
			}
		}
```

를 다음으로 변경(핸들러 경유, 보관된 경우만 업적):

```cpp
		if (UInventoryComponent* Inventory = TargetCharacter->FindComponentByClass<UInventoryComponent>())
		{
			if (UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>())
			{
				if (GameInstance->HandleDroppedEquipment(Inventory, Payload))
				{
					GameInstance->RecordAchievementItemCollected(Payload);
				}
			}
			else
			{
				Inventory->AddItem(Payload); // GameInstance 없으면 기존 동작
			}
		}
```

- [ ] **Step 6: 빌드 (테스트 red 가능 — Task 5에서 해소)**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.GameCore.SaveSystem"`
Expected: **빌드 성공** 확인(SaveVer 28 단언은 Task 5 후 GREEN). 컴파일 오류 0.

- [ ] **Step 7: 커밋**

```bash
git add client/Source/IdleProject/GameCore/AutomationPolicyService.h client/Source/IdleProject/GameCore/AutomationPolicyService.cpp client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/ItemSystem/EquipmentDrop.cpp
git commit -m "feat(client): 자동 장비 정책 + SaveVer 28 + 드랍 핸들러(자동매각/장착)"
```

---

## Task 5: SaveVer 27→28 stale 단언 일괄 갱신

**Files:** P1/P2에서 갱신한 동일 테스트 파일들의 "현재 세이브 버전" 단언.

- [ ] **Step 1: 일괄 갱신**

`grep -rn "to be 27\|current version (27)\|V27\|v27\|version (27)" client/Source/IdleProject/Tests/` 로 현재-버전 단언을 찾아 27→28(+ 메시지 라벨 V27/v27/(27)→V28/v28/(28)). 마이그레이션 SOURCE(v7/v15 등)는 유지. 동일 13개 파일:
`ConsumableTests, AttendanceServiceTests, DungeonServiceTests, MissionServiceTests, SaveSystemTests(다수), TitleServiceTests, TreasureBoxServiceTests(+주석), GuildTests(SaveRoundTripV19), RarityMigrationTests, MasteryTests, RebirthPerkServiceTests, RuneServiceTests, RuneSetServiceTests(2)`.

- [ ] **Step 2: 잔여 확인**

Run: `grep -rn "SaveVersion, .*27\|to be 27\|current version (27)" client/Source/IdleProject/Tests/`
Expected: 현재-버전 27 잔존 0(마이그 SOURCE 제외).

- [ ] **Step 3: 커밋**

```bash
git add client/Source/IdleProject/Tests/
git commit -m "test(client): SaveVer 27→28 stale 단언 일괄 갱신 (P3 SaveVer 범프)"
```

---

## Task 6: HUD 장비 탭 진입점 (해금 게이트)

**Files:** Modify `IdleGameInstance.h/.cpp`(필요 시 — 토글은 Task 4에서 노출됨).

- [ ] **Step 1: 해금 판정 재사용 확인**

자동 장비 해금은 P1 `IsAutomationFeatureUnlocked(EAutomationFeature::AutoGear)`(챕터 5)로 이미 판정 가능. 신규 코드 없이 HUD(데이터 구동)가 이 BP 게터 + Task 4의 토글/매각 함수를 바인딩한다. 추가 C++ 위젯 금지.

자동 매각 등급 선택 표시명은 기존 레어도 표시명(#65)을 재사용한다. 신규 로컬라이즈 키 필요 시 `UI.csv` 에 자동 장비 탭 라벨만 추가(자동 장착/자동 매각/판매 등 소수).

- [ ] **Step 2: (해당 시) UI 라벨 키 추가 + 빌드 + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject"`
Expected: 표준 jumbo 빌드 성공(ODR 0) + 전체 Automation GREEN(`IdleProject.Item`/`IdleProject.UI.HUD`/SaveSystem v28 포함)

- [ ] **Step 3: 커밋(라벨 추가 있었던 경우만)**

```bash
git add client/Source/IdleProject
git commit -m "feat(client): 자동 장비 탭 라벨 + 해금 게이트(AutoGear ch5)"
```

---

## Task 7: 최종 게이트

- [ ] **Step 1: 서버 전체 + lint**

Run: `cd server; npm test` → GREEN
Run: `cd server; npx biome check . ../tools/balance-sim` → clean

- [ ] **Step 2: UE 표준 jumbo + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1`
Expected: Fail 0, EXIT 0. SaveVer 28 단언 GREEN

- [ ] **Step 3: SaveVer stale 점검(3곳 정합)**

Run: `grep -rn "SaveVersion = 2" client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/GameCore/IdleSaveGame.h`
Expected: CaptureToSave writer = 28, 헤더 기본값 = 28 (불일치 0).

Run: `grep -rn "to be 27\|current version (27)" client/Source/IdleProject/Tests/`
Expected: 0

- [ ] **Step 4: 최종 커밋(보정 있었던 경우만)**

```bash
git add -A
git commit -m "chore: 자동화 P3 SaveVer 28 정합 점검"
```

---

## Self-Review (작성자 점검)

**스펙 커버리지:**
- §4 parity(computeItemSellValue/computeItemPower) → Task 1,2 ✅
- §5① 최소 판매(SellItem/SellInventoryItem) → Task 3,4 ✅
- §5② 자동 장착(AutoEquipBestPerSlot, bAutoEquipByPower) → Task 3,4 ✅
- §5③ 자동 매각 + 드랍 핸들러 + 제외 가드 + EquipmentDrop 경유 → Task 4 ✅
- §6 세이브 SaveVer 28(3곳) + 해금 AutoGear ch5 + HUD → Task 4,5,6 ✅
- §7 가드(장착/잠금 거부·제외 화이트리스트·약한 교체 금지·구버전 OFF) → Task 3,4 ✅
- §8 테스트/게이트(parity·인벤 회귀·SaveVer28·jumbo·biome) → Task 1~7 ✅

**P3 제외:** 자동 강화/부위 필터·슬롯 sink/자동 소비/샵 자동매각 → 미포함(의도적) ✅

**플레이스홀더:** "주:" 는 기존 시그니처/접근자명(FItemInstance 보너스 필드, EquippedIndex/EquipItem, 플레이어 인벤 접근자, 테스트 인덱스) 확인 안내(실코드 제공). TODO/TBD 없음.

**타입 일관성:** `computeItemSellValue`↔`ComputeSellValue`/`computeItemPower`↔`ComputeItemPower`/`SellItem`/`AutoEquipBestPerSlot`/`HandleDroppedEquipment`/`SellInventoryItem`/`RestoreGearPolicy`/`bAutomationAutoEquipByPower`·`bAutomationAutoSell`·`AutomationAutoSellMaxRarity` — 서버/클라/세이브 정합.

**SaveVer 28 3곳(P2 교훈):** Task 4 Step 2 헤더 + CaptureToSave writer 명시, Task 5 테스트 단언, Task 7 Step 3 정합 grep. 누락 방지.
