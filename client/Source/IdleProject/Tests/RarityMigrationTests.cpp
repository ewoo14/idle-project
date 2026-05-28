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
	LegacySave->InventoryItems.Add(MakeLegacyItem(TEXT("legacy_uncommon_sword"), static_cast<EItemRarity>(2)));
	LegacySave->InventoryItems.Add(MakeLegacyItem(TEXT("legacy_mythic_sword"), static_cast<EItemRarity>(6)));
	LegacySave->Runes.Add(MakeLegacyRune(TEXT("legacy_uncommon_rune"), ERuneType::PhysAtk, static_cast<EItemRarity>(2)));
	LegacySave->Runes.Add(MakeLegacyRune(TEXT("legacy_mythic_rune"), ERuneType::GoldFind, static_cast<EItemRarity>(6)));

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
	TestEqual(TEXT("Legacy item value two migrated to Rare"), Captured->InventoryItems[0].Rarity, EItemRarity::Rare);
	TestEqual(TEXT("Legacy item Mythic migrated to Mythic seven"), Captured->InventoryItems[1].Rarity, EItemRarity::Mythic);
	TestEqual(TEXT("Legacy rune value two migrated to Rare"), Captured->Runes[0].Rarity, EItemRarity::Rare);
	TestEqual(TEXT("Legacy rune Mythic migrated to Mythic seven"), Captured->Runes[1].Rarity, EItemRarity::Mythic);
	TestEqual(TEXT("Migrated codex expands to sixty three cells"), Captured->RuneCodex.Num(), FRuneCodexFormula::TotalCells);

	URuneService* RuneService = GameInstance->GetRuneService();
	TestNotNull(TEXT("Rune service is available"), RuneService);
	if (RuneService)
	{
		TestEqual(TEXT("Legacy value two and three cells migrate to new Rare cells"), RuneService->GetCodexCompletion().UnlockedCells, 2);
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

#endif
