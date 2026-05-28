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

#endif
