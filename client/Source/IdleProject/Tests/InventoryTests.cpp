#include "Misc/AutomationTest.h"

#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/DropFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/ItemTypes.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeTestItem(FName ItemId, EItemSlot Slot, EItemRarity Rarity, float Atk, float Def, float Hp, int32 EnhanceLevel = 0)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = Atk;
	Item.BonusDef = Def;
	Item.BonusHp = Hp;
	Item.EnhanceLevel = EnhanceLevel;
	return Item;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRarityMultiplierTest,
	"IdleProject.Inventory.DropFormula.RarityMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRarityMultiplierTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("None rarity has no stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::None), 0.0f);
	TestEqual(TEXT("Common stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Common), 1.0f);
	TestEqual(TEXT("Uncommon stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Uncommon), 1.3f);
	TestEqual(TEXT("Rare stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Rare), 1.7f);
	TestEqual(TEXT("Epic stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Epic), 2.3f);
	TestEqual(TEXT("Legendary stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Legendary), 3.2f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaLevelRarityTrendTest,
	"IdleProject.Inventory.DropFormula.LevelRarityTrend",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaLevelRarityTrendTest::RunTest(const FString& Parameters)
{
	constexpr int32 SampleCount = 2000;
	int32 LowRareOrBetter = 0;
	int32 HighRareOrBetter = 0;
	int32 HighEpicOrBetter = 0;

	FRandomStream LowLevelRng(3601);
	FRandomStream HighLevelRng(3601);
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		if (FDropFormula::RollRarityForLevel(1, LowLevelRng) >= EItemRarity::Rare)
		{
			++LowRareOrBetter;
		}

		const EItemRarity HighRarity = FDropFormula::RollRarityForLevel(100, HighLevelRng);
		if (HighRarity >= EItemRarity::Rare)
		{
			++HighRareOrBetter;
		}
		if (HighRarity >= EItemRarity::Epic)
		{
			++HighEpicOrBetter;
		}
	}

	TestTrue(TEXT("High-level drops roll Rare+ more often than low-level drops"), HighRareOrBetter > LowRareOrBetter);
	TestTrue(TEXT("High-level drops can roll Epic+"), HighEpicOrBetter > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaComputeItemBonusTest,
	"IdleProject.Inventory.DropFormula.ComputeItemBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaComputeItemBonusTest::RunTest(const FString& Parameters)
{
	const FItemInstance LegendaryWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 10, EItemRarity::Legendary, 1.5f);
	TestEqual(TEXT("Legendary weapon attack scales by level variance and rarity"), LegendaryWeapon.BonusAtk, 48.0f);
	TestEqual(TEXT("Legendary weapon has no defense bonus"), LegendaryWeapon.BonusDef, 0.0f);
	TestEqual(TEXT("Legendary weapon has no HP bonus"), LegendaryWeapon.BonusHp, 0.0f);

	const FItemInstance EpicAccessory = FDropFormula::ComputeItemBonus(EItemSlot::Accessory, 10, EItemRarity::Epic, 1.0f);
	TestEqual(TEXT("Epic accessory attack split"), EpicAccessory.BonusAtk, 11.5f);
	TestEqual(TEXT("Epic accessory defense split"), EpicAccessory.BonusDef, 6.9f);
	TestEqual(TEXT("Epic accessory HP split"), EpicAccessory.BonusHp, 46.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceFormulaCurveTest,
	"IdleProject.Inventory.EnhanceFormula.Curve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceFormulaCurveTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Max enhance level matches item clamp"), FEnhanceFormula::MaxEnhanceLevel, 5);
	TestEqual(TEXT("Level 0 cost"), FEnhanceFormula::GetEnhanceCost(0), static_cast<int64>(100));
	TestEqual(TEXT("Level 1 cost"), FEnhanceFormula::GetEnhanceCost(1), static_cast<int64>(400));
	TestEqual(TEXT("Negative level uses level 0 cost"), FEnhanceFormula::GetEnhanceCost(-1), static_cast<int64>(100));
	TestEqual(TEXT("Max level has no next enhance cost"), FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel), static_cast<int64>(0));

	TestEqual(TEXT("Level 0 success rate"), FEnhanceFormula::GetEnhanceSuccessRate(0), 0.95f);
	TestEqual(TEXT("Level 4 success rate"), FEnhanceFormula::GetEnhanceSuccessRate(4), 0.40f);
	TestEqual(TEXT("Max level success rate"), FEnhanceFormula::GetEnhanceSuccessRate(FEnhanceFormula::MaxEnhanceLevel), 0.0f);

	FRandomStream AlwaysSucceeds(123);
	TestTrue(TEXT("Success rate 1 always succeeds"), FEnhanceFormula::RollEnhanceSuccess(1.0f, AlwaysSucceeds));

	FRandomStream AlwaysFails(123);
	TestFalse(TEXT("Success rate 0 always fails"), FEnhanceFormula::RollEnhanceSuccess(0.0f, AlwaysFails));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemPowerScoreDeterministicTest,
	"IdleProject.Inventory.ItemPowerScore.Deterministic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemPowerScoreDeterministicTest::RunTest(const FString& Parameters)
{
	const FItemInstance Sword = MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f, 2);

	TestEqual(TEXT("같은 아이템 점수 1"), FItemPowerScore::Compute(Sword), 12);
	TestEqual(TEXT("같은 아이템 점수 2"), FItemPowerScore::Compute(Sword), 12);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryAutoEquipBetterWeaponTest,
	"IdleProject.Inventory.AutoEquip.BetterWeapon",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryAutoEquipBetterWeaponTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 5.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("uncommon_sword"), EItemSlot::Weapon, EItemRarity::Uncommon, 8.0f, 0.0f, 0.0f));

	const FItemInstance* Equipped = Inventory->GetEquippedItem(EItemSlot::Weapon);
	TestNotNull(TEXT("무기 자동 장착"), Equipped);
	if (Equipped)
	{
		TestEqual(TEXT("더 강한 무기로 교체"), Equipped->ItemId, FName(TEXT("uncommon_sword")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEnhanceEquippedItemTest,
	"IdleProject.Inventory.EnhanceEquippedItem.SuccessAndMaxGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEnhanceEquippedItemTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0));

	TestEqual(TEXT("Initial enhance level"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 0);
	TestTrue(TEXT("Enhance equipped weapon succeeds below max"), Inventory->EnhanceEquippedItem(EItemSlot::Weapon));
	TestEqual(TEXT("Enhance level increments"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 1);

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Enhanced item increases equipment attack bonus"), Bonus.PhysAtk, 11.0f);

	for (int32 Attempt = 1; Attempt < FEnhanceFormula::MaxEnhanceLevel; ++Attempt)
	{
		Inventory->EnhanceEquippedItem(EItemSlot::Weapon);
	}

	TestEqual(TEXT("Enhance level reaches max"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), FEnhanceFormula::MaxEnhanceLevel);
	TestFalse(TEXT("Enhance fails at max level"), Inventory->EnhanceEquippedItem(EItemSlot::Weapon));
	TestFalse(TEXT("Enhance fails when slot is empty"), Inventory->EnhanceEquippedItem(EItemSlot::Helmet));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhancePanelViewModelStateTest,
	"IdleProject.UI.HUD.EnhancePanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhancePanelViewModelStateTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0));

	const FIdleHUDEnhancePanelViewModel NoGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		0,
		FText::GetEmpty(),
		false);
	TestEqual(TEXT("Enhance panel exposes all equipment slots"), NoGold.Rows.Num(), 8);
	TestEqual(TEXT("Weapon row uses current enhance level"), NoGold.Rows[0].EnhanceLevel, 0);
	TestEqual(TEXT("Weapon row shows level zero cost"), NoGold.Rows[0].Cost, static_cast<int64>(100));
	TestFalse(TEXT("Weapon row is disabled without gold"), NoGold.Rows[0].bCanEnhance);
	TestFalse(TEXT("Weapon row reports insufficient gold"), NoGold.Rows[0].bGoldEnough);
	TestFalse(TEXT("Empty helmet row is not enhanceable"), NoGold.Rows[1].bCanEnhance);
	TestFalse(TEXT("Empty helmet row reports no equipment"), NoGold.Rows[1].bEquipped);

	const FIdleHUDEnhancePanelViewModel EnoughGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		FEnhanceFormula::GetEnhanceCost(0),
		FText::GetEmpty(),
		false);
	TestTrue(TEXT("Weapon row enables when equipped and gold is enough"), EnoughGold.Rows[0].bCanEnhance);

	UInventoryComponent* MaxInventory = NewObject<UInventoryComponent>();
	MaxInventory->AddItem(MakeTestItem(TEXT("max_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, FEnhanceFormula::MaxEnhanceLevel));
	const FIdleHUDEnhancePanelViewModel MaxLevel = IdleProject::UI::BuildEnhancePanelViewModel(
		*MaxInventory,
		100000,
		FText::GetEmpty(),
		false);
	TestTrue(TEXT("Max level row reports max state"), MaxLevel.Rows[0].bMaxLevel);
	TestFalse(TEXT("Max level row is disabled"), MaxLevel.Rows[0].bCanEnhance);
	TestEqual(TEXT("Max level row has zero next cost"), MaxLevel.Rows[0].Cost, static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusTwoSlotsTest,
	"IdleProject.Inventory.Bonus.TwoSlots",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusTwoSlotsTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 12.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("common_helmet"), EItemSlot::Helmet, EItemRarity::Common, 0.0f, 4.0f, 20.0f));

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("무기 공격력 보너스"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("투구 방어력 보너스"), Bonus.PhysDef, 4.0f);
	TestEqual(TEXT("투구 체력 보너스"), Bonus.Hp, 20.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryUnequipReducesBonusTest,
	"IdleProject.Inventory.Bonus.Unequip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryUnequipReducesBonusTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 12.0f, 0.0f, 0.0f));
	Inventory->AddItem(MakeTestItem(TEXT("common_helmet"), EItemSlot::Helmet, EItemRarity::Common, 0.0f, 4.0f, 20.0f));

	Inventory->UnequipSlot(EItemSlot::Helmet);
	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("무기 공격력 유지"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("투구 방어력 제거"), Bonus.PhysDef, 0.0f);
	TestEqual(TEXT("투구 체력 제거"), Bonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryRandomDropRangeTest,
	"IdleProject.Inventory.ItemFactory.RandomDropRange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryRandomDropRangeTest::RunTest(const FString& Parameters)
{
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FItemInstance Drop = FItemFactory::RandomDropFromMonster(1);
		TestTrue(TEXT("등급 범위"), Drop.Rarity >= EItemRarity::None && Drop.Rarity <= EItemRarity::Legendary);

		if (Drop.Rarity == EItemRarity::None)
		{
			TestEqual(TEXT("드롭 없음 슬롯"), static_cast<int32>(Drop.Slot), static_cast<int32>(EItemSlot::None));
			continue;
		}

		TestTrue(TEXT("슬롯 범위"), Drop.Slot >= EItemSlot::Weapon && Drop.Slot <= EItemSlot::Accessory);
		TestTrue(TEXT("보너스 중 하나 이상"), Drop.BonusAtk > 0.0f || Drop.BonusDef > 0.0f || Drop.BonusHp > 0.0f);
		TestEqual(TEXT("드롭 강화 기본값"), Drop.EnhanceLevel, 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryHighLevelDropIncludesExpandedRarityTest,
	"IdleProject.Inventory.ItemFactory.HighLevelExpandedRarity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryHighLevelDropIncludesExpandedRarityTest::RunTest(const FString& Parameters)
{
	bool bFoundEpicOrLegendary = false;
	for (int32 Index = 0; Index < 1000; ++Index)
	{
		const FItemInstance Drop = FItemFactory::RandomDropFromMonster(100);
		if (Drop.Rarity >= EItemRarity::Epic)
		{
			bFoundEpicOrLegendary = true;
			TestTrue(TEXT("Epic+ drop carries scaled stats"), FItemPowerScore::Compute(Drop) > 0);
			break;
		}
	}

	TestTrue(TEXT("High-level monster drops can produce Epic or Legendary items"), bFoundEpicOrLegendary);

	return true;
}

#endif
