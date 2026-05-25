#include "Misc/AutomationTest.h"

#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/ItemTypes.h"

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
		TestTrue(TEXT("등급 범위"), Drop.Rarity >= EItemRarity::None && Drop.Rarity <= EItemRarity::Rare);

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

#endif
