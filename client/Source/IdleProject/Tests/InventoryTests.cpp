#include "Misc/AutomationTest.h"

#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/DropFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/SetBonusFormula.h"
#include "ItemSystem/ShopFormula.h"
#include "ItemSystem/ItemTypes.h"
#include "UI/IdleHUD.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeTestItem(FName ItemId, EItemSlot Slot, EItemRarity Rarity, float Atk, float Def, float Hp, int32 EnhanceLevel = 0, float CritRate = 0.0f, float AtkSpeed = 0.0f, float MagicAtk = 0.0f, EItemSet ItemSet = EItemSet::None)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.ItemSet = ItemSet;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = Atk;
	Item.BonusDef = Def;
	Item.BonusHp = Hp;
	Item.EnhanceLevel = EnhanceLevel;
	Item.BonusCritRate = CritRate;
	Item.BonusAtkSpeed = AtkSpeed;
	Item.BonusMagicAtk = MagicAtk;
	return Item;
}

int32 CountAffixes(const FItemInstance& Item)
{
	return (Item.BonusCritRate > 0.0f ? 1 : 0)
		+ (Item.BonusAtkSpeed > 0.0f ? 1 : 0)
		+ (Item.BonusMagicAtk > 0.0f ? 1 : 0);
}

void TestAffixCountForRarity(FAutomationTestBase& Test, const TCHAR* Context, const FItemInstance& Item)
{
	const int32 AffixCount = CountAffixes(Item);
	switch (Item.Rarity)
	{
	case EItemRarity::Common:
		Test.TestEqual(FString::Printf(TEXT("%s Common has no affixes"), Context), AffixCount, 0);
		break;
	case EItemRarity::Uncommon:
	case EItemRarity::Rare:
		Test.TestEqual(FString::Printf(TEXT("%s Uncommon/Rare has one affix"), Context), AffixCount, 1);
		break;
	case EItemRarity::Epic:
		Test.TestEqual(FString::Printf(TEXT("%s Epic has two affixes"), Context), AffixCount, 2);
		break;
	case EItemRarity::Legendary:
		Test.TestTrue(FString::Printf(TEXT("%s Legendary has two or three affixes"), Context), AffixCount >= 2 && AffixCount <= 3);
		break;
	case EItemRarity::Mythic:
		Test.TestEqual(FString::Printf(TEXT("%s Mythic has three affixes"), Context), AffixCount, 3);
		break;
	case EItemRarity::None:
	default:
		Test.TestEqual(FString::Printf(TEXT("%s None has no affixes"), Context), AffixCount, 0);
		break;
	}
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopFormulaGearRollCostTest,
	"IdleProject.Inventory.ShopFormula.GearRollCost",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopFormulaGearRollCostTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Negative stage uses stage zero cost"), FShopFormula::GetGearRollCost(-10), static_cast<int64>(300));
	TestEqual(TEXT("Stage zero cost"), FShopFormula::GetGearRollCost(0), static_cast<int64>(300));
	TestEqual(TEXT("Stage one cost rounds 300 * 1.15"), FShopFormula::GetGearRollCost(1), static_cast<int64>(345));
	TestEqual(TEXT("Stage five cost rounds 300 * 1.75"), FShopFormula::GetGearRollCost(5), static_cast<int64>(525));
	TestTrue(TEXT("Gear roll cost increases with stage"), FShopFormula::GetGearRollCost(10) > FShopFormula::GetGearRollCost(1));

	return true;
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
	TestEqual(TEXT("Mythic stat multiplier"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Mythic), 4.5f);
	TestTrue(TEXT("Mythic stat multiplier exceeds Legendary"), FDropFormula::GetRarityStatMultiplier(EItemRarity::Mythic) > FDropFormula::GetRarityStatMultiplier(EItemRarity::Legendary));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaLevelRarityTrendTest,
	"IdleProject.Inventory.DropFormula.LevelRarityTrend",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaLevelRarityTrendTest::RunTest(const FString& Parameters)
{
	constexpr int32 SampleCount = 10000;
	int32 LowRareOrBetter = 0;
	int32 HighRareOrBetter = 0;
	int32 HighEpicOrBetter = 0;
	int32 LowMythic = 0;
	int32 HighMythic = 0;

	FRandomStream LowLevelRng(3601);
	FRandomStream HighLevelRng(3601);
	for (int32 Index = 0; Index < SampleCount; ++Index)
	{
		const EItemRarity LowRarity = FDropFormula::RollRarityForLevel(1, LowLevelRng);
		if (LowRarity >= EItemRarity::Rare)
		{
			++LowRareOrBetter;
		}
		if (LowRarity == EItemRarity::Mythic)
		{
			++LowMythic;
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
		if (HighRarity == EItemRarity::Mythic)
		{
			++HighMythic;
		}
	}

	TestTrue(TEXT("High-level drops roll Rare+ more often than low-level drops"), HighRareOrBetter > LowRareOrBetter);
	TestTrue(TEXT("High-level drops can roll Epic+"), HighEpicOrBetter > 0);
	TestEqual(TEXT("Level 1 drops never roll Mythic"), LowMythic, 0);
	TestTrue(TEXT("Level 100 drops can roll Mythic"), HighMythic > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemRarityMythicHudMappingTest,
	"IdleProject.Inventory.Rarity.MythicHudMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemRarityMythicHudMappingTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));
	TestEqual(TEXT("Mythic rarity label is localized"), IdleProject::UI::RarityToLabel(EItemRarity::Mythic).ToString(), FString(TEXT("Mythic")));
	TestEqual(TEXT("Mythic rarity color uses designer token"), IdleProject::UI::RarityToColor(EItemRarity::Mythic), IdleProject::UI::Theme::RarityMythicStart);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
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
	TestEqual(TEXT("Computed base item has no crit affix"), LegendaryWeapon.BonusCritRate, 0.0f);
	TestEqual(TEXT("Computed base item has no attack speed affix"), LegendaryWeapon.BonusAtkSpeed, 0.0f);
	TestEqual(TEXT("Computed base item has no magic attack affix"), LegendaryWeapon.BonusMagicAtk, 0.0f);

	const FItemInstance EpicAccessory = FDropFormula::ComputeItemBonus(EItemSlot::Accessory, 10, EItemRarity::Epic, 1.0f);
	TestEqual(TEXT("Epic accessory attack split"), EpicAccessory.BonusAtk, 11.5f);
	TestEqual(TEXT("Epic accessory defense split"), EpicAccessory.BonusDef, 6.9f);
	TestEqual(TEXT("Epic accessory HP split"), EpicAccessory.BonusHp, 46.0f);

	const FItemInstance CommonWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 1, EItemRarity::Common, 1.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base attack"), CommonWeapon.BonusAtk, 1.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base defense"), CommonWeapon.BonusDef, 0.0f);
	TestEqual(TEXT("Level 1 Common keeps legacy base HP"), CommonWeapon.BonusHp, 0.0f);

	const FItemInstance RareArmor = FDropFormula::ComputeItemBonus(EItemSlot::Helmet, 10, EItemRarity::Rare, 2.0f);
	TestEqual(TEXT("Rare armor defense uses float slot split"), RareArmor.BonusDef, 23.8f);
	TestEqual(TEXT("Rare armor HP uses float slot split"), RareArmor.BonusHp, 102.0f);

	const FItemInstance MythicWeapon = FDropFormula::ComputeItemBonus(EItemSlot::Weapon, 10, EItemRarity::Mythic, 1.0f);
	TestEqual(TEXT("Mythic weapon attack scales by level variance and rarity"), MythicWeapon.BonusAtk, 45.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRollAffixesTest,
	"IdleProject.Inventory.DropFormula.RollAffixes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRollAffixesTest::RunTest(const FString& Parameters)
{
	FItemInstance Common = MakeTestItem(TEXT("common_sword"), EItemSlot::Weapon, EItemRarity::Common, 1.0f, 0.0f, 0.0f);
	Common.BonusCritRate = 0.05f;
	Common.BonusAtkSpeed = 0.15f;
	Common.BonusMagicAtk = 30.0f;
	FRandomStream CommonRng(4001);
	FDropFormula::RollAffixes(Common.Rarity, 20, CommonRng, Common);
	TestEqual(TEXT("Common items roll no crit affix"), Common.BonusCritRate, 0.0f);
	TestEqual(TEXT("Common items roll no speed affix"), Common.BonusAtkSpeed, 0.0f);
	TestEqual(TEXT("Common items roll no magic affix"), Common.BonusMagicAtk, 0.0f);

	FItemInstance Uncommon = MakeTestItem(TEXT("uncommon_sword"), EItemSlot::Weapon, EItemRarity::Uncommon, 1.0f, 0.0f, 0.0f);
	Uncommon.BonusCritRate = 0.05f;
	Uncommon.BonusAtkSpeed = 0.15f;
	Uncommon.BonusMagicAtk = 30.0f;
	FRandomStream UncommonRng(4001);
	FDropFormula::RollAffixes(Uncommon.Rarity, 20, UncommonRng, Uncommon);
	const int32 UncommonAffixCount = CountAffixes(Uncommon);
	TestEqual(TEXT("Uncommon rolls one affix"), UncommonAffixCount, 1);

	FItemInstance Epic = MakeTestItem(TEXT("epic_sword"), EItemSlot::Weapon, EItemRarity::Epic, 1.0f, 0.0f, 0.0f);
	FRandomStream EpicRng(4002);
	FDropFormula::RollAffixes(Epic.Rarity, 20, EpicRng, Epic);
	const int32 EpicAffixCount = CountAffixes(Epic);
	TestEqual(TEXT("Epic rolls two unique affixes"), EpicAffixCount, 2);
	TestTrue(TEXT("Crit affix stays in range when present"), Epic.BonusCritRate == 0.0f || (Epic.BonusCritRate >= 0.01f && Epic.BonusCritRate <= 0.05f));
	TestTrue(TEXT("Attack speed affix stays in range when present"), Epic.BonusAtkSpeed == 0.0f || (Epic.BonusAtkSpeed >= 0.05f && Epic.BonusAtkSpeed <= 0.15f));
	TestTrue(TEXT("Magic attack affix scales by level when present"), Epic.BonusMagicAtk == 0.0f || (Epic.BonusMagicAtk >= 10.0f && Epic.BonusMagicAtk <= 30.0f));

	FItemInstance LegendaryA = MakeTestItem(TEXT("legendary_a"), EItemSlot::Weapon, EItemRarity::Legendary, 1.0f, 0.0f, 0.0f);
	FItemInstance LegendaryB = MakeTestItem(TEXT("legendary_b"), EItemSlot::Weapon, EItemRarity::Legendary, 1.0f, 0.0f, 0.0f);
	FRandomStream LegendaryRngA(4003);
	FRandomStream LegendaryRngB(4003);
	FDropFormula::RollAffixes(LegendaryA.Rarity, 20, LegendaryRngA, LegendaryA);
	FDropFormula::RollAffixes(LegendaryB.Rarity, 20, LegendaryRngB, LegendaryB);
	const int32 LegendaryAffixCount = CountAffixes(LegendaryA);
	TestTrue(TEXT("Legendary rolls two or three affixes"), LegendaryAffixCount >= 2 && LegendaryAffixCount <= 3);
	TestEqual(TEXT("Legendary roll is deterministic for crit"), LegendaryA.BonusCritRate, LegendaryB.BonusCritRate);
	TestEqual(TEXT("Legendary roll is deterministic for speed"), LegendaryA.BonusAtkSpeed, LegendaryB.BonusAtkSpeed);
	TestEqual(TEXT("Legendary roll is deterministic for magic"), LegendaryA.BonusMagicAtk, LegendaryB.BonusMagicAtk);

	FItemInstance Mythic = MakeTestItem(TEXT("mythic_sword"), EItemSlot::Weapon, EItemRarity::Mythic, 1.0f, 0.0f, 0.0f);
	FRandomStream MythicRng(4004);
	FDropFormula::RollAffixes(Mythic.Rarity, 20, MythicRng, Mythic);
	TestEqual(TEXT("Mythic rolls all three affixes"), CountAffixes(Mythic), 3);
	TestTrue(TEXT("Mythic crit affix stays in range"), Mythic.BonusCritRate >= 0.01f && Mythic.BonusCritRate <= 0.05f);
	TestTrue(TEXT("Mythic attack speed affix stays in range"), Mythic.BonusAtkSpeed >= 0.05f && Mythic.BonusAtkSpeed <= 0.15f);
	TestTrue(TEXT("Mythic magic attack affix scales by level"), Mythic.BonusMagicAtk >= 10.0f && Mythic.BonusMagicAtk <= 30.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDropFormulaRollItemSetTest,
	"IdleProject.Inventory.DropFormula.RollItemSet",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDropFormulaRollItemSetTest::RunTest(const FString& Parameters)
{
	FRandomStream CommonRngA(4301);
	FRandomStream CommonRngB(4301);
	TestEqual(TEXT("Common items do not roll an item set"), FDropFormula::RollItemSet(EItemRarity::Common, CommonRngA), EItemSet::None);
	TestEqual(TEXT("Common item set roll is deterministic"), FDropFormula::RollItemSet(EItemRarity::Common, CommonRngB), EItemSet::None);

	bool bFoundWarrior = false;
	bool bFoundGuardian = false;
	bool bFoundArcane = false;
	FRandomStream RareRng(4302);
	for (int32 Index = 0; Index < 120; ++Index)
	{
		const EItemSet ItemSet = FDropFormula::RollItemSet(EItemRarity::Rare, RareRng);
		TestTrue(TEXT("Rare+ set roll stays in enum range"), ItemSet >= EItemSet::Warrior && ItemSet <= EItemSet::Arcane);
		bFoundWarrior = bFoundWarrior || ItemSet == EItemSet::Warrior;
		bFoundGuardian = bFoundGuardian || ItemSet == EItemSet::Guardian;
		bFoundArcane = bFoundArcane || ItemSet == EItemSet::Arcane;
	}

	TestTrue(TEXT("Rare+ rolls can produce Warrior"), bFoundWarrior);
	TestTrue(TEXT("Rare+ rolls can produce Guardian"), bFoundGuardian);
	TestTrue(TEXT("Rare+ rolls can produce Arcane"), bFoundArcane);

	FRandomStream MythicRng(4303);
	const EItemSet MythicSet = FDropFormula::RollItemSet(EItemRarity::Mythic, MythicRng);
	TestTrue(TEXT("Mythic items roll an item set"), MythicSet >= EItemSet::Warrior && MythicSet <= EItemSet::Arcane);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSetBonusFormulaDefinitionParityTest,
	"IdleProject.Inventory.SetBonusFormula.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSetBonusFormulaDefinitionParityTest::RunTest(const FString& Parameters)
{
	const FDerivedStats WarriorTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Warrior);
	TestEqual(TEXT("Warrior 2-piece attack matches server"), WarriorTwoPiece.PhysAtk, 20.0f);
	TestEqual(TEXT("Warrior 2-piece crit remains zero"), WarriorTwoPiece.CritRate, 0.0f);

	const FDerivedStats WarriorFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Warrior);
	TestEqual(TEXT("Warrior 4-piece adds attack"), WarriorFourPiece.PhysAtk, 50.0f);
	TestEqual(TEXT("Warrior 4-piece adds crit"), WarriorFourPiece.CritRate, 0.05f);

	const FDerivedStats GuardianTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Guardian);
	TestEqual(TEXT("Guardian 2-piece defense matches server"), GuardianTwoPiece.PhysDef, 15.0f);
	TestEqual(TEXT("Guardian 2-piece HP matches server"), GuardianTwoPiece.Hp, 100.0f);

	const FDerivedStats GuardianFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Guardian);
	TestEqual(TEXT("Guardian 4-piece defense matches server"), GuardianFourPiece.PhysDef, 35.0f);
	TestEqual(TEXT("Guardian 4-piece HP matches server"), GuardianFourPiece.Hp, 250.0f);

	const FDerivedStats ArcaneTwoPiece = FSetBonusFormula::GetTwoPieceBonus(EItemSet::Arcane);
	TestEqual(TEXT("Arcane 2-piece magic attack matches server"), ArcaneTwoPiece.MagicAtk, 20.0f);

	const FDerivedStats ArcaneFourPiece = FSetBonusFormula::GetFourPieceBonus(EItemSet::Arcane);
	TestEqual(TEXT("Arcane 4-piece magic attack matches server"), ArcaneFourPiece.MagicAtk, 50.0f);
	TestEqual(TEXT("Arcane 4-piece crit damage matches server"), ArcaneFourPiece.CritDmg, 0.10f);

	const FDerivedStats NoneBonus = FSetBonusFormula::GetTwoPieceBonus(EItemSet::None);
	TestEqual(TEXT("None set has no two-piece attack"), NoneBonus.PhysAtk, 0.0f);
	TestEqual(TEXT("None set has no two-piece HP"), NoneBonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSetBonusFormulaThresholdsTest,
	"IdleProject.Inventory.SetBonusFormula.Thresholds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSetBonusFormulaThresholdsTest::RunTest(const FString& Parameters)
{
	TArray<FItemInstance> EquippedItems;
	EquippedItems.Add(MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 3.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	FDerivedStats Bonus = FSetBonusFormula::ComputeSetBonus(EquippedItems);
	TestEqual(TEXT("Warrior 2-piece grants physical attack"), Bonus.PhysAtk, 20.0f);
	TestEqual(TEXT("Warrior 2-piece does not grant crit"), Bonus.CritRate, 0.0f);

	EquippedItems.Add(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	Bonus = FSetBonusFormula::ComputeSetBonus(EquippedItems);
	TestEqual(TEXT("Warrior 4-piece includes 2-piece and 4-piece attack"), Bonus.PhysAtk, 70.0f);
	TestEqual(TEXT("Warrior 4-piece grants crit rate"), Bonus.CritRate, 0.05f);

	TArray<FItemInstance> MixedItems;
	MixedItems.Add(MakeTestItem(TEXT("none_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 3.0f, 0.0f, 0.0f));
	MixedItems.Add(MakeTestItem(TEXT("guardian_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 1.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Guardian));
	Bonus = FSetBonusFormula::ComputeSetBonus(MixedItems);
	TestEqual(TEXT("None and under-threshold sets grant no attack"), Bonus.PhysAtk, 0.0f);
	TestEqual(TEXT("None and under-threshold sets grant no defense"), Bonus.PhysDef, 0.0f);
	TestEqual(TEXT("None and under-threshold sets grant no HP"), Bonus.Hp, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInventoryEquipmentBonusSetBonusTest,
	"IdleProject.Inventory.Bonus.SetBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusSetBonusTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	const FItemInstance WarriorWeapon = MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior);
	Inventory->AddItem(WarriorWeapon);
	Inventory->AddItem(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	Inventory->AddItem(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	Inventory->AddItem(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Equipment bonus includes per-item attack and Warrior set attack"), Bonus.PhysAtk, 80.0f);
	TestEqual(TEXT("Equipment bonus includes per-item defense only for defense"), Bonus.PhysDef, 15.0f);
	TestEqual(TEXT("Equipment bonus includes Warrior set crit"), Bonus.CritRate, 0.05f);

	const FPrimaryStats Primary(10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f);
	const FDerivedStats Derived = FStatFormulas::DeriveStats(Primary, 1, Bonus);
	TestEqual(TEXT("Derived stats include set physical attack"), Derived.PhysAtk, 100.0f);
	TestEqual(TEXT("Derived stats include set crit rate"), Derived.CritRate, 0.07f);
	TestEqual(TEXT("PowerScore ignores item set membership"), FItemPowerScore::Compute(WarriorWeapon), 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceFormulaCurveTest,
	"IdleProject.Inventory.EnhanceFormula.Curve",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceFormulaCurveTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Max enhance level matches item clamp"), FEnhanceFormula::MaxEnhanceLevel, 50);
	TestEqual(TEXT("Level 0 cost"), FEnhanceFormula::GetEnhanceCost(0), static_cast<int64>(100));
	TestEqual(TEXT("Level 1 cost"), FEnhanceFormula::GetEnhanceCost(1), static_cast<int64>(400));
	TestEqual(TEXT("Negative level uses level 0 cost"), FEnhanceFormula::GetEnhanceCost(-1), static_cast<int64>(100));
	TestEqual(TEXT("Max level has no next enhance cost"), FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel), static_cast<int64>(0));
	TestEqual(TEXT("None rarity has no enhance cost multiplier"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::None), static_cast<int64>(0));
	TestEqual(TEXT("Common rarity keeps legacy cost multiplier"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Common), static_cast<int64>(1));
	TestEqual(TEXT("Uncommon rarity doubles enhance cost"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Uncommon), static_cast<int64>(2));
	TestEqual(TEXT("Rare rarity quadruples enhance cost"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Rare), static_cast<int64>(4));
	TestEqual(TEXT("Epic rarity multiplies enhance cost by eight"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Epic), static_cast<int64>(8));
	TestEqual(TEXT("Legendary rarity multiplies enhance cost by sixteen"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Legendary), static_cast<int64>(16));
	TestEqual(TEXT("Mythic rarity multiplies enhance cost by thirty-two"), FEnhanceFormula::GetRarityCostMultiplier(EItemRarity::Mythic), static_cast<int64>(32));
	TestEqual(TEXT("Common overload matches legacy single-argument cost"), FEnhanceFormula::GetEnhanceCost(1, EItemRarity::Common), static_cast<int64>(400));
	TestEqual(TEXT("Rare level 1 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(1, EItemRarity::Rare), static_cast<int64>(1600));
	TestEqual(TEXT("Legendary level 0 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Legendary), static_cast<int64>(1600));
	TestEqual(TEXT("Mythic level 0 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Mythic), static_cast<int64>(3200));
	TestEqual(TEXT("Max level rarity cost remains zero"), FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel, EItemRarity::Legendary), static_cast<int64>(0));
	TestEqual(TEXT("Uncommon level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Uncommon), static_cast<int64>(5000));
	TestEqual(TEXT("Rare level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Rare), static_cast<int64>(10000));
	TestEqual(TEXT("Epic level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Epic), static_cast<int64>(20000));
	TestEqual(TEXT("Legendary level 4 cost matches server parity table"), FEnhanceFormula::GetEnhanceCost(4, EItemRarity::Legendary), static_cast<int64>(40000));
	TestEqual(TEXT("Common level 49 cost reaches 50-level sink"), FEnhanceFormula::GetEnhanceCost(49), static_cast<int64>(250000));
	TestEqual(TEXT("Legendary level 49 cost applies rarity multiplier"), FEnhanceFormula::GetEnhanceCost(49, EItemRarity::Legendary), static_cast<int64>(4000000));

	TestEqual(TEXT("Level 0 success rate"), FEnhanceFormula::GetEnhanceSuccessRate(0), 0.95f);
	TestTrue(TEXT("Level 4 success rate remains on the 50-level curve"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(4), 0.878f, 0.0001f));
	TestTrue(TEXT("Level 10 success rate follows linear curve"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(10), 0.77f, 0.0001f));
	TestTrue(TEXT("Level 25 success rate reaches midpoint pressure"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(25), 0.50f, 0.0001f));
	TestTrue(TEXT("Level 40 success rate reaches high-level pressure"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(40), 0.23f, 0.0001f));
	TestTrue(TEXT("Level 49 success rate stays above floor"), FMath::IsNearlyEqual(FEnhanceFormula::GetEnhanceSuccessRate(49), 0.068f, 0.0001f));
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

	const FItemInstance AffixSword = MakeTestItem(TEXT("affix_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 2, 0.02f, 0.10f, 5.0f);
	TestEqual(TEXT("Affixes contribute weighted power before enhance multiplier"), FItemPowerScore::Compute(AffixSword), 54);

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
	const FDerivedStats MaxBonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Lv50 enhanced item applies x6 equipment attack bonus"), MaxBonus.PhysAtk, 60.0f);
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
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0));
	Inventory->AddItem(MakeTestItem(TEXT("legendary_gloves"), EItemSlot::Gloves, EItemRarity::Legendary, 0.0f, 1.0f, 0.0f, 0));
	Inventory->AddItem(MakeTestItem(TEXT("common_cloak"), EItemSlot::Cloak, EItemRarity::Common, 0.0f, 1.0f, 0.0f, 0));

	const FIdleHUDEnhancePanelViewModel NoGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		0,
		FText::GetEmpty(),
		false);
	TestEqual(TEXT("Enhance panel exposes all equipment slots"), NoGold.Rows.Num(), 8);
	TestEqual(TEXT("Weapon row uses current enhance level"), NoGold.Rows[0].EnhanceLevel, 0);
	TestEqual(TEXT("Rare weapon row shows rarity-scaled level zero cost"), NoGold.Rows[0].Cost, static_cast<int64>(400));
	TestEqual(TEXT("Weapon row level label shows +N / 50 without a plus on the cap"), NoGold.Rows[0].LevelLabel.ToString(), FString(TEXT("+0 / 50")));
	TestEqual(TEXT("Weapon row success label shows integer percent"), NoGold.Rows[0].SuccessRateLabel.ToString(), FString(TEXT("Success 95%")));
	TestEqual(TEXT("Legendary gloves row shows rarity-scaled level zero cost"), NoGold.Rows[5].Cost, static_cast<int64>(1600));
	TestEqual(TEXT("Common cloak row preserves legacy level zero cost"), NoGold.Rows[6].Cost, static_cast<int64>(100));
	TestFalse(TEXT("Weapon row is disabled without gold"), NoGold.Rows[0].bCanEnhance);
	TestFalse(TEXT("Weapon row reports insufficient gold"), NoGold.Rows[0].bGoldEnough);
	TestFalse(TEXT("Empty helmet row is not enhanceable"), NoGold.Rows[1].bCanEnhance);
	TestFalse(TEXT("Empty helmet row reports no equipment"), NoGold.Rows[1].bEquipped);

	const FIdleHUDEnhancePanelViewModel EnoughGold = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Rare),
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
	TestEqual(TEXT("Max level row shows +50 / 50 level label"), MaxLevel.Rows[0].LevelLabel.ToString(), FString(TEXT("+50 / 50")));
	TestEqual(TEXT("Max level row shows localized max status"), MaxLevel.Rows[0].StatusLabel.ToString(), FString(TEXT("MAX")));
	TestEqual(TEXT("Max level row shows max instead of next cost"), MaxLevel.Rows[0].CostLabel.ToString(), FString(TEXT("MAX")));
	TestEqual(TEXT("Max level row shows max instead of success percent"), MaxLevel.Rows[0].SuccessRateLabel.ToString(), FString(TEXT("MAX")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentAffixHudSummaryTest,
	"IdleProject.UI.HUD.EquipmentAffixSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentAffixHudSummaryTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FItemInstance PlainItem = MakeTestItem(TEXT("plain_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f);
	TestTrue(TEXT("Zero affixes produce an empty summary"), IdleProject::UI::BuildAffixSummary(PlainItem).IsEmpty());

	const FItemInstance PartialAffixItem = MakeTestItem(TEXT("partial_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.03f, 0.0f, 12.0f);
	TestEqual(
		TEXT("Only positive affixes are shown"),
		IdleProject::UI::BuildAffixSummary(PartialAffixItem).ToString(),
		FString(TEXT("Crit +3% / MATK +12")));

	const FItemInstance FullAffixItem = MakeTestItem(TEXT("full_sword"), EItemSlot::Weapon, EItemRarity::Legendary, 10.0f, 0.0f, 0.0f, 0, 0.03f, 0.10f, 12.0f);
	TestEqual(
		TEXT("Affix summary uses stable crit speed magic order"),
		IdleProject::UI::BuildAffixSummary(FullAffixItem).ToString(),
		FString(TEXT("Crit +3% / ASPD +0.10 / MATK +12")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEquipmentSetHudSummaryTest,
	"IdleProject.UI.HUD.EquipmentSetSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEquipmentSetHudSummaryTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	TArray<FItemInstance> EquippedItems;
	EquippedItems.Add(MakeTestItem(TEXT("warrior_weapon"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("warrior_top"), EItemSlot::Top, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	EquippedItems.Add(MakeTestItem(TEXT("common_shoes"), EItemSlot::Shoes, EItemRarity::Common, 0.0f, 2.0f, 5.0f));

	const FIdleHUDSetSummaryViewModel ViewModel = IdleProject::UI::BuildSetSummaryViewModel(EquippedItems);
	TestEqual(TEXT("Only non-empty item sets are summarized"), ViewModel.Rows.Num(), 1);
	TestEqual(TEXT("Warrior row counts three pieces toward four-piece cap"), ViewModel.Rows[0].PieceCount, 3);
	TestEqual(TEXT("Warrior row exposes current tier label"), ViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("2-piece active")));
	TestEqual(TEXT("Warrior row exposes next tier label"), ViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Next 4-piece: 1 more")));
	TestEqual(TEXT("Warrior row summarizes active set bonus"), ViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: PATK +20")));
	TestEqual(TEXT("Warrior row summary is compact for equipment HUD"), ViewModel.Rows[0].SummaryLabel.ToString(), FString(TEXT("Warrior Set 3/4 (2-piece active)")));

	EquippedItems.Add(MakeTestItem(TEXT("warrior_bottom"), EItemSlot::Bottom, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Warrior));
	const FIdleHUDSetSummaryViewModel FourPieceViewModel = IdleProject::UI::BuildSetSummaryViewModel(EquippedItems);
	TestEqual(TEXT("Four-piece row exposes four-piece tier label"), FourPieceViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("4-piece active")));
	TestEqual(TEXT("Four-piece row reports complete state"), FourPieceViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Set complete")));
	TestEqual(TEXT("Four-piece row summarizes cumulative set bonus"), FourPieceViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: PATK +70 / Crit +5%")));

	TArray<FItemInstance> UnderThresholdItems;
	UnderThresholdItems.Add(MakeTestItem(TEXT("guardian_helmet"), EItemSlot::Helmet, EItemRarity::Rare, 0.0f, 5.0f, 10.0f, 0, 0.0f, 0.0f, 0.0f, EItemSet::Guardian));
	const FIdleHUDSetSummaryViewModel UnderThresholdViewModel = IdleProject::UI::BuildSetSummaryViewModel(UnderThresholdItems);
	TestEqual(TEXT("Under-threshold row exposes inactive tier"), UnderThresholdViewModel.Rows[0].TierLabel.ToString(), FString(TEXT("No set bonus")));
	TestEqual(TEXT("Under-threshold row points to two-piece"), UnderThresholdViewModel.Rows[0].NextTierLabel.ToString(), FString(TEXT("Next 2-piece: 1 more")));
	TestEqual(TEXT("Under-threshold row has no active bonus"), UnderThresholdViewModel.Rows[0].BonusLabel.ToString(), FString(TEXT("Bonus: -")));

	TArray<FItemInstance> NoSetItems;
	NoSetItems.Add(MakeTestItem(TEXT("plain_sword"), EItemSlot::Weapon, EItemRarity::Common, 10.0f, 0.0f, 0.0f));
	const FIdleHUDSetSummaryViewModel NoSetViewModel = IdleProject::UI::BuildSetSummaryViewModel(NoSetItems);
	TestEqual(TEXT("No-set equipment does not create a set row"), NoSetViewModel.Rows.Num(), 0);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FShopPanelViewModelStateTest,
	"IdleProject.UI.HUD.ShopPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShopPanelViewModelStateTest::RunTest(const FString& Parameters)
{
	FShopPurchaseResult PurchaseResult;
	PurchaseResult.bPurchased = true;
	PurchaseResult.GoldSpent = 300;
	PurchaseResult.Rarity = EItemRarity::Rare;
	PurchaseResult.Slot = EItemSlot::Weapon;
	PurchaseResult.ItemName = FText::FromString(TEXT("rare_sword"));

	const FIdleHUDShopPanelViewModel Ready = IdleProject::UI::BuildShopPanelViewModel(300, 450, PurchaseResult);
	TestEqual(TEXT("Shop panel exposes gear roll cost"), Ready.GearRollCost, static_cast<int64>(300));
	TestTrue(TEXT("Shop panel enables purchase when gold is enough"), Ready.bCanBuyGearRoll);
	TestEqual(TEXT("Shop gear roll hitbox is stable"), Ready.GearRollHitBoxName, FName(TEXT("ShopGearRoll")));
	TestEqual(TEXT("Shop result carries purchased rarity"), Ready.LastResultRarity, EItemRarity::Rare);
	TestTrue(TEXT("Shop result is visible after purchase"), Ready.bHasLastResult);
	TestFalse(TEXT("Shop result is not an error after purchase"), Ready.bLastResultError);

	const FIdleHUDShopPanelViewModel NoGold = IdleProject::UI::BuildShopPanelViewModel(300, 250, FShopPurchaseResult());
	TestFalse(TEXT("Shop panel disables purchase when gold is short"), NoGold.bCanBuyGearRoll);
	TestFalse(TEXT("Empty shop result is hidden"), NoGold.bHasLastResult);

	FShopPurchaseResult FailedResult;
	FailedResult.bPurchased = false;
	FailedResult.GoldSpent = 300;
	const FIdleHUDShopPanelViewModel Failed = IdleProject::UI::BuildShopPanelViewModel(300, 250, FailedResult);
	TestTrue(TEXT("Failed purchase result is visible when a cost was attempted"), Failed.bHasLastResult);
	TestTrue(TEXT("Failed purchase result is flagged as an error"), Failed.bLastResultError);

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
	FInventoryEquipmentBonusAffixTest,
	"IdleProject.Inventory.Bonus.Affixes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FInventoryEquipmentBonusAffixTest::RunTest(const FString& Parameters)
{
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeTestItem(TEXT("rare_sword"), EItemSlot::Weapon, EItemRarity::Rare, 10.0f, 0.0f, 0.0f, 2, 0.02f, 0.10f, 5.0f));

	const FDerivedStats Bonus = Inventory->ComputeEquipmentBonus();
	TestEqual(TEXT("Enhanced item increases physical attack"), Bonus.PhysAtk, 12.0f);
	TestEqual(TEXT("Enhanced item increases crit affix"), Bonus.CritRate, 0.024f);
	TestEqual(TEXT("Enhanced item increases attack speed affix"), Bonus.AtkSpeed, 0.12f);
	TestEqual(TEXT("Enhanced item increases magic attack affix"), Bonus.MagicAtk, 6.0f);

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
	FItemFactoryGuaranteedDropForLevelTest,
	"IdleProject.Inventory.ItemFactory.GuaranteedDropForLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryGuaranteedDropForLevelTest::RunTest(const FString& Parameters)
{
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FItemInstance Drop = FItemFactory::GuaranteedDropForLevel(1);
		TestTrue(TEXT("Guaranteed drop never returns None rarity"), Drop.Rarity >= EItemRarity::Common);
		TestTrue(TEXT("Guaranteed drop has an equipment slot"), Drop.Slot >= EItemSlot::Weapon && Drop.Slot <= EItemSlot::Accessory);
		TestTrue(TEXT("Guaranteed drop has stats"), FItemPowerScore::Compute(Drop) > 0);
	}

	const FItemInstance LowLevelDrop = FItemFactory::GuaranteedDropForLevel(-20);
	const FItemInstance HighLevelDrop = FItemFactory::GuaranteedDropForLevel(20);
	TestTrue(TEXT("Guaranteed drop clamps invalid levels to playable stats"), FItemPowerScore::Compute(LowLevelDrop) > 0);
	TestTrue(TEXT("Guaranteed drop level affects item stats"), FItemPowerScore::Compute(HighLevelDrop) > FItemPowerScore::Compute(LowLevelDrop));

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
		TestTrue(TEXT("등급 범위"), Drop.Rarity >= EItemRarity::None && Drop.Rarity <= EItemRarity::Mythic);

		if (Drop.Rarity == EItemRarity::None)
		{
			TestEqual(TEXT("드롭 없음 슬롯"), static_cast<int32>(Drop.Slot), static_cast<int32>(EItemSlot::None));
			continue;
		}

		TestTrue(TEXT("슬롯 범위"), Drop.Slot >= EItemSlot::Weapon && Drop.Slot <= EItemSlot::Accessory);
		TestTrue(TEXT("보너스 중 하나 이상"), Drop.BonusAtk > 0.0f || Drop.BonusDef > 0.0f || Drop.BonusHp > 0.0f);
		TestEqual(TEXT("드롭 강화 기본값"), Drop.EnhanceLevel, 0);
		TestAffixCountForRarity(*this, TEXT("RandomDropFromMonster"), Drop);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FItemFactoryHighLevelDropIncludesExpandedRarityTest,
	"IdleProject.Inventory.ItemFactory.HighLevelExpandedRarity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FItemFactoryHighLevelDropIncludesExpandedRarityTest::RunTest(const FString& Parameters)
{
	bool bFoundEpicOrHigher = false;
	for (int32 Index = 0; Index < 2000; ++Index)
	{
		const FItemInstance Drop = FItemFactory::RandomDropFromMonster(100);
		if (Drop.Rarity >= EItemRarity::Epic)
		{
			bFoundEpicOrHigher = true;
			TestTrue(TEXT("Epic+ drop carries scaled stats"), FItemPowerScore::Compute(Drop) > 0);
			TestAffixCountForRarity(*this, TEXT("High-level random drop"), Drop);
			break;
		}
	}

	TestTrue(TEXT("High-level monster drops can produce Epic, Legendary, or Mythic items"), bFoundEpicOrHigher);

	return true;
}

#endif
