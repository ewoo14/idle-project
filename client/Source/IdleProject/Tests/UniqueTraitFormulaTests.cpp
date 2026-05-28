#include "Misc/AutomationTest.h"

#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/DropFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/ItemTypes.h"
#include "ItemSystem/UniqueTraitFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeTraitTestItem(EItemSlot Slot, EItemRarity Rarity)
{
	FItemInstance Item;
	Item.ItemId = TEXT("trait_test_item");
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromString(TEXT("Trait Test Item"));
	Item.BonusAtk = 10.0f;
	Item.BonusDef = 5.0f;
	Item.BonusHp = 20.0f;
	return Item;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUniqueTraitFormulaValueTest,
	"IdleProject.Inventory.UniqueTrait.Values",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUniqueTraitFormulaValueTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("None trait contributes zero"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::None, EItemRarity::Unique), 0.0f);
	TestEqual(TEXT("AllStatSurge Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::AllStatSurge, EItemRarity::Unique), 0.08f);
	TestEqual(TEXT("CritDamageSurge Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::CritDamageSurge, EItemRarity::Unique), 0.15f);
	TestEqual(TEXT("CritRateSurge Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::CritRateSurge, EItemRarity::Unique), 0.05f);
	TestEqual(TEXT("LifeSurge Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::LifeSurge, EItemRarity::Unique), 0.10f);
	TestEqual(TEXT("SwiftSurge Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::SwiftSurge, EItemRarity::Unique), 0.08f);
	TestEqual(TEXT("PhysMastery Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::PhysMastery, EItemRarity::Unique), 0.12f);
	TestEqual(TEXT("MagicMastery Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::MagicMastery, EItemRarity::Unique), 0.12f);
	TestEqual(TEXT("GuardMastery Unique value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::GuardMastery, EItemRarity::Unique), 0.10f);
	TestEqual(TEXT("Transcendent uses 1.5x value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::AllStatSurge, EItemRarity::Transcendent), 0.12f);
	TestEqual(TEXT("Mythic does not grant unique trait value"), FUniqueTraitFormula::GetTraitValue(EUniqueTrait::AllStatSurge, EItemRarity::Mythic), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUniqueTraitFormulaRollTest,
	"IdleProject.Inventory.UniqueTrait.RollRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUniqueTraitFormulaRollTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Common does not grant a trait"), FUniqueTraitFormula::RarityGrantsUnique(EItemRarity::Common));
	TestTrue(TEXT("Unique grants one trait"), FUniqueTraitFormula::RarityGrantsUnique(EItemRarity::Unique));
	TestTrue(TEXT("Transcendent grants traits"), FUniqueTraitFormula::RarityGrantsUnique(EItemRarity::Transcendent));
	TestFalse(TEXT("Mythic does not grant a trait"), FUniqueTraitFormula::RarityGrantsUnique(EItemRarity::Mythic));
	TestFalse(TEXT("Unique does not grant two traits"), FUniqueTraitFormula::RarityGrantsTwoTraits(EItemRarity::Unique));
	TestTrue(TEXT("Transcendent grants two traits"), FUniqueTraitFormula::RarityGrantsTwoTraits(EItemRarity::Transcendent));

	EUniqueTrait Trait1 = EUniqueTrait::AllStatSurge;
	EUniqueTrait Trait2 = EUniqueTrait::CritRateSurge;
	FRandomStream CommonRng(6701);
	FUniqueTraitFormula::RollUniqueTraits(EItemRarity::Common, CommonRng, Trait1, Trait2);
	TestEqual(TEXT("Common clears trait 1"), Trait1, EUniqueTrait::None);
	TestEqual(TEXT("Common clears trait 2"), Trait2, EUniqueTrait::None);

	FRandomStream UniqueRngA(6702);
	FRandomStream UniqueRngB(6702);
	EUniqueTrait UniqueTraitA1 = EUniqueTrait::None;
	EUniqueTrait UniqueTraitA2 = EUniqueTrait::None;
	EUniqueTrait UniqueTraitB1 = EUniqueTrait::None;
	EUniqueTrait UniqueTraitB2 = EUniqueTrait::None;
	FUniqueTraitFormula::RollUniqueTraits(EItemRarity::Unique, UniqueRngA, UniqueTraitA1, UniqueTraitA2);
	FUniqueTraitFormula::RollUniqueTraits(EItemRarity::Unique, UniqueRngB, UniqueTraitB1, UniqueTraitB2);
	TestTrue(TEXT("Unique rolls trait 1"), UniqueTraitA1 >= EUniqueTrait::AllStatSurge && UniqueTraitA1 <= EUniqueTrait::GuardMastery);
	TestEqual(TEXT("Unique leaves trait 2 empty"), UniqueTraitA2, EUniqueTrait::None);
	TestEqual(TEXT("Unique roll is deterministic for trait 1"), UniqueTraitA1, UniqueTraitB1);
	TestEqual(TEXT("Unique roll is deterministic for trait 2"), UniqueTraitA2, UniqueTraitB2);

	FRandomStream TranscendentRng(6703);
	EUniqueTrait TranscendentTrait1 = EUniqueTrait::None;
	EUniqueTrait TranscendentTrait2 = EUniqueTrait::None;
	FUniqueTraitFormula::RollUniqueTraits(EItemRarity::Transcendent, TranscendentRng, TranscendentTrait1, TranscendentTrait2);
	TestTrue(TEXT("Transcendent rolls trait 1"), TranscendentTrait1 >= EUniqueTrait::AllStatSurge && TranscendentTrait1 <= EUniqueTrait::GuardMastery);
	TestTrue(TEXT("Transcendent rolls trait 2"), TranscendentTrait2 >= EUniqueTrait::AllStatSurge && TranscendentTrait2 <= EUniqueTrait::GuardMastery);
	TestNotEqual(TEXT("Transcendent rolls two distinct traits"), TranscendentTrait1, TranscendentTrait2);

	FRandomStream MythicRng(6704);
	EUniqueTrait MythicTrait1 = EUniqueTrait::AllStatSurge;
	EUniqueTrait MythicTrait2 = EUniqueTrait::CritRateSurge;
	FUniqueTraitFormula::RollUniqueTraits(EItemRarity::Mythic, MythicRng, MythicTrait1, MythicTrait2);
	TestEqual(TEXT("Mythic clears trait 1"), MythicTrait1, EUniqueTrait::None);
	TestEqual(TEXT("Mythic clears trait 2"), MythicTrait2, EUniqueTrait::None);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUniqueTraitFormulaAccumulateTest,
	"IdleProject.Inventory.UniqueTrait.AccumulateBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUniqueTraitFormulaAccumulateTest::RunTest(const FString& Parameters)
{
	FItemInstance Item = MakeTraitTestItem(EItemSlot::Weapon, EItemRarity::Unique);
	Item.UniqueTrait1 = EUniqueTrait::AllStatSurge;
	Item.UniqueTrait2 = EUniqueTrait::CritDamageSurge;

	FDerivedStats Bonus;
	FUniqueTraitFormula::AccumulateTraitBonus(Item, Bonus);
	TestEqual(TEXT("AllStatSurge grants physical attack"), Bonus.PhysAtk, 0.08f);
	TestEqual(TEXT("AllStatSurge grants magic attack"), Bonus.MagicAtk, 0.08f);
	TestEqual(TEXT("AllStatSurge grants physical defense"), Bonus.PhysDef, 0.08f);
	TestEqual(TEXT("AllStatSurge grants magic defense"), Bonus.MagicDef, 0.08f);
	TestEqual(TEXT("CritDamageSurge grants crit damage"), Bonus.CritDmg, 0.15f);
	TestEqual(TEXT("None HP remains zero"), Bonus.Hp, 0.0f);

	FItemInstance NoneItem = MakeTraitTestItem(EItemSlot::Weapon, EItemRarity::Common);
	FDerivedStats NoneBonus;
	FUniqueTraitFormula::AccumulateTraitBonus(NoneItem, NoneBonus);
	TestEqual(TEXT("No traits grant no attack"), NoneBonus.PhysAtk, 0.0f);
	TestEqual(TEXT("No traits grant no HP"), NoneBonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUniqueTraitInventoryBonusTest,
	"IdleProject.Inventory.UniqueTrait.ComputeEquipmentBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUniqueTraitInventoryBonusTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	FItemInstance Weapon = MakeTraitTestItem(EItemSlot::Weapon, EItemRarity::Unique);
	Weapon.UniqueTrait1 = EUniqueTrait::PhysMastery;
	Inventory->AddItem(Weapon);

	FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Equipment bonus includes base physical attack"), Bonus.PhysAtk, 10.12f);
	TestEqual(TEXT("Equipment bonus does not add unrelated magic attack"), Bonus.MagicAtk, 0.0f);

	FItemInstance Helmet = MakeTraitTestItem(EItemSlot::Helmet, EItemRarity::Transcendent);
	Helmet.UniqueTrait1 = EUniqueTrait::LifeSurge;
	Helmet.UniqueTrait2 = EUniqueTrait::GuardMastery;
	Inventory->AddItem(Helmet);

	Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Equipment bonus includes LifeSurge HP"), Bonus.Hp, 40.15f);
	TestEqual(TEXT("Equipment bonus includes GuardMastery physical defense"), Bonus.PhysDef, 10.15f);
	TestEqual(TEXT("Equipment bonus includes GuardMastery magic defense"), Bonus.MagicDef, 0.15f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUniqueTraitItemFactoryRollTest,
	"IdleProject.Inventory.UniqueTrait.ItemFactoryRolls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUniqueTraitItemFactoryRollTest::RunTest(const FString& Parameters)
{
	FRandomStream Rng(6705);
	bool bFoundUniqueOrTranscendent = false;
	bool bFoundMythic = false;

	for (int32 Index = 0; Index < 5000 && (!bFoundUniqueOrTranscendent || !bFoundMythic); ++Index)
	{
		const FItemInstance Drop = FItemFactory::GuaranteedDropForLevel(100, Rng);
		if (Drop.Rarity == EItemRarity::Unique)
		{
			bFoundUniqueOrTranscendent = true;
			TestTrue(TEXT("Unique factory drop carries one trait"), Drop.UniqueTrait1 >= EUniqueTrait::AllStatSurge && Drop.UniqueTrait1 <= EUniqueTrait::GuardMastery);
			TestEqual(TEXT("Unique factory drop leaves trait 2 empty"), Drop.UniqueTrait2, EUniqueTrait::None);
		}
		if (Drop.Rarity == EItemRarity::Transcendent)
		{
			bFoundUniqueOrTranscendent = true;
			TestTrue(TEXT("Transcendent factory drop carries trait 1"), Drop.UniqueTrait1 >= EUniqueTrait::AllStatSurge && Drop.UniqueTrait1 <= EUniqueTrait::GuardMastery);
			TestTrue(TEXT("Transcendent factory drop carries trait 2"), Drop.UniqueTrait2 >= EUniqueTrait::AllStatSurge && Drop.UniqueTrait2 <= EUniqueTrait::GuardMastery);
			TestNotEqual(TEXT("Transcendent factory drop traits are distinct"), Drop.UniqueTrait1, Drop.UniqueTrait2);
		}
		if (Drop.Rarity == EItemRarity::Mythic)
		{
			bFoundMythic = true;
			TestEqual(TEXT("Mythic factory drop has no trait 1"), Drop.UniqueTrait1, EUniqueTrait::None);
			TestEqual(TEXT("Mythic factory drop has no trait 2"), Drop.UniqueTrait2, EUniqueTrait::None);
		}
	}

	TestTrue(TEXT("High-level factory rolls produce Unique or Transcendent for trait validation"), bFoundUniqueOrTranscendent);
	TestTrue(TEXT("High-level factory rolls produce Mythic for trait exclusion validation"), bFoundMythic);

	return true;
}

#endif
