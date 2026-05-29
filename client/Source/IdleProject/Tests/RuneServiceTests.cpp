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
	TestEqual(TEXT("Rune save uses current version (21)"), CapturedSave->SaveVersion, 21);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneServiceRerollUpgradeTransferTest,
	"IdleProject.Rune.Service.RerollUpgradeTransfer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneServiceRerollUpgradeTransferTest::RunTest(const FString& Parameters)
{
	// 세트 리롤: 세트가 Offense~Fortune 범위로 설정된다(현재와 같아도 성공).
	URuneService* RuneService = NewObject<URuneService>();
	RuneService->AddRune(MakeRune(TEXT("phys_rare"), ERuneType::PhysAtk, EItemRarity::Rare, 0));
	FRandomStream RerollStream(777);
	TestTrue(TEXT("Reroll succeeds on a valid owned index"), RuneService->RerollRuneSet(0, RerollStream));
	const ERuneSet RolledSet = RuneService->GetOwnedRunes()[0].RuneSet;
	TestTrue(TEXT("Rerolled set is within Offense..Fortune"), RolledSet >= ERuneSet::Offense && RolledSet <= ERuneSet::Fortune);
	TestFalse(TEXT("Reroll on invalid index fails"), RuneService->RerollRuneSet(99, RerollStream));

	// 등급 상승: chance=1 강제 성공 → +1, chance=0 강제 실패 → 불변.
	bool bUpgraded = false;
	FRandomStream UpgradeStream(123);
	TestTrue(TEXT("Forced-success upgrade attempt succeeds"), RuneService->TryUpgradeRuneRarity(0, 1.0f, UpgradeStream, bUpgraded));
	TestTrue(TEXT("Forced-success upgrade sets success flag"), bUpgraded);
	TestEqual(TEXT("Forced-success upgrade raises rarity by one (Rare->Epic)"), RuneService->GetOwnedRunes()[0].Rarity, EItemRarity::Epic);

	TestTrue(TEXT("Forced-failure upgrade attempt is still a valid attempt"), RuneService->TryUpgradeRuneRarity(0, 0.0f, UpgradeStream, bUpgraded));
	TestFalse(TEXT("Forced-failure upgrade clears success flag"), bUpgraded);
	TestEqual(TEXT("Forced-failure upgrade leaves rarity unchanged"), RuneService->GetOwnedRunes()[0].Rarity, EItemRarity::Epic);

	// Mythic 상한: 시도 불성립.
	RuneService->AddRune(MakeRune(TEXT("phys_mythic"), ERuneType::PhysAtk, EItemRarity::Mythic, 0));
	TestFalse(TEXT("Mythic rune cannot be upgraded"), RuneService->TryUpgradeRuneRarity(1, 1.0f, UpgradeStream, bUpgraded));
	TestFalse(TEXT("Mythic upgrade rejection keeps success flag false"), bUpgraded);
	TestEqual(TEXT("Mythic rune stays Mythic"), RuneService->GetOwnedRunes()[1].Rarity, EItemRarity::Mythic);

	// 전송: Dst = max(Dst, Src), Src 삭제, 길이 감소, 장착 인덱스 정합.
	URuneService* TransferService = NewObject<URuneService>();
	TransferService->AddRune(MakeRune(TEXT("low"), ERuneType::PhysAtk, EItemRarity::Rare, 2));   // index 0 (src)
	TransferService->AddRune(MakeRune(TEXT("high"), ERuneType::MagicAtk, EItemRarity::Rare, 8));  // index 1 (dst)
	TransferService->AddRune(MakeRune(TEXT("tail"), ERuneType::Hp, EItemRarity::Rare, 0));        // index 2
	TestTrue(TEXT("Equip dst before transfer"), TransferService->TryEquipRune(0, 1));
	TestTrue(TEXT("Equip a rune after src before transfer"), TransferService->TryEquipRune(1, 2));

	TestFalse(TEXT("Transfer to same index fails"), TransferService->TransferEnhancement(0, 0));
	TestFalse(TEXT("Transfer with invalid index fails"), TransferService->TransferEnhancement(0, 99));

	TestTrue(TEXT("Transfer src->dst succeeds"), TransferService->TransferEnhancement(0, 1));
	TestEqual(TEXT("Source rune removed reduces owned count"), TransferService->GetOwnedRunes().Num(), 2);
	// src(idx0) 삭제 후: 이전 dst(idx1)->0, tail(idx2)->1.
	TestEqual(TEXT("Dst level becomes max(dst, src)"), TransferService->GetOwnedRunes()[0].EnhanceLevel, 8);
	TestEqual(TEXT("Equipped slot tracking dst shifts down by one"), TransferService->GetEquippedOwnedIndex(0), 0);
	TestEqual(TEXT("Equipped slot tracking post-src rune shifts down by one"), TransferService->GetEquippedOwnedIndex(1), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneFormulaRerollUpgradeTransferParityTest,
	"IdleProject.Rune.Formula.RerollUpgradeTransferParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneFormulaRerollUpgradeTransferParityTest::RunTest(const FString& Parameters)
{
	// 서버 rune.ts 표값 1:1 앵커(Common..Mythic = 1..7).
	TestEqual(TEXT("Reroll essence cost Common"), FRuneFormula::GetRerollSetEssenceCost(EItemRarity::Common), static_cast<int64>(20));
	TestEqual(TEXT("Reroll essence cost Rare"), FRuneFormula::GetRerollSetEssenceCost(EItemRarity::Rare), static_cast<int64>(50));
	TestEqual(TEXT("Reroll essence cost Mythic"), FRuneFormula::GetRerollSetEssenceCost(EItemRarity::Mythic), static_cast<int64>(1500));

	TestEqual(TEXT("Upgrade essence cost Common"), FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity::Common), static_cast<int64>(100));
	TestEqual(TEXT("Upgrade essence cost Transcendent"), FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity::Transcendent), static_cast<int64>(10000));
	TestEqual(TEXT("Upgrade essence cost Mythic guarded to zero"), FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity::Mythic), static_cast<int64>(0));

	TestEqual(TEXT("Upgrade gold cost Common"), FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity::Common), static_cast<int64>(5000));
	TestEqual(TEXT("Upgrade gold cost Transcendent"), FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity::Transcendent), static_cast<int64>(1500000));
	TestEqual(TEXT("Upgrade gold cost Mythic guarded to zero"), FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity::Mythic), static_cast<int64>(0));

	TestEqual(TEXT("Upgrade chance Common (float parity)"), FRuneFormula::GetRarityUpgradeChance(EItemRarity::Common), 0.6f);
	TestEqual(TEXT("Upgrade chance Legendary (float parity)"), FRuneFormula::GetRarityUpgradeChance(EItemRarity::Legendary), 0.12f);
	TestEqual(TEXT("Upgrade chance Transcendent (float parity)"), FRuneFormula::GetRarityUpgradeChance(EItemRarity::Transcendent), 0.05f);
	TestEqual(TEXT("Upgrade chance Mythic guarded to zero"), FRuneFormula::GetRarityUpgradeChance(EItemRarity::Mythic), 0.0f);

	TestEqual(TEXT("Transfer cost at level zero"), FRuneFormula::GetTransferEssenceCost(0), static_cast<int64>(50));
	TestEqual(TEXT("Transfer cost at level ten"), FRuneFormula::GetTransferEssenceCost(10), static_cast<int64>(300));
	TestEqual(TEXT("Transfer cost clamps negative level to base"), FRuneFormula::GetTransferEssenceCost(-5), static_cast<int64>(50));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneGameInstanceRerollUpgradeTransferTest,
	"IdleProject.Rune.GameInstance.RerollUpgradeTransfer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneGameInstanceRerollUpgradeTransferTest::RunTest(const FString& Parameters)
{
	// 세트 리롤: 정수 충분 시 차감, 부족 시 거부.
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeRuneServiceForTests();
	GameInstance->AddRuneForTests(MakeRune(TEXT("phys_rare"), ERuneType::PhysAtk, EItemRarity::Rare, 0));

	TestFalse(TEXT("Reroll rejected without essence"), GameInstance->TryRerollRuneSet(0));
	GameInstance->AddRuneEssenceForTests(FRuneFormula::GetRerollSetEssenceCost(EItemRarity::Rare));
	TestTrue(TEXT("Reroll succeeds when essence is sufficient"), GameInstance->TryRerollRuneSet(0));
	TestEqual(TEXT("Reroll deducts essence exactly once"), GameInstance->GetRuneEssence(), static_cast<int64>(0));

	// 등급 상승: 자원 부족 거부, 자원 충분 시 시도(단일 차감).
	bool bUpgraded = false;
	TestFalse(TEXT("Upgrade rejected without resources"), GameInstance->TryUpgradeRuneRarity(0, bUpgraded));
	GameInstance->AddRuneEssenceForTests(FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity::Rare));
	GameInstance->AddGold(FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity::Rare));
	const int64 EssenceBeforeUpgrade = GameInstance->GetRuneEssence();
	const int64 GoldBeforeUpgrade = GameInstance->GetGold();
	TestTrue(TEXT("Upgrade attempt succeeds with resources"), GameInstance->TryUpgradeRuneRarity(0, bUpgraded));
	TestEqual(TEXT("Upgrade deducts essence once"), GameInstance->GetRuneEssence(), EssenceBeforeUpgrade - FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity::Rare));
	TestEqual(TEXT("Upgrade deducts gold once"), GameInstance->GetGold(), GoldBeforeUpgrade - FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity::Rare));

	// Mythic 거부(자원 무차감).
	GameInstance->AddRuneForTests(MakeRune(TEXT("mythic"), ERuneType::PhysAtk, EItemRarity::Mythic, 0));
	GameInstance->AddRuneEssenceForTests(1000000);
	GameInstance->AddGold(10000000);
	const int64 EssenceBeforeMythic = GameInstance->GetRuneEssence();
	const int64 GoldBeforeMythic = GameInstance->GetGold();
	TestFalse(TEXT("Mythic rune upgrade rejected at game instance"), GameInstance->TryUpgradeRuneRarity(1, bUpgraded));
	TestEqual(TEXT("Rejected Mythic upgrade does not spend essence"), GameInstance->GetRuneEssence(), EssenceBeforeMythic);
	TestEqual(TEXT("Rejected Mythic upgrade does not spend gold"), GameInstance->GetGold(), GoldBeforeMythic);

	// 전송: 정수 차감·source 삭제·길이 감소.
	UIdleGameInstance* TransferInstance = NewObject<UIdleGameInstance>();
	TransferInstance->InitializeRuneServiceForTests();
	TransferInstance->AddRuneForTests(MakeRune(TEXT("src"), ERuneType::PhysAtk, EItemRarity::Rare, 3));
	TransferInstance->AddRuneForTests(MakeRune(TEXT("dst"), ERuneType::MagicAtk, EItemRarity::Rare, 1));

	TestFalse(TEXT("Transfer rejected without essence"), TransferInstance->TransferRuneEnhancement(0, 1));
	const int64 TransferCost = FRuneFormula::GetTransferEssenceCost(3);
	TransferInstance->AddRuneEssenceForTests(TransferCost);
	TestTrue(TEXT("Transfer succeeds when essence is sufficient"), TransferInstance->TransferRuneEnhancement(0, 1));
	TestEqual(TEXT("Transfer deducts essence exactly once"), TransferInstance->GetRuneEssence(), static_cast<int64>(0));
	TestEqual(TEXT("Transfer removes source rune"), TransferInstance->GetRuneService()->GetOwnedRunes().Num(), 1);
	TestEqual(TEXT("Transferred dst takes max enhance level"), TransferInstance->GetRuneService()->GetOwnedRunes()[0].EnhanceLevel, 3);

	return true;
}

#endif
