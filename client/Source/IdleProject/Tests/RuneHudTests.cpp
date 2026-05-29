#include "Misc/AutomationTest.h"

#include "Internationalization/IdleLocalization.h"
#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FRuneInstance MakeHudRune(FName RuneId, ERuneType Type, EItemRarity Rarity, int32 EnhanceLevel = 0)
{
	FRuneInstance Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = Type;
	Rune.Rarity = Rarity;
	Rune.EnhanceLevel = EnhanceLevel;
	return Rune;
}

FRuneInstance MakeHudClassRune(FName RuneId, EClassId ClassId, EItemRarity Rarity, int32 EnhanceLevel = 0)
{
	FRuneInstance Rune = MakeHudRune(RuneId, ERuneType::ClassMastery, Rarity, EnhanceLevel);
	Rune.ClassRestriction = ClassId;
	return Rune;
}

FRuneInstance MakeHudSetRune(FName RuneId, ERuneType Type, EItemRarity Rarity, ERuneSet RuneSet)
{
	FRuneInstance Rune = MakeHudRune(RuneId, Type, Rarity);
	Rune.RuneSet = RuneSet;
	return Rune;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneHudViewModelTest,
	"IdleProject.Rune.HUD.ViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::ResetCacheForTests();
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	URuneService* RuneService = NewObject<URuneService>();
	RuneService->AddRune(MakeHudRune(TEXT("phys_rare"), ERuneType::PhysAtk, EItemRarity::Rare, 2));
	RuneService->AddRune(MakeHudRune(TEXT("gold_mythic"), ERuneType::GoldFind, EItemRarity::Mythic, 0));
	TestTrue(TEXT("Equipped rune setup succeeds"), RuneService->TryEquipRune(0, 0));

	const FIdleHUDRuneViewModel ViewModel = IdleProject::UI::BuildRuneViewModel(
		*RuneService,
		125,
		20000,
		12,
		1,
		0);

	TestEqual(TEXT("Rune panel exposes seven slots"), ViewModel.Slots.Num(), FRuneFormula::RuneSlotCount);
	TestEqual(TEXT("Rune panel exposes owned rows"), ViewModel.OwnedRows.Num(), 2);
	TestEqual(TEXT("Selected index is retained"), ViewModel.SelectedOwnedIndex, 1);

	// 룬 확장4: 선택 룬 액션 섹션.
	TestTrue(TEXT("Action section has selection"), ViewModel.Action.bHasSelection);
	TestEqual(TEXT("Action source index matches selection"), ViewModel.Action.SourceOwnedIndex, 1);
	TestEqual(TEXT("Reroll essence cost matches formula"), ViewModel.Action.RerollEssenceCost, FRuneFormula::GetRerollSetEssenceCost(EItemRarity::Mythic));
	TestTrue(TEXT("Mythic selection cannot upgrade rarity"), ViewModel.Action.bIsMythic);
	TestFalse(TEXT("Mythic upgrade disabled"), ViewModel.Action.bCanUpgrade);
	TestEqual(TEXT("Transfer target resolves to other rune"), ViewModel.Action.TransferTargetOwnedIndex, 0);
	TestEqual(TEXT("Reroll hitbox exposed"), ViewModel.Action.RerollHitBoxName, FName(TEXT("RuneRerollSet")));
	TestEqual(TEXT("Upgrade hitbox exposed"), ViewModel.Action.UpgradeHitBoxName, FName(TEXT("RuneUpgradeRarity")));
	TestEqual(TEXT("Transfer hitbox exposed"), ViewModel.Action.TransferHitBoxName, FName(TEXT("RuneTransfer")));
	TestEqual(TEXT("Essence label is localized"), ViewModel.EssenceLabel.ToString(), FString(TEXT("Rune Essence 125")));

	const FIdleHUDRuneSlotViewModel& EquippedSlot = ViewModel.Slots[0];
	TestTrue(TEXT("Slot zero is marked equipped"), EquippedSlot.bEquipped);
	TestEqual(TEXT("Slot zero stores owned index"), EquippedSlot.OwnedIndex, 0);
	TestEqual(TEXT("Slot zero type label is localized"), EquippedSlot.TypeLabel.ToString(), FString(TEXT("Physical Attack")));
	TestTrue(TEXT("Equipped slot can unequip"), EquippedSlot.bCanUnequip);
	TestFalse(TEXT("Equipped slot cannot equip selected rune"), EquippedSlot.bCanEquipSelected);

	const FIdleHUDRuneOwnedRowViewModel& SelectedRow = ViewModel.OwnedRows[1];
	TestEqual(TEXT("Selected owned row points at rune one"), SelectedRow.OwnedIndex, 1);
	TestEqual(TEXT("Selected owned row type is localized"), SelectedRow.TypeLabel.ToString(), FString(TEXT("Gold Find")));
	TestTrue(TEXT("Selected row can enhance with enough resources"), SelectedRow.bCanEnhance);
	TestTrue(TEXT("Selected row can disenchant when not equipped"), SelectedRow.bCanDisenchant);
	TestTrue(TEXT("Selected row is marked selected"), SelectedRow.bSelected);
	TestEqual(TEXT("Enhance essence cost label uses current level"), SelectedRow.EnhanceCostLabel.ToString(), FString(TEXT("Enhance: Essence 10 / Gold 1,000")));
	TestEqual(TEXT("Enhance preview shows next value"), SelectedRow.EnhancePreviewLabel.ToString(), FString(TEXT("Next +12% -> +13%")));

	TestTrue(TEXT("Rune shop roll is available with enough gold"), ViewModel.bCanBuyRuneRoll);
	TestEqual(TEXT("Rune shop cost uses progress index"), ViewModel.ShopCost, FRuneFormula::GetShopRuneRollCost(12));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneClassSlotHudViewModelTest,
	"IdleProject.Rune.ClassSlot.HUD.ViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneClassSlotHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::ResetCacheForTests();
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	URuneService* RuneService = NewObject<URuneService>();
	RuneService->SetOwnerClassId(EClassId::Warrior);
	RuneService->AddRune(MakeHudClassRune(TEXT("warrior_class"), EClassId::Warrior, EItemRarity::Rare, 1));
	TestTrue(TEXT("Matching class rune equips into class slot"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 0));

	const FIdleHUDRuneViewModel ViewModel = IdleProject::UI::BuildRuneViewModel(
		*RuneService,
		150,
		0,
		12,
		INDEX_NONE,
		INDEX_NONE);

	TestEqual(TEXT("Class craft button label is localized"), ViewModel.ClassCraftButtonLabel.ToString(), FString(TEXT("Craft Class Rune")));
	TestEqual(TEXT("Class craft cost label is localized"), ViewModel.ClassCraftCostLabel.ToString(), FString(TEXT("Craft: Essence 25")));
	TestTrue(TEXT("Class craft is available with enough essence"), ViewModel.bCanCraftClassRune);
	TestEqual(TEXT("Class craft hitbox is exposed"), ViewModel.ClassCraftHitBoxName, FName(TEXT("CraftClassRune")));

	const FIdleHUDRuneSlotViewModel& ClassSlot = ViewModel.Slots[FClassRuneFormula::ClassRuneSlotIndex];
	TestEqual(TEXT("Class slot label uses class copy"), ClassSlot.SlotLabel.ToString(), FString(TEXT("Class Rune")));
	TestEqual(TEXT("Class slot mastery label includes owner class"), ClassSlot.TypeLabel.ToString(), FString(TEXT("Warrior Mastery")));
	TestEqual(TEXT("Class slot value surfaces mastery bonus"), ClassSlot.ValueLabel.ToString(), FString(TEXT("+6%")));

	RuneService->UnequipRune(FClassRuneFormula::ClassRuneSlotIndex);
	const FIdleHUDRuneViewModel EmptyViewModel = IdleProject::UI::BuildRuneViewModel(
		*RuneService,
		10,
		0,
		12,
		0,
		INDEX_NONE);
	const FIdleHUDRuneSlotViewModel& EmptyClassSlot = EmptyViewModel.Slots[FClassRuneFormula::ClassRuneSlotIndex];
	TestEqual(TEXT("Empty class slot uses class empty copy"), EmptyClassSlot.TypeLabel.ToString(), FString(TEXT("No class rune")));
	TestEqual(TEXT("Selected class rune can equip into class slot"), EmptyClassSlot.ActionHitBoxName, FName(TEXT("RuneEquip_6")));
	TestFalse(TEXT("Class craft disabled without essence"), EmptyViewModel.bCanCraftClassRune);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetHudViewModelTest,
	"IdleProject.Rune.Set.HUD.ViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::ResetCacheForTests();
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	URuneService* RuneService = NewObject<URuneService>();
	RuneService->SetOwnerClassId(EClassId::Warrior);
	RuneService->AddRune(MakeHudSetRune(TEXT("offense_phys"), ERuneType::PhysAtk, EItemRarity::Rare, ERuneSet::Offense));
	RuneService->AddRune(MakeHudSetRune(TEXT("offense_magic"), ERuneType::MagicAtk, EItemRarity::Rare, ERuneSet::Offense));
	RuneService->AddRune(MakeHudSetRune(TEXT("vitality_hp"), ERuneType::Hp, EItemRarity::Epic, ERuneSet::Vitality));
	RuneService->AddRune(MakeHudClassRune(TEXT("warrior_class"), EClassId::Warrior, EItemRarity::Rare, 0));
	TestTrue(TEXT("Offense rune 0 equips"), RuneService->TryEquipRune(0, 0));
	TestTrue(TEXT("Offense rune 1 equips"), RuneService->TryEquipRune(1, 1));
	TestTrue(TEXT("Vitality rune equips"), RuneService->TryEquipRune(2, 2));
	TestTrue(TEXT("Class rune equips outside set count"), RuneService->TryEquipRune(FClassRuneFormula::ClassRuneSlotIndex, 3));

	const FIdleHUDRuneViewModel ViewModel = IdleProject::UI::BuildRuneViewModel(
		*RuneService,
		150,
		20000,
		12,
		INDEX_NONE,
		INDEX_NONE);

	TestEqual(TEXT("Rune set panel title is localized"), ViewModel.SetTitle.ToString(), FString(TEXT("Rune Sets")));
	TestEqual(TEXT("Rune set panel exposes four rows"), ViewModel.SetRows.Num(), 4);

	const FIdleHUDRuneSetRowViewModel& OffenseRow = ViewModel.SetRows[0];
	TestEqual(TEXT("Offense set row is first"), static_cast<int32>(OffenseRow.RuneSet), static_cast<int32>(ERuneSet::Offense));
	TestEqual(TEXT("Offense count excludes class slot"), OffenseRow.Count, 2);
	TestEqual(TEXT("Offense count label is localized"), OffenseRow.CountLabel.ToString(), FString(TEXT("Offense 2/6")));
	TestEqual(TEXT("Offense tier label shows active threshold"), OffenseRow.TierLabel.ToString(), FString(TEXT("2-Set Active")));
	TestEqual(TEXT("Offense bonus summarizes active stats"), OffenseRow.BonusLabel.ToString(), FString(TEXT("Physical Attack +5% / Magic Attack +5%")));
	TestEqual(TEXT("Offense next tier label points to four set"), OffenseRow.NextTierLabel.ToString(), FString(TEXT("Next 4-Set: 2 more")));
	TestTrue(TEXT("Offense row is active"), OffenseRow.bActive);
	TestTrue(TEXT("Offense row marks two set active"), OffenseRow.bTwoSetActive);
	TestFalse(TEXT("Offense row does not mark four set active"), OffenseRow.bFourSetActive);
	TestFalse(TEXT("Offense row does not mark six set active"), OffenseRow.bSixSetActive);

	const FIdleHUDRuneSetRowViewModel& BastionRow = ViewModel.SetRows[1];
	TestEqual(TEXT("Bastion empty row keeps no-set copy"), BastionRow.TierLabel.ToString(), FString(TEXT("No set")));
	TestEqual(TEXT("Bastion inactive bonus is localized"), BastionRow.BonusLabel.ToString(), FString(TEXT("No active bonus")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneCodexHudViewModelTest,
	"IdleProject.Rune.Codex.HUD.ViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneCodexHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::ResetCacheForTests();
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	URuneService* RuneService = NewObject<URuneService>();
	RuneService->UnlockCodexCell(ERuneType::PhysAtk, EItemRarity::Common);
	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		RuneService->UnlockCodexCell(static_cast<ERuneType>(TypeValue), EItemRarity::Mythic);
	}

	const FIdleHUDRuneCodexViewModel ViewModel = IdleProject::UI::BuildRuneCodexViewModel(*RuneService);

	TestEqual(TEXT("Codex panel title is localized"), ViewModel.Title.ToString(), FString(TEXT("Rune Codex")));
	TestEqual(TEXT("Codex progress counts unique unlocked cells"), ViewModel.UnlockedCells, 10);
	TestEqual(TEXT("Codex total is sixty three"), ViewModel.TotalCells, 63);
	TestEqual(TEXT("Codex progress label is localized"), ViewModel.ProgressLabel.ToString(), FString(TEXT("Codex 10/63")));
	TestEqual(TEXT("Codex grid exposes sixty three cells"), ViewModel.Cells.Num(), 63);
	TestEqual(TEXT("Codex grid exposes seven row summaries"), ViewModel.Rows.Num(), 7);
	TestEqual(TEXT("Codex grid exposes nine column labels"), ViewModel.ColumnLabels.Num(), 9);

	const FIdleHUDRuneCodexCellViewModel& FirstCell = ViewModel.Cells[0];
	TestEqual(TEXT("First cell row is common"), FirstCell.RowIndex, 0);
	TestEqual(TEXT("First cell column is physical attack"), FirstCell.ColumnIndex, 0);
	TestTrue(TEXT("Unlocked cell is marked unlocked"), FirstCell.bUnlocked);
	TestEqual(TEXT("Unlocked cell status is localized"), FirstCell.StatusLabel.ToString(), FString(TEXT("Unlocked")));

	const FIdleHUDRuneCodexCellViewModel& MythicLastCell = ViewModel.Cells.Last();
	TestEqual(TEXT("Last cell row is mythic"), MythicLastCell.RowIndex, 6);
	TestEqual(TEXT("Last cell column is offline efficiency"), MythicLastCell.ColumnIndex, 8);
	TestTrue(TEXT("Completed mythic row marks last cell unlocked"), MythicLastCell.bUnlocked);

	TestTrue(TEXT("Mythic row completion is surfaced"), ViewModel.Rows[6].bComplete);
	TestEqual(TEXT("Mythic row bonus label is localized"), ViewModel.Rows[6].BonusLabel.ToString(), FString(TEXT("Mythic Row Bonus +12%")));
	TestFalse(TEXT("Core category is incomplete"), ViewModel.bCoreCategoryComplete);
	TestFalse(TEXT("Util category is incomplete"), ViewModel.bUtilCategoryComplete);
	TestEqual(TEXT("Core category incomplete copy is localized"), ViewModel.CoreCategoryLabel.ToString(), FString(TEXT("Core 6/35")));
	TestEqual(TEXT("Util category incomplete copy is localized"), ViewModel.UtilCategoryLabel.ToString(), FString(TEXT("Util 4/28")));
	TestEqual(TEXT("Core bonus label includes cells and row completion"), ViewModel.CoreBonusLabel.ToString(), FString(TEXT("Core +16%")));
	TestEqual(TEXT("Util cap label remains zero before util completion"), ViewModel.UtilCapLabel.ToString(), FString(TEXT("Util Cap +0%")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

#endif
