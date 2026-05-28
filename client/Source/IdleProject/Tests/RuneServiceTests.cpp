#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
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
	TestEqual(TEXT("AddRune increases owned count"), RuneService->GetOwnedRunes().Num(), 2);

	TestTrue(TEXT("Core rune equips into slot zero"), RuneService->TryEquipRune(0, 0));
	TestTrue(TEXT("Util rune equips into slot one"), RuneService->TryEquipRune(1, 1));
	TestEqual(TEXT("Slot zero points at first owned rune"), RuneService->GetEquippedOwnedIndex(0), 0);
	TestEqual(TEXT("Slot one points at second owned rune"), RuneService->GetEquippedOwnedIndex(1), 1);

	const FRuneCoreMultipliers Core = RuneService->GetEquippedCoreMultipliers();
	const FRuneUtilValues Util = RuneService->GetEquippedUtilValues();
	TestEqual(TEXT("Equipped phys attack rune adds one plus formula multiplier"), Core.PhysAtk, 1.0f + FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 10));
	TestEqual(TEXT("Other core stats remain neutral"), Core.MagicAtk, 1.0f);
	TestEqual(TEXT("Equipped gold find uses util formula"), Util.GoldFind, FRuneFormula::GetUtilRuneValue(ERuneType::GoldFind, EItemRarity::Mythic, 0));

	TestTrue(TEXT("Unequip clears slot"), RuneService->UnequipRune(0));
	TestEqual(TEXT("Unequipped slot returns INDEX_NONE"), RuneService->GetEquippedOwnedIndex(0), INDEX_NONE);
	TestFalse(TEXT("Invalid slot cannot equip"), RuneService->TryEquipRune(6, 0));
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
	TestEqual(TEXT("Captured slot array always has six entries"), CapturedSlots.Num(), FRuneFormula::RuneSlotCount);

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
	TestEqual(TEXT("Rune save bumps version to three"), CapturedSave->SaveVersion, 3);
	TestEqual(TEXT("Captured rune count round trips"), CapturedSave->Runes.Num(), 2);
	TestEqual(TEXT("Captured rune essence round trips"), CapturedSave->RuneEssence, GameInstance->GetRuneEssence());
	TestEqual(TEXT("Captured equipped rune slots have six entries"), CapturedSave->EquippedRuneSlots.Num(), FRuneFormula::RuneSlotCount);

	UIdleGameInstance* RestoredGameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply accepts captured rune save"), RestoredGameInstance->ApplyFromSave(CapturedSave));
	TestEqual(TEXT("Restored rune essence round trips"), RestoredGameInstance->GetRuneEssence(), GameInstance->GetRuneEssence());
	TestEqual(TEXT("Restored equipped gold find bonus round trips"), RestoredGameInstance->GetRuneGoldFindBonus(), GameInstance->GetRuneGoldFindBonus());

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
