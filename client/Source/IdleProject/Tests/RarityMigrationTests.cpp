#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "ItemSystem/RarityMigration.h"
#include "RuneSystem/RuneCodexFormula.h"
#include "RuneSystem/RuneFormula.h"
#include "RuneSystem/RuneService.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeLegacyItem(FName ItemId, EItemRarity Rarity)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = EItemSlot::Weapon;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = 1.0f;
	return Item;
}

FRuneSaveEntry MakeLegacyRune(FName RuneId, ERuneType Type, EItemRarity Rarity)
{
	FRuneSaveEntry Rune;
	Rune.RuneId = RuneId;
	Rune.RuneType = Type;
	Rune.Rarity = Rarity;
	return Rune;
}

FRuneCodexEntry MakeLegacyCodexEntry(ERuneType Type, EItemRarity Rarity)
{
	FRuneCodexEntry Entry;
	Entry.RuneType = Type;
	Entry.Rarity = Rarity;
	Entry.bUnlocked = true;
	return Entry;
}

bool IsCodexUnlocked(const URuneService& RuneService, ERuneType Type, EItemRarity Rarity)
{
	for (const FRuneCodexEntry& Entry : RuneService.GetOwnedCodex())
	{
		if (Entry.RuneType == Type && Entry.Rarity == Rarity)
		{
			return Entry.bUnlocked;
		}
	}
	return false;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRarityMigrationLegacyValueTest,
	"IdleProject.Item.RarityMigration.LegacyValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRarityMigrationLegacyValueTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Legacy Common remains Common"), FRarityMigration::MigrateLegacy(1), EItemRarity::Common);
	TestEqual(TEXT("Legacy value two is promoted to Rare"), FRarityMigration::MigrateLegacy(2), EItemRarity::Rare);
	TestEqual(TEXT("Legacy Rare remains new Rare"), FRarityMigration::MigrateLegacy(3), EItemRarity::Rare);
	TestEqual(TEXT("Legacy Epic maps to new Epic"), FRarityMigration::MigrateLegacy(4), EItemRarity::Epic);
	TestEqual(TEXT("Legacy Legendary keeps value five"), FRarityMigration::MigrateLegacy(5), EItemRarity::Legendary);
	TestEqual(TEXT("Legacy Mythic maps to new Mythic seven"), FRarityMigration::MigrateLegacy(6), EItemRarity::Mythic);
	TestEqual(TEXT("Legacy zero maps to None"), FRarityMigration::MigrateLegacy(0), EItemRarity::None);
	TestEqual(TEXT("Out of range legacy value maps to None"), FRarityMigration::MigrateLegacy(8), EItemRarity::None);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRarityMigrationApplyFromSaveTest,
	"IdleProject.Item.RarityMigration.ApplyFromSaveV6ToV7",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRarityMigrationApplyFromSaveTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 6;
	const EItemRarity ExpectedMigratedRarities[] = {
		EItemRarity::Common,
		EItemRarity::Rare,
		EItemRarity::Rare,
		EItemRarity::Epic,
		EItemRarity::Legendary,
		EItemRarity::Mythic,
		EItemRarity::None,
		EItemRarity::None,
	};
	const int32 LegacyValues[] = {1, 2, 3, 4, 5, 6, 0, 8};
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(LegacyValues); ++Index)
	{
		const EItemRarity LegacyRarity = static_cast<EItemRarity>(LegacyValues[Index]);
		LegacySave->InventoryItems.Add(MakeLegacyItem(*FString::Printf(TEXT("legacy_item_%d"), LegacyValues[Index]), LegacyRarity));
		LegacySave->Runes.Add(MakeLegacyRune(*FString::Printf(TEXT("legacy_rune_%d"), LegacyValues[Index]), ERuneType::PhysAtk, LegacyRarity));
	}

	FRuneCodexEntry LegacyValueTwoCodex;
	LegacyValueTwoCodex.RuneType = ERuneType::PhysAtk;
	LegacyValueTwoCodex.Rarity = static_cast<EItemRarity>(2);
	LegacyValueTwoCodex.bUnlocked = true;
	LegacySave->RuneCodex.Add(LegacyValueTwoCodex);

	FRuneCodexEntry LegacyRareCodex;
	LegacyRareCodex.RuneType = ERuneType::MagicAtk;
	LegacyRareCodex.Rarity = static_cast<EItemRarity>(3);
	LegacyRareCodex.bUnlocked = true;
	LegacySave->RuneCodex.Add(LegacyRareCodex);

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply accepts legacy v6 save"), GameInstance->ApplyFromSave(LegacySave));

	UIdleSaveGame* Captured = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture writes migrated save"), GameInstance->CaptureToSave(Captured));
	TestEqual(TEXT("Captured save is v7"), Captured->SaveVersion, 7);
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ExpectedMigratedRarities); ++Index)
	{
		TestEqual(
			*FString::Printf(TEXT("Legacy item value %d migrates to expected rarity"), LegacyValues[Index]),
			Captured->InventoryItems[Index].Rarity,
			ExpectedMigratedRarities[Index]);
	}
	TestEqual(TEXT("Invalid legacy item rarities are dropped by rune restore only"), Captured->InventoryItems.Num(), static_cast<int32>(UE_ARRAY_COUNT(ExpectedMigratedRarities)));
	TestEqual(TEXT("Only valid migrated runes survive restore"), Captured->Runes.Num(), 6);
	for (const FRuneSaveEntry& Rune : Captured->Runes)
	{
		TestTrue(TEXT("Migrated rune rarity is in the v7 active range"), Rune.Rarity >= EItemRarity::Common && Rune.Rarity <= EItemRarity::Mythic);
	}
	TestEqual(TEXT("Migrated codex expands to sixty three cells"), Captured->RuneCodex.Num(), FRuneCodexFormula::TotalCells);

	URuneService* RuneService = GameInstance->GetRuneService();
	TestNotNull(TEXT("Rune service is available"), RuneService);
	if (RuneService)
	{
		TestEqual(TEXT("Legacy value two and three cells migrate to new Rare cells"), RuneService->GetCodexCompletion().UnlockedCells, 2);
		TestTrue(TEXT("Legacy grade two codex unlocks new Rare row"), IsCodexUnlocked(*RuneService, ERuneType::PhysAtk, EItemRarity::Rare));
		TestTrue(TEXT("Legacy grade three codex unlocks new Rare row"), IsCodexUnlocked(*RuneService, ERuneType::MagicAtk, EItemRarity::Rare));
		TestFalse(TEXT("New Unique row starts locked after legacy migration"), IsCodexUnlocked(*RuneService, ERuneType::PhysAtk, EItemRarity::Unique));
		TestFalse(TEXT("New Transcendent row starts locked after legacy migration"), IsCodexUnlocked(*RuneService, ERuneType::PhysAtk, EItemRarity::Transcendent));
	}

	UIdleSaveGame* CurrentSave = NewObject<UIdleSaveGame>();
	CurrentSave->bHasSave = true;
	CurrentSave->SaveVersion = 7;
	CurrentSave->InventoryItems.Add(MakeLegacyItem(TEXT("current_transcendent"), EItemRarity::Transcendent));
	TestTrue(TEXT("Apply accepts current v7 save"), GameInstance->ApplyFromSave(CurrentSave));
	TestTrue(TEXT("Capture after v7 apply succeeds"), GameInstance->CaptureToSave(Captured));
	TestEqual(TEXT("V7 Transcendent is not migrated twice"), Captured->InventoryItems[0].Rarity, EItemRarity::Transcendent);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRarityMigrationRuneCodexGridTest,
	"IdleProject.Item.RarityMigration.RuneCodexLegacy54To63",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRarityMigrationRuneCodexGridTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 6;
	for (int32 TypeValue = static_cast<int32>(ERuneType::PhysAtk); TypeValue <= static_cast<int32>(ERuneType::OfflineEff); ++TypeValue)
	{
		const ERuneType Type = static_cast<ERuneType>(TypeValue);
		LegacySave->RuneCodex.Add(MakeLegacyCodexEntry(Type, static_cast<EItemRarity>(2)));
		LegacySave->RuneCodex.Add(MakeLegacyCodexEntry(Type, static_cast<EItemRarity>(3)));
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply accepts legacy codex grid"), GameInstance->ApplyFromSave(LegacySave));
	URuneService* RuneService = GameInstance->GetRuneService();
	TestNotNull(TEXT("Rune service is available"), RuneService);
	if (!RuneService)
	{
		return false;
	}

	const FRuneCodexCompletion Completion = RuneService->GetCodexCompletion();
	TestEqual(TEXT("Migrated codex keeps sixty three cells"), RuneService->GetOwnedCodex().Num(), FRuneCodexFormula::TotalCells);
	TestEqual(TEXT("Legacy grade two and three merge into nine Rare unlocks"), Completion.UnlockedCells, 9);
	TestTrue(TEXT("Rare row is complete after merge"), Completion.RowComplete[static_cast<int32>(EItemRarity::Rare) - 1]);
	TestFalse(TEXT("Unique row is locked after 54 to 63 migration"), Completion.RowComplete[static_cast<int32>(EItemRarity::Unique) - 1]);
	TestFalse(TEXT("Transcendent row is locked after 54 to 63 migration"), Completion.RowComplete[static_cast<int32>(EItemRarity::Transcendent) - 1]);
	TestEqual(TEXT("Core codex dimension remains thirty five"), FRuneCodexFormula::CoreCategoryCells, 35);
	TestEqual(TEXT("Util codex dimension remains twenty eight"), FRuneCodexFormula::UtilCategoryCells, 28);

	return true;
}

#endif
