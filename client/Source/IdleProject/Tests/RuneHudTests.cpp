#include "Misc/AutomationTest.h"

#include "Internationalization/IdleLocalization.h"
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
		1);

	TestEqual(TEXT("Rune panel exposes six slots"), ViewModel.Slots.Num(), FRuneFormula::RuneSlotCount);
	TestEqual(TEXT("Rune panel exposes owned rows"), ViewModel.OwnedRows.Num(), 2);
	TestEqual(TEXT("Selected index is retained"), ViewModel.SelectedOwnedIndex, 1);
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
	TestEqual(TEXT("Codex total is fifty four"), ViewModel.TotalCells, 54);
	TestEqual(TEXT("Codex progress label is localized"), ViewModel.ProgressLabel.ToString(), FString(TEXT("Codex 10/54")));
	TestEqual(TEXT("Codex grid exposes fifty four cells"), ViewModel.Cells.Num(), 54);
	TestEqual(TEXT("Codex grid exposes six row summaries"), ViewModel.Rows.Num(), 6);
	TestEqual(TEXT("Codex grid exposes nine column labels"), ViewModel.ColumnLabels.Num(), 9);

	const FIdleHUDRuneCodexCellViewModel& FirstCell = ViewModel.Cells[0];
	TestEqual(TEXT("First cell row is common"), FirstCell.RowIndex, 0);
	TestEqual(TEXT("First cell column is physical attack"), FirstCell.ColumnIndex, 0);
	TestTrue(TEXT("Unlocked cell is marked unlocked"), FirstCell.bUnlocked);
	TestEqual(TEXT("Unlocked cell status is localized"), FirstCell.StatusLabel.ToString(), FString(TEXT("Unlocked")));

	const FIdleHUDRuneCodexCellViewModel& MythicLastCell = ViewModel.Cells.Last();
	TestEqual(TEXT("Last cell row is mythic"), MythicLastCell.RowIndex, 5);
	TestEqual(TEXT("Last cell column is offline efficiency"), MythicLastCell.ColumnIndex, 8);
	TestTrue(TEXT("Completed mythic row marks last cell unlocked"), MythicLastCell.bUnlocked);

	TestTrue(TEXT("Mythic row completion is surfaced"), ViewModel.Rows[5].bComplete);
	TestEqual(TEXT("Mythic row bonus label is localized"), ViewModel.Rows[5].BonusLabel.ToString(), FString(TEXT("Mythic Row Bonus +12%")));
	TestFalse(TEXT("Core category is incomplete"), ViewModel.bCoreCategoryComplete);
	TestFalse(TEXT("Util category is incomplete"), ViewModel.bUtilCategoryComplete);
	TestEqual(TEXT("Core category incomplete copy is localized"), ViewModel.CoreCategoryLabel.ToString(), FString(TEXT("Core 6/30")));
	TestEqual(TEXT("Util category incomplete copy is localized"), ViewModel.UtilCategoryLabel.ToString(), FString(TEXT("Util 4/24")));
	TestEqual(TEXT("Core bonus label includes cells and row completion"), ViewModel.CoreBonusLabel.ToString(), FString(TEXT("Core +16%")));
	TestEqual(TEXT("Util cap label remains zero before util completion"), ViewModel.UtilCapLabel.ToString(), FString(TEXT("Util Cap +0%")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

#endif
