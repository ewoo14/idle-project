#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "RuneSystem/RuneCodexFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FRuneInstance MakeRune(FName RuneId, ERuneType Type, EItemRarity Rarity, int32 EnhanceLevel = 0)
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
	FRuneServiceCodexUnlockTest,
	"IdleProject.Rune.Service.CodexUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceCodexUnlockTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	TestEqual(TEXT("New codex has all sixty three cells modeled"), RuneService->GetOwnedCodex().Num(), FRuneCodexFormula::TotalCells);
	TestEqual(TEXT("New codex has no unlocked cells"), RuneService->GetCodexCompletion().UnlockedCells, 0);
	TestEqual(TEXT("New codex has no core bonus"), RuneService->GetCodexBonus().CoreStatAdd, 0.0f);

	RuneService->AddRune(MakeRune(TEXT("phys_common_1"), ERuneType::PhysAtk, EItemRarity::Common, 0));
	TestEqual(TEXT("AddRune unlocks the matching codex cell"), RuneService->GetCodexCompletion().UnlockedCells, 1);
	TestEqual(TEXT("One codex cell adds per-cell core bonus"), RuneService->GetCodexBonus().CoreStatAdd, FRuneCodexFormula::PerCellCoreBonus);

	RuneService->AddRune(MakeRune(TEXT("phys_common_2"), ERuneType::PhysAtk, EItemRarity::Common, 10));
	TestEqual(TEXT("Duplicate rune type and rarity does not unlock another cell"), RuneService->GetCodexCompletion().UnlockedCells, 1);

	int64 Refund = 0;
	TestTrue(TEXT("Unlocked rune can be disenchanted"), RuneService->TryDisenchantRune(1, Refund));
	TestEqual(TEXT("Codex unlock persists after disenchant"), RuneService->GetCodexCompletion().UnlockedCells, 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceCodexCompletionAndRestoreTest,
	"IdleProject.Rune.Service.CodexCompletionAndRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceCodexCompletionAndRestoreTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		RuneService->AddRune(MakeRune(
			*FString::Printf(TEXT("common_%d"), TypeValue),
			static_cast<ERuneType>(TypeValue),
			EItemRarity::Common));
	}

	FRuneCodexCompletion Completion = RuneService->GetCodexCompletion();
	TestEqual(TEXT("One full rarity row unlocks nine cells"), Completion.UnlockedCells, 9);
	TestTrue(TEXT("Common row is complete"), Completion.RowComplete[0]);
	TestEqual(TEXT("Common row bonus is included"), RuneService->GetCodexBonus().CoreStatAdd, 9.0f * FRuneCodexFormula::PerCellCoreBonus + 0.01f);

	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::Hp); ++TypeValue)
	{
		for (int32 RarityValue = static_cast<int32>(EItemRarity::Rare); RarityValue <= static_cast<int32>(EItemRarity::Mythic); ++RarityValue)
		{
			RuneService->AddRune(MakeRune(
				*FString::Printf(TEXT("core_%d_%d"), TypeValue, RarityValue),
				static_cast<ERuneType>(TypeValue),
				static_cast<EItemRarity>(RarityValue)));
		}
	}

	Completion = RuneService->GetCodexCompletion();
	TestTrue(TEXT("All core type and rarity cells complete the core category"), Completion.bCoreCategoryComplete);
	TestFalse(TEXT("Partial util cells do not complete util category"), Completion.bUtilCategoryComplete);

	TArray<FRuneSaveEntry> CapturedRunes;
	TArray<int32> CapturedSlots;
	TArray<FRuneCodexEntry> CapturedCodex;
	RuneService->CaptureState(CapturedRunes, CapturedSlots, CapturedCodex);

	URuneService* RestoredService = NewObject<URuneService>();
	RestoredService->RestoreState(CapturedRunes, CapturedSlots, CapturedCodex);
	TestEqual(TEXT("Codex completion round trips through capture restore"), RestoredService->GetCodexCompletion().UnlockedCells, Completion.UnlockedCells);
	TestTrue(TEXT("Core category completion round trips"), RestoredService->GetCodexCompletion().bCoreCategoryComplete);

	TArray<FRuneCodexEntry> InvalidCodex = CapturedCodex;
	InvalidCodex.Add(FRuneCodexEntry());
	InvalidCodex[0].RuneType = ERuneType::None;
	InvalidCodex[0].Rarity = EItemRarity::None;
	InvalidCodex[0].bUnlocked = true;
	RestoredService->RestoreState(CapturedRunes, CapturedSlots, InvalidCodex);
	TestEqual(TEXT("Restore sanitizes invalid codex cells back to a sixty three cell grid"), RestoredService->GetOwnedCodex().Num(), FRuneCodexFormula::TotalCells);
	TestFalse(TEXT("Invalid codex entry does not unlock None/None"), RestoredService->GetOwnedCodex()[0].RuneType == ERuneType::None);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceCodexUtilCapTest,
	"IdleProject.Rune.Service.CodexUtilCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceCodexUtilCapTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->AddRune(MakeRune(TEXT("gold_cap"), ERuneType::GoldFind, EItemRarity::Mythic, 10000));
	RuneService->AddRune(MakeRune(TEXT("gold_cap_2"), ERuneType::GoldFind, EItemRarity::Mythic, 10000));
	TestTrue(TEXT("High util rune equips"), RuneService->TryEquipRune(0, 0));
	TestTrue(TEXT("Second high util rune equips"), RuneService->TryEquipRune(1, 1));
	TestEqual(TEXT("Util cap starts at base formula cap"), RuneService->GetEquippedUtilValues().GoldFind, FRuneFormula::GetUtilCap(ERuneType::GoldFind));

	for (int32 TypeValue = static_cast<int32>(ERuneType::CritDamage); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		for (int32 RarityValue = static_cast<int32>(EItemRarity::Common); RarityValue <= static_cast<int32>(EItemRarity::Mythic); ++RarityValue)
		{
			RuneService->UnlockCodexCell(static_cast<ERuneType>(TypeValue), static_cast<EItemRarity>(RarityValue));
		}
	}

	TestTrue(TEXT("Util category is complete"), RuneService->GetCodexCompletion().bUtilCategoryComplete);
	TestEqual(TEXT("Completed util category extends equipped util cap"), RuneService->GetEquippedUtilValues().GoldFind, FRuneFormula::GetUtilCap(ERuneType::GoldFind) + FRuneCodexFormula::UtilCategoryCapExtension);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceEquipAndMultiplierTest,
	"IdleProject.Rune.Service.EquipAndMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceEquipAndMultiplierTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	TestNotNull(TEXT("Rune service is created"), RuneService);
	if (!RuneService)
	{
		return false;
	}

	const FRuneCoreMultipliers EmptyCore = RuneService->GetEquippedCoreMultipliers();
	const FRuneUtilValues EmptyUtil = RuneService->GetEquippedUtilValues();
	TestEqual(TEXT("Empty core physical attack is neutral"), EmptyCore.PhysAtk, 1.0f);
	TestEqual(TEXT("Empty core magic attack is neutral"), EmptyCore.MagicAtk, 1.0f);
	TestEqual(TEXT("Empty core HP is neutral"), EmptyCore.Hp, 1.0f);
	TestEqual(TEXT("Empty util crit damage is zero"), EmptyUtil.CritDamage, 0.0f);
	TestEqual(TEXT("Empty util gold find is zero"), EmptyUtil.GoldFind, 0.0f);

	RuneService->AddRune(MakeRune(TEXT("phys_rare"), ERuneType::PhysAtk, EItemRarity::Rare, 10));
	RuneService->AddRune(MakeRune(TEXT("gold_mythic"), ERuneType::GoldFind, EItemRarity::Mythic, 0));
	RuneService->AddRune(MakeRune(TEXT("phys_epic"), ERuneType::PhysAtk, EItemRarity::Epic, 5));
	TestEqual(TEXT("AddRune increases owned count"), RuneService->GetOwnedRunes().Num(), 3);

	TestTrue(TEXT("Core rune equips into slot zero"), RuneService->TryEquipRune(0, 0));
	TestTrue(TEXT("Util rune equips into slot one"), RuneService->TryEquipRune(1, 1));
	TestTrue(TEXT("Second core rune equips into slot two"), RuneService->TryEquipRune(2, 2));
	TestEqual(TEXT("Slot zero points at first owned rune"), RuneService->GetEquippedOwnedIndex(0), 0);
	TestEqual(TEXT("Slot one points at second owned rune"), RuneService->GetEquippedOwnedIndex(1), 1);
	TestEqual(TEXT("Slot two points at third owned rune"), RuneService->GetEquippedOwnedIndex(2), 2);

	const FRuneCoreMultipliers Core = RuneService->GetEquippedCoreMultipliers();
	const FRuneUtilValues Util = RuneService->GetEquippedUtilValues();
	TestEqual(TEXT("Equipped phys attack runes add flat bonuses to one multiplier"), Core.PhysAtk, 1.0f + FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 10) + FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Epic, 5));
	TestEqual(TEXT("Other core stats remain neutral"), Core.MagicAtk, 1.0f);
	TestEqual(TEXT("Equipped gold find uses util formula"), Util.GoldFind, FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::Mythic, 0));

	TestTrue(TEXT("Unequip clears slot"), RuneService->UnequipRune(0));
	TestEqual(TEXT("Unequipped slot returns INDEX_NONE"), RuneService->GetEquippedOwnedIndex(0), INDEX_NONE);
	TestFalse(TEXT("Invalid slot cannot equip"), RuneService->TryEquipRune(FRuneFormula::RuneSlotCount, 0));
	TestFalse(TEXT("Invalid owned index cannot equip"), RuneService->TryEquipRune(0, 99));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceEnhanceDisenchantTest,
	"IdleProject.Rune.Service.EnhanceDisenchant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceEnhanceDisenchantTest::RunTest(const FString& Parameters)
{
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->AddRune(MakeRune(TEXT("phys_common"), ERuneType::PhysAtk, EItemRarity::Common, 0));
	RuneService->AddRune(MakeRune(TEXT("crit_rare"), ERuneType::CritDamage, EItemRarity::Rare, 4));

	TestTrue(TEXT("EnhanceRune increments enhance level"), RuneService->EnhanceRune(0));
	TestEqual(TEXT("Enhanced rune level is stored"), RuneService->GetOwnedRunes()[0].EnhanceLevel, 1);
	TestTrue(TEXT("Enhanced multiplier increases"), FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Common, 1) > FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Common, 0));

	TestTrue(TEXT("Equipped rune can be equipped before disenchant guard"), RuneService->TryEquipRune(0, 0));
	int64 Refund = 0;
	TestFalse(TEXT("Equipped rune cannot be disenchanted"), RuneService->TryDisenchantRune(0, Refund));
	TestEqual(TEXT("Failed disenchant does not refund essence"), Refund, static_cast<int64>(0));

	TestTrue(TEXT("Unequipped rune can be disenchanted"), RuneService->TryDisenchantRune(1, Refund));
	TestEqual(TEXT("Disenchant refund uses formula"), Refund, FRuneFormula::GetDisenchantEssence(EItemRarity::Rare, 4));
	TestEqual(TEXT("Disenchant removes owned rune"), RuneService->GetOwnedRunes().Num(), 1);
	TestEqual(TEXT("Equipped index survives removal before another index"), RuneService->GetEquippedOwnedIndex(0), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceRestoreSanitizeTest,
	"IdleProject.Rune.Service.RestoreSanitize",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceRestoreSanitizeTest::RunTest(const FString& Parameters)
{
	TArray<FRuneSaveEntry> SavedRunes;
	FRuneSaveEntry InvalidType;
	InvalidType.RuneId = TEXT("invalid_type");
	InvalidType.RuneType = ERuneType::None;
	InvalidType.Rarity = EItemRarity::Rare;
	SavedRunes.Add(InvalidType);

	FRuneSaveEntry ValidPhys;
	ValidPhys.RuneId = TEXT("valid_phys");
	ValidPhys.RuneType = ERuneType::PhysAtk;
	ValidPhys.Rarity = EItemRarity::Rare;
	ValidPhys.EnhanceLevel = 5;
	SavedRunes.Add(ValidPhys);

	FRuneSaveEntry InvalidRarity;
	InvalidRarity.RuneId = TEXT("invalid_rarity");
	InvalidRarity.RuneType = ERuneType::GoldFind;
	InvalidRarity.Rarity = EItemRarity::None;
	SavedRunes.Add(InvalidRarity);

	FRuneSaveEntry ValidGold;
	ValidGold.RuneId = TEXT("valid_gold");
	ValidGold.RuneType = ERuneType::GoldFind;
	ValidGold.Rarity = EItemRarity::Mythic;
	ValidGold.EnhanceLevel = -3;
	SavedRunes.Add(ValidGold);

	TArray<int32> SavedEquipped;
	SavedEquipped.Init(INDEX_NONE, FRuneFormula::RuneSlotCount);
	SavedEquipped[0] = 1;
	SavedEquipped[1] = 3;
	SavedEquipped[2] = 99;
	SavedEquipped[3] = 0;

	URuneService* RuneService = NewObject<URuneService>();
	RuneService->RestoreState(SavedRunes, SavedEquipped);

	TArray<FRuneSaveEntry> CapturedRunes;
	TArray<int32> CapturedSlots;
	RuneService->CaptureState(CapturedRunes, CapturedSlots);

	TestEqual(TEXT("Restore keeps only valid rune entries"), CapturedRunes.Num(), 2);
	TestEqual(TEXT("First valid rune remaps from old index one to zero"), CapturedRunes[0].RuneId, FName(TEXT("valid_phys")));
	TestEqual(TEXT("Second valid rune remaps from old index three to one"), CapturedRunes[1].RuneId, FName(TEXT("valid_gold")));
	TestEqual(TEXT("Negative enhance level clamps to zero"), CapturedRunes[1].EnhanceLevel, 0);
	TestEqual(TEXT("Equipped slot remaps old index one to new index zero"), CapturedSlots[0], 0);
	TestEqual(TEXT("Equipped slot remaps old index three to new index one"), CapturedSlots[1], 1);
	TestEqual(TEXT("Out of range equipped slot is cleared"), CapturedSlots[2], INDEX_NONE);
	TestEqual(TEXT("Equipped slot pointing at dropped rune is cleared"), CapturedSlots[3], INDEX_NONE);
	TestEqual(TEXT("Captured slot array always has seven entries"), CapturedSlots.Num(), FRuneFormula::RuneSlotCount);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneGameInstanceSaveAndEconomyTest,
	"IdleProject.Rune.GameInstance.SaveAndEconomy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneGameInstanceSaveAndEconomyTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeRuneServiceForTests();
	GameInstance->AddGold(5000);
	GameInstance->AddRuneForTests(MakeRune(TEXT("phys_common"), ERuneType::PhysAtk, EItemRarity::Common, 0));
	GameInstance->AddRuneForTests(MakeRune(TEXT("gold_mythic"), ERuneType::GoldFind, EItemRarity::Mythic, 0));
	TestTrue(TEXT("Game instance equips rune through service"), GameInstance->TryEquipRune(1, 1));
	GameInstance->AddRuneEssenceForTests(10);

	TestTrue(TEXT("Enhance spends gold and essence when both are available"), GameInstance->TryEnhanceRune(0));
	TestEqual(TEXT("Gold is reduced by rune enhance cost"), GameInstance->GetGold(), static_cast<int64>(4000));
	TestEqual(TEXT("Rune essence is reduced by rune enhance cost"), GameInstance->GetRuneEssence(), static_cast<int64>(0));
	TestFalse(TEXT("Insufficient essence blocks second enhance without spending gold"), GameInstance->TryEnhanceRune(0));
	TestEqual(TEXT("Failed enhance leaves gold unchanged"), GameInstance->GetGold(), static_cast<int64>(4000));

	TestEqual(TEXT("Equipped gold find bonus proxies util value"), GameInstance->GetRuneGoldFindBonus(), FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::Mythic, 0));
	TestEqual(TEXT("No exp rune keeps bonus zero"), GameInstance->GetRuneExpBoostBonus(), 0.0f);

	UIdleSaveGame* CapturedSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture writes rune state"), GameInstance->CaptureToSave(CapturedSave));
	TestEqual(TEXT("Rune save bumps version to eight"), CapturedSave->SaveVersion, 8);
	TestEqual(TEXT("Captured rune count round trips"), CapturedSave->Runes.Num(), 2);
	TestEqual(TEXT("Captured rune codex has sixty three cells"), CapturedSave->RuneCodex.Num(), FRuneCodexFormula::TotalCells);
	TestEqual(TEXT("Captured rune essence round trips"), CapturedSave->RuneEssence, GameInstance->GetRuneEssence());
	TestEqual(TEXT("Captured equipped rune slots have seven entries"), CapturedSave->EquippedRuneSlots.Num(), FRuneFormula::RuneSlotCount);

	UIdleGameInstance* RestoredGameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply accepts captured rune save"), RestoredGameInstance->ApplyFromSave(CapturedSave));
	TestEqual(TEXT("Restored rune essence round trips"), RestoredGameInstance->GetRuneEssence(), GameInstance->GetRuneEssence());
	TestEqual(TEXT("Restored equipped gold find bonus round trips"), RestoredGameInstance->GetRuneGoldFindBonus(), GameInstance->GetRuneGoldFindBonus());

	UIdleSaveGame* LegacyV3Save = NewObject<UIdleSaveGame>();
	LegacyV3Save->bHasSave = true;
	LegacyV3Save->SaveVersion = 3;
	LegacyV3Save->RuneEssence = 123;
	LegacyV3Save->Runes = CapturedSave->Runes;
	LegacyV3Save->EquippedRuneSlots = CapturedSave->EquippedRuneSlots;
	TestTrue(TEXT("Apply accepts legacy v3 rune save without codex field"), RestoredGameInstance->ApplyFromSave(LegacyV3Save));
	TestEqual(TEXT("Legacy v3 save keeps rune essence"), RestoredGameInstance->GetRuneEssence(), static_cast<int64>(123));
	TestEqual(TEXT("Legacy v3 save starts with empty rune codex bonus"), RestoredGameInstance->GetRuneService()->GetCodexBonus().CoreStatAdd, 0.0f);

	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 2;
	LegacySave->RuneEssence = 999;
	LegacySave->Runes.Add(CapturedSave->Runes[0]);
	TestTrue(TEXT("Apply accepts legacy v2 save"), RestoredGameInstance->ApplyFromSave(LegacySave));
	TestEqual(TEXT("Legacy v2 save ignores rune essence"), RestoredGameInstance->GetRuneEssence(), static_cast<int64>(0));
	TestEqual(TEXT("Legacy v2 save restores empty rune bonus"), RestoredGameInstance->GetRuneGoldFindBonus(), 0.0f);

	return true;
}

#endif
