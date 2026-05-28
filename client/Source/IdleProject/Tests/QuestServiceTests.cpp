#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/QuestService.h"
#include "GameCore/StageService.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemTypes.h"
#include "ItemSystem/ShopFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeEnhanceTestItem(FName ItemId, EItemSlot Slot, int32 EnhanceLevel = 0, EItemRarity Rarity = EItemRarity::Rare)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = 10.0f;
	Item.EnhanceLevel = EnhanceLevel;
	return Item;
}

}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceGearRollPurchaseTest,
	"IdleProject.GameCore.IdleGameInstance.GearRollPurchase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceGearRollPurchaseTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeStageServiceForTests();
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();

	const UStageService* StageService = GameInstance->GetStageService();
	const int64 Cost = FShopFormula::GetGearRollCost(StageService ? StageService->GetGlobalStageIndex() : 1);

	const FShopPurchaseResult MissingInventory = GameInstance->TryBuyGearRoll(nullptr);
	TestFalse(TEXT("Missing inventory does not purchase"), MissingInventory.bPurchased);
	TestEqual(TEXT("Missing inventory spends nothing"), MissingInventory.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Missing inventory leaves gold unchanged"), GameInstance->GetGold(), static_cast<int64>(0));

	GameInstance->AddGold(Cost - 1);
	const FShopPurchaseResult NoGold = GameInstance->TryBuyGearRoll(Inventory);
	TestFalse(TEXT("Insufficient gold does not purchase"), NoGold.bPurchased);
	TestEqual(TEXT("Insufficient gold spends nothing"), NoGold.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Insufficient gold keeps balance"), GameInstance->GetGold(), Cost - 1);
	TestEqual(TEXT("Insufficient gold adds no item"), Inventory->GetItemCount(), 0);

	GameInstance->AddGold(1);
	const FShopPurchaseResult Success = GameInstance->TryBuyGearRoll(Inventory);
	TestTrue(TEXT("Enough gold purchases a gear roll"), Success.bPurchased);
	TestEqual(TEXT("Purchase spends exactly one roll cost"), Success.GoldSpent, Cost);
	TestEqual(TEXT("Purchase deducts gold once"), GameInstance->GetGold(), static_cast<int64>(0));
	TestEqual(TEXT("Purchase adds one item"), Inventory->GetItemCount(), 1);
	TestTrue(TEXT("Purchase returns guaranteed rarity"), Success.Rarity >= EItemRarity::Common);
	TestTrue(TEXT("Purchase returns equipment slot"), Success.Slot >= EItemSlot::Weapon && Success.Slot <= EItemSlot::Accessory);
	TestFalse(TEXT("Purchase returns item name"), Success.ItemName.IsEmpty());

	const FItemInstance* Equipped = Inventory->GetEquippedItem(Success.Slot);
	TestNotNull(TEXT("Purchased item is present through auto equip"), Equipped);
	if (Equipped)
	{
		TestEqual(TEXT("Result rarity matches inventory item"), Equipped->Rarity, Success.Rarity);
		TestEqual(TEXT("Result name matches inventory item"), Equipped->DisplayName.ToString(), Success.ItemName.ToString());
	}

	UIdleGameInstance* FullInventoryGameInstance = NewObject<UIdleGameInstance>();
	FullInventoryGameInstance->InitializeStageServiceForTests();
	UInventoryComponent* FullInventory = NewObject<UInventoryComponent>();
	for (int32 Index = 0; Index < 100; ++Index)
	{
		const FName ItemId(*FString::Printf(TEXT("full_inventory_sword_%d"), Index));
		TestTrue(TEXT("Full inventory setup accepts item"), FullInventory->AddItem(MakeEnhanceTestItem(ItemId, EItemSlot::Weapon)));
	}

	FullInventoryGameInstance->AddGold(Cost);
	const FShopPurchaseResult FullInventoryResult = FullInventoryGameInstance->TryBuyGearRoll(FullInventory);
	TestFalse(TEXT("Full inventory does not purchase"), FullInventoryResult.bPurchased);
	TestEqual(TEXT("Full inventory spends nothing"), FullInventoryResult.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Full inventory keeps gold balance"), FullInventoryGameInstance->GetGold(), Cost);
	TestEqual(TEXT("Full inventory does not add item"), FullInventory->GetItemCount(), 100);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceMaterialShopPurchaseTest,
	"IdleProject.GameCore.IdleGameInstance.MaterialShopPurchase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceMaterialShopPurchaseTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeStageServiceForTests();

	TestFalse(TEXT("Protection scroll purchase fails with no gold"), GameInstance->TryBuyProtectionScroll());
	TestEqual(TEXT("Failed protection scroll purchase does not grant resource"), GameInstance->GetProtectionScrolls(), static_cast<int64>(0));

	const int64 ProtectionCost = FShopFormula::GetProtectionScrollCost(0);
	GameInstance->AddGold(ProtectionCost);
	TestTrue(TEXT("Protection scroll purchase succeeds with enough gold"), GameInstance->TryBuyProtectionScroll());
	TestEqual(TEXT("Protection scroll purchase deducts gold once"), GameInstance->GetGold(), static_cast<int64>(0));
	TestEqual(TEXT("Protection scroll purchase grants one scroll"), GameInstance->GetProtectionScrolls(), static_cast<int64>(1));

	const int64 ResetCost = FShopFormula::GetResetCubeCost(0);
	GameInstance->AddGold(ResetCost - 1);
	TestFalse(TEXT("Reset cube purchase fails below cost"), GameInstance->TryBuyResetCube());
	TestEqual(TEXT("Failed reset cube purchase keeps balance"), GameInstance->GetGold(), ResetCost - 1);
	TestEqual(TEXT("Failed reset cube purchase grants no cube"), GameInstance->GetResetCubes(), static_cast<int64>(0));
	GameInstance->AddGold(1);
	TestTrue(TEXT("Reset cube purchase succeeds at exact cost"), GameInstance->TryBuyResetCube());
	TestEqual(TEXT("Reset cube purchase grants one cube"), GameInstance->GetResetCubes(), static_cast<int64>(1));

	const int64 RankCost = FShopFormula::GetRankCubeCost(0);
	GameInstance->AddGold(RankCost);
	TestTrue(TEXT("Rank cube purchase succeeds at exact cost"), GameInstance->TryBuyRankCube());
	TestEqual(TEXT("Rank cube purchase deducts gold once"), GameInstance->GetGold(), static_cast<int64>(0));
	TestEqual(TEXT("Rank cube purchase grants one cube"), GameInstance->GetRankCubes(), static_cast<int64>(1));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceEnhanceAttemptTest,
	"IdleProject.GameCore.IdleGameInstance.EnhanceAttempt",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceEnhanceAttemptTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeQuestServiceForTests(TEXT("2026-05-26"));
	GameInstance->SetEnhanceRandomSeed(1);

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeEnhanceTestItem(TEXT("rare_sword"), EItemSlot::Weapon));

	const FEnhanceAttemptResult InvalidSlot = GameInstance->TryEnhanceEquipped(EItemSlot::None, Inventory);
	TestFalse(TEXT("Invalid slot does not attempt enhance"), InvalidSlot.bAttempted);
	TestEqual(TEXT("Invalid slot spends nothing"), InvalidSlot.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Invalid slot returns no new level"), InvalidSlot.NewLevel, INDEX_NONE);

	const FEnhanceAttemptResult EmptySlot = GameInstance->TryEnhanceEquipped(EItemSlot::Helmet, Inventory);
	TestFalse(TEXT("Empty equipped slot does not attempt enhance"), EmptySlot.bAttempted);
	TestEqual(TEXT("Empty equipped slot spends nothing"), EmptySlot.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Empty equipped slot reports missing level"), EmptySlot.NewLevel, INDEX_NONE);

	const FEnhanceAttemptResult NoGold = GameInstance->TryEnhanceEquipped(EItemSlot::Weapon, Inventory);
	TestFalse(TEXT("Insufficient gold does not attempt enhance"), NoGold.bAttempted);
	TestEqual(TEXT("Insufficient gold spends nothing"), NoGold.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Insufficient gold keeps level"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 0);

	UIdleGameInstance* CommonGameInstance = NewObject<UIdleGameInstance>();
	CommonGameInstance->SetEnhanceRandomSeed(1);
	UInventoryComponent* CommonInventory = NewObject<UInventoryComponent>();
	CommonInventory->AddItem(MakeEnhanceTestItem(TEXT("common_sword"), EItemSlot::Weapon, 0, EItemRarity::Common));
	CommonGameInstance->AddGold(FEnhanceFormula::GetEnhanceCost(0));
	const FEnhanceAttemptResult CommonAttempt = CommonGameInstance->TryEnhanceEquipped(EItemSlot::Weapon, CommonInventory);
	TestTrue(TEXT("Common item keeps legacy single-argument cost gate"), CommonAttempt.bAttempted);
	TestEqual(TEXT("Common item spends legacy level 0 cost once"), CommonAttempt.GoldSpent, FEnhanceFormula::GetEnhanceCost(0));
	TestEqual(TEXT("Common item deducts only legacy level 0 cost"), CommonGameInstance->GetGold(), static_cast<int64>(0));

	UIdleGameInstance* LegendaryGateGameInstance = NewObject<UIdleGameInstance>();
	UInventoryComponent* LegendaryGateInventory = NewObject<UInventoryComponent>();
	LegendaryGateInventory->AddItem(MakeEnhanceTestItem(TEXT("legendary_sword"), EItemSlot::Weapon, 0, EItemRarity::Legendary));
	LegendaryGateGameInstance->AddGold(FEnhanceFormula::GetEnhanceCost(0));
	const FEnhanceAttemptResult LegendaryNoGold = LegendaryGateGameInstance->TryEnhanceEquipped(EItemSlot::Weapon, LegendaryGateInventory);
	TestFalse(TEXT("Legendary item rejects Common-only gold budget"), LegendaryNoGold.bAttempted);
	TestEqual(TEXT("Legendary item spends nothing below rarity-scaled cost"), LegendaryNoGold.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Legendary gold gate keeps balance unchanged"), LegendaryGateGameInstance->GetGold(), FEnhanceFormula::GetEnhanceCost(0));

	GameInstance->AddGold(FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Rare));
	const FEnhanceAttemptResult Success = GameInstance->TryEnhanceEquipped(EItemSlot::Weapon, Inventory);
	TestTrue(TEXT("Enough gold attempts enhance"), Success.bAttempted);
	TestTrue(TEXT("Seeded first enhance succeeds"), Success.bSuccess);
	TestEqual(TEXT("Attempt spends rare level 0 cost once"), Success.GoldSpent, static_cast<int64>(200));
	TestEqual(TEXT("Successful enhance returns new level"), Success.NewLevel, 1);
	TestEqual(TEXT("Successful enhance mutates equipped item"), Inventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), 1);
	TestEqual(TEXT("Gold is deducted"), GameInstance->GetGold(), static_cast<int64>(0));

	FQuestState EnhanceDaily;
	TestTrue(TEXT("Enhance daily exists"), GameInstance->GetQuestState(TEXT("daily_enhance_gear"), EnhanceDaily));
	TestEqual(TEXT("Enhance attempt records quest progress"), EnhanceDaily.Progress, 1);

	UInventoryComponent* FailInventory = NewObject<UInventoryComponent>();
	FailInventory->AddItem(MakeEnhanceTestItem(TEXT("rare_axe"), EItemSlot::Weapon, FEnhanceFormula::MaxEnhanceLevel - 1));

	int32 FailingSeed = INDEX_NONE;
	for (int32 Seed = 1; Seed < 1000; ++Seed)
	{
		FRandomStream Probe(Seed);
		if (!FEnhanceFormula::RollEnhanceSuccess(FEnhanceFormula::GetEnhanceSuccessRate(FEnhanceFormula::MaxEnhanceLevel - 1), Probe))
		{
			FailingSeed = Seed;
			break;
		}
	}
	TestTrue(TEXT("A deterministic failure seed is available"), FailingSeed != INDEX_NONE);

	GameInstance->SetEnhanceRandomSeed(FailingSeed);
	GameInstance->AddGold(FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel - 1, EItemRarity::Rare));
	const int64 GoldBeforeFailure = GameInstance->GetGold();
	const FEnhanceAttemptResult Failure = GameInstance->TryEnhanceEquipped(EItemSlot::Weapon, FailInventory);
	TestTrue(TEXT("Failed roll still attempts enhance"), Failure.bAttempted);
	TestFalse(TEXT("Failed roll reports failure"), Failure.bSuccess);
	TestEqual(TEXT("Failed roll spends exactly one rarity-scaled attempt cost"), Failure.GoldSpent, FEnhanceFormula::GetEnhanceCost(FEnhanceFormula::MaxEnhanceLevel - 1, EItemRarity::Rare));
	TestEqual(TEXT("Failed roll deducts gold once"), GameInstance->GetGold(), GoldBeforeFailure - Failure.GoldSpent);
	TestEqual(TEXT("Risk failure downgrades one level"), Failure.NewLevel, FEnhanceFormula::MaxEnhanceLevel - 2);
	TestEqual(TEXT("Risk failure mutates item level"), FailInventory->GetEquippedEnhanceLevel(EItemSlot::Weapon), FEnhanceFormula::MaxEnhanceLevel - 2);

	TestTrue(TEXT("Enhance daily still exists after failed roll"), GameInstance->GetQuestState(TEXT("daily_enhance_gear"), EnhanceDaily));
	TestEqual(TEXT("Failed roll also records quest progress"), EnhanceDaily.Progress, 2);

	for (int32 Level = 1; Level < FEnhanceFormula::MaxEnhanceLevel; ++Level)
	{
		Inventory->EnhanceEquippedItem(EItemSlot::Weapon);
	}

	GameInstance->AddGold(10000);
	const int64 GoldBeforeMaxAttempt = GameInstance->GetGold();
	const FEnhanceAttemptResult AtMax = GameInstance->TryEnhanceEquipped(EItemSlot::Weapon, Inventory);
	TestFalse(TEXT("Max level does not attempt enhance"), AtMax.bAttempted);
	TestEqual(TEXT("Max level spends no gold"), GameInstance->GetGold(), GoldBeforeMaxAttempt);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceProgressClaimTest,
	"IdleProject.GameCore.QuestService.ProgressClaimUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceProgressClaimTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	TestEqual(TEXT("First main quest, daily quests, and weekly quests are active"), Quests->GetActiveQuestStates().Num(), 12);

	Quests->RecordProgress(EQuestObjective::KillMonster, 5);

	FQuestState MainQuest;
	TestTrue(TEXT("Main quest state is available"), Quests->GetQuestState(TEXT("main_ch1_001"), MainQuest));
	TestEqual(TEXT("Kill progress reaches main quest target"), MainQuest.Progress, 5);
	TestTrue(TEXT("Main quest completes at target count"), MainQuest.bCompleted);
	TestFalse(TEXT("Completed quest is not claimed automatically"), MainQuest.bClaimed);

	FQuestClaimResult Claim = Quests->ClaimQuest(TEXT("main_ch1_001"));
	TestTrue(TEXT("Completed quest can be claimed"), Claim.bSuccess);
	TestEqual(TEXT("Claim returns quest reward gold"), Claim.RewardGold, static_cast<int64>(150));
	TestEqual(TEXT("Claim returns quest reward exp"), Claim.RewardExp, static_cast<int64>(80));
	TestTrue(TEXT("Claim unlocks next main quest"), Claim.UnlockedQuestIds.Contains(TEXT("main_ch1_002")));

	FQuestState NextMainQuest;
	TestTrue(TEXT("Unlocked main quest becomes active"), Quests->GetQuestState(TEXT("main_ch1_002"), NextMainQuest));
	TestEqual(TEXT("Unlocked main quest starts at zero progress"), NextMainQuest.Progress, 0);

	FQuestClaimResult DuplicateClaim = Quests->ClaimQuest(TEXT("main_ch1_001"));
	TestFalse(TEXT("Claimed quest cannot be claimed twice"), DuplicateClaim.bSuccess);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceDefinitionParityTest,
	"IdleProject.GameCore.QuestService.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceDefinitionParityTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	const TArray<FQuestDefinition> Definitions = Quests->GetQuestDefinitions();
	TestEqual(TEXT("Quest definition count matches chapter four client contract"), Definitions.Num(), 35);

	struct FExpectedQuestDefinition
	{
		const TCHAR* QuestId;
		EQuestType Type;
		EQuestObjective Objective;
		int32 TargetCount;
		int64 RewardGold;
		int64 RewardExp;
		const TCHAR* PrerequisiteQuestId;
		const TCHAR* ChapterMapId;
	};

	const FExpectedQuestDefinition Expected[] = {
		{TEXT("main_ch1_001"), EQuestType::Main, EQuestObjective::KillMonster, 5, 150, 80, TEXT(""), TEXT("1-1")},
		{TEXT("main_ch1_002"), EQuestType::Main, EQuestObjective::ClearMap, 1, 220, 140, TEXT("main_ch1_001"), TEXT("1-1")},
		{TEXT("main_ch1_003"), EQuestType::Main, EQuestObjective::KillMonster, 12, 420, 300, TEXT("main_ch1_002"), TEXT("1-2")},
		{TEXT("main_ch1_004"), EQuestType::Main, EQuestObjective::ClearMap, 1, 700, 520, TEXT("main_ch1_003"), TEXT("1-3")},
		{TEXT("main_ch1_005"), EQuestType::Main, EQuestObjective::KillMonster, 20, 1200, 900, TEXT("main_ch1_004"), TEXT("1-5")},
		{TEXT("main_ch1_006"), EQuestType::Main, EQuestObjective::Enhance, 2, 1600, 1200, TEXT("main_ch1_005"), TEXT("1-5")},
		{TEXT("main_ch1_007"), EQuestType::Main, static_cast<EQuestObjective>(4), 1, 2200, 1600, TEXT("main_ch1_006"), TEXT("1-5")},
		{TEXT("main_ch2_001"), EQuestType::Main, EQuestObjective::KillMonster, 25, 2600, 1900, TEXT("main_ch1_007"), TEXT("2-1")},
		{TEXT("main_ch2_002"), EQuestType::Main, EQuestObjective::ClearMap, 1, 3200, 2300, TEXT("main_ch2_001"), TEXT("2-2")},
		{TEXT("main_ch2_003"), EQuestType::Main, static_cast<EQuestObjective>(8), 10, 3900, 2800, TEXT("main_ch2_002"), TEXT("2-3")},
		{TEXT("main_ch2_004"), EQuestType::Main, static_cast<EQuestObjective>(5), 1, 4800, 3400, TEXT("main_ch2_003"), TEXT("2-4")},
		{TEXT("main_ch2_005"), EQuestType::Main, static_cast<EQuestObjective>(4), 1, 6200, 4500, TEXT("main_ch2_004"), TEXT("2-5")},
		{TEXT("main_ch3_001"), EQuestType::Main, EQuestObjective::KillMonster, 35, 7600, 5400, TEXT("main_ch2_005"), TEXT("3-1")},
		{TEXT("main_ch3_002"), EQuestType::Main, EQuestObjective::ClearMap, 1, 8800, 6200, TEXT("main_ch3_001"), TEXT("3-2")},
		{TEXT("main_ch3_003"), EQuestType::Main, EQuestObjective::ReachLevel, 25, 10200, 7300, TEXT("main_ch3_002"), TEXT("3-4")},
		{TEXT("main_ch3_004"), EQuestType::Main, EQuestObjective::ClimbTower, 15, 11800, 8400, TEXT("main_ch3_003"), TEXT("3-5")},
		{TEXT("main_ch3_005"), EQuestType::Main, EQuestObjective::KillMonster, 50, 13600, 9800, TEXT("main_ch3_004"), TEXT("3-8")},
		{TEXT("main_ch3_006"), EQuestType::Main, EQuestObjective::DefeatBoss, 1, 16000, 12000, TEXT("main_ch3_005"), TEXT("3-10")},
		{TEXT("main_ch4_001"), EQuestType::Main, EQuestObjective::KillMonster, 65, 18400, 13800, TEXT("main_ch3_006"), TEXT("4-1")},
		{TEXT("main_ch4_002"), EQuestType::Main, EQuestObjective::ClearMap, 1, 21000, 15800, TEXT("main_ch4_001"), TEXT("4-2")},
		{TEXT("main_ch4_003"), EQuestType::Main, EQuestObjective::ReachLevel, 40, 24000, 18000, TEXT("main_ch4_002"), TEXT("4-4")},
		{TEXT("main_ch4_004"), EQuestType::Main, EQuestObjective::ClimbTower, 25, 27500, 20500, TEXT("main_ch4_003"), TEXT("4-5")},
		{TEXT("main_ch4_005"), EQuestType::Main, EQuestObjective::KillMonster, 80, 31500, 23600, TEXT("main_ch4_004"), TEXT("4-8")},
		{TEXT("main_ch4_006"), EQuestType::Main, EQuestObjective::DefeatBoss, 1, 38000, 28500, TEXT("main_ch4_005"), TEXT("4-10")},
		{TEXT("daily_kill_monsters"), EQuestType::Daily, EQuestObjective::KillMonster, 30, 500, 240, TEXT(""), TEXT("")},
		{TEXT("daily_claim_offline"), EQuestType::Daily, EQuestObjective::ClaimOffline, 1, 300, 180, TEXT(""), TEXT("")},
		{TEXT("daily_enhance_gear"), EQuestType::Daily, EQuestObjective::Enhance, 3, 650, 320, TEXT(""), TEXT("")},
		{TEXT("daily_reach_level"), EQuestType::Daily, static_cast<EQuestObjective>(8), 10, 700, 360, TEXT(""), TEXT("")},
		{TEXT("daily_spend_gold"), EQuestType::Daily, static_cast<EQuestObjective>(9), 1000, 750, 380, TEXT(""), TEXT("")},
		{TEXT("daily_roll_gear_shop"), EQuestType::Daily, static_cast<EQuestObjective>(10), 1, 850, 420, TEXT(""), TEXT("")},
		{TEXT("daily_feed_pet"), EQuestType::Daily, static_cast<EQuestObjective>(11), 1, 900, 450, TEXT(""), TEXT("")},
		{TEXT("weekly_defeat_bosses"), static_cast<EQuestType>(2), static_cast<EQuestObjective>(4), 3, 5000, 2500, TEXT(""), TEXT("")},
		{TEXT("weekly_rebirth"), static_cast<EQuestType>(2), static_cast<EQuestObjective>(5), 1, 8000, 4000, TEXT(""), TEXT("")},
		{TEXT("weekly_climb_tower"), static_cast<EQuestType>(2), static_cast<EQuestObjective>(7), 10, 7000, 3600, TEXT(""), TEXT("")},
		{TEXT("weekly_spend_gold"), static_cast<EQuestType>(2), static_cast<EQuestObjective>(9), 10000, 6500, 3200, TEXT(""), TEXT("")},
	};

	for (int32 Index = 0; Index < Definitions.Num() && Index < UE_ARRAY_COUNT(Expected); ++Index)
	{
		const FQuestDefinition& Actual = Definitions[Index];
		const FExpectedQuestDefinition& ExpectedQuest = Expected[Index];
		const FString Prefix = FString::Printf(TEXT("Quest definition %d "), Index);

		TestEqual(*(Prefix + TEXT("quest id")), Actual.QuestId, FString(ExpectedQuest.QuestId));
		TestEqual(*(Prefix + TEXT("type")), Actual.Type, ExpectedQuest.Type);
		TestEqual(*(Prefix + TEXT("objective")), Actual.Objective, ExpectedQuest.Objective);
		TestEqual(*(Prefix + TEXT("target count")), Actual.TargetCount, ExpectedQuest.TargetCount);
		TestEqual(*(Prefix + TEXT("reward gold")), Actual.RewardGold, ExpectedQuest.RewardGold);
		TestEqual(*(Prefix + TEXT("reward exp")), Actual.RewardExp, ExpectedQuest.RewardExp);
		TestEqual(*(Prefix + TEXT("prerequisite")), Actual.PrerequisiteQuestId, FString(ExpectedQuest.PrerequisiteQuestId));
		TestEqual(*(Prefix + TEXT("chapter map")), Actual.ChapterMapId, FString(ExpectedQuest.ChapterMapId));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceChapterFourPrerequisiteChainTest,
	"IdleProject.GameCore.QuestService.ChapterFourPrerequisiteChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceChapterFourPrerequisiteChainTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	const TArray<TPair<EQuestObjective, int32>> MainChainProgress = {
		{EQuestObjective::KillMonster, 5},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 12},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 20},
		{EQuestObjective::Enhance, 2},
		{EQuestObjective::DefeatBoss, 1},
		{EQuestObjective::KillMonster, 25},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::ReachLevel, 10},
		{EQuestObjective::Rebirth, 1},
		{EQuestObjective::DefeatBoss, 1},
		{EQuestObjective::KillMonster, 35},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::ReachLevel, 25},
		{EQuestObjective::ClimbTower, 15},
		{EQuestObjective::KillMonster, 50},
		{EQuestObjective::DefeatBoss, 1},
	};
	const TCHAR* MainChainIds[] = {
		TEXT("main_ch1_001"),
		TEXT("main_ch1_002"),
		TEXT("main_ch1_003"),
		TEXT("main_ch1_004"),
		TEXT("main_ch1_005"),
		TEXT("main_ch1_006"),
		TEXT("main_ch1_007"),
		TEXT("main_ch2_001"),
		TEXT("main_ch2_002"),
		TEXT("main_ch2_003"),
		TEXT("main_ch2_004"),
		TEXT("main_ch2_005"),
		TEXT("main_ch3_001"),
		TEXT("main_ch3_002"),
		TEXT("main_ch3_003"),
		TEXT("main_ch3_004"),
		TEXT("main_ch3_005"),
		TEXT("main_ch3_006"),
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(MainChainIds); ++Index)
	{
		Quests->RecordProgress(MainChainProgress[Index].Key, MainChainProgress[Index].Value);
		const FQuestClaimResult Claim = Quests->ClaimQuest(MainChainIds[Index]);
		TestTrue(*FString::Printf(TEXT("%s can be claimed"), MainChainIds[Index]), Claim.bSuccess);
	}

	FQuestState ChapterFourStart;
	TestTrue(TEXT("Claiming chapter three finale unlocks chapter four start"), Quests->GetQuestState(TEXT("main_ch4_001"), ChapterFourStart));
	TestEqual(TEXT("Chapter four quest starts on map 4-1"), ChapterFourStart.ChapterMapId, FString(TEXT("4-1")));

	Quests->RecordProgress(EQuestObjective::KillMonster, 65);
	FQuestClaimResult ChapterFourFirstClaim = Quests->ClaimQuest(TEXT("main_ch4_001"));
	TestTrue(TEXT("Chapter four first quest can be claimed"), ChapterFourFirstClaim.bSuccess);
	TestTrue(TEXT("Chapter four first quest unlocks map survey"), ChapterFourFirstClaim.UnlockedQuestIds.Contains(TEXT("main_ch4_002")));

	Quests->RecordProgress(EQuestObjective::ClearMap, 1);
	TestTrue(TEXT("Chapter four map survey can be claimed"), Quests->ClaimQuest(TEXT("main_ch4_002")).bSuccess);
	Quests->RecordProgress(EQuestObjective::ReachLevel, 40);
	TestTrue(TEXT("Chapter four level gate can be claimed"), Quests->ClaimQuest(TEXT("main_ch4_003")).bSuccess);
	Quests->RecordProgress(EQuestObjective::ClimbTower, 25);
	TestTrue(TEXT("Chapter four tower gate can be claimed"), Quests->ClaimQuest(TEXT("main_ch4_004")).bSuccess);
	Quests->RecordProgress(EQuestObjective::KillMonster, 80);
	TestTrue(TEXT("Chapter four legion quest can be claimed"), Quests->ClaimQuest(TEXT("main_ch4_005")).bSuccess);
	Quests->RecordProgress(EQuestObjective::DefeatBoss, 1);
	TestTrue(TEXT("Chapter four boss quest can be claimed"), Quests->ClaimQuest(TEXT("main_ch4_006")).bSuccess);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceChapterThreePrerequisiteChainTest,
	"IdleProject.GameCore.QuestService.ChapterThreePrerequisiteChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceChapterThreePrerequisiteChainTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	const TArray<TPair<EQuestObjective, int32>> MainChainProgress = {
		{EQuestObjective::KillMonster, 5},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 12},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 20},
		{EQuestObjective::Enhance, 2},
		{EQuestObjective::DefeatBoss, 1},
		{EQuestObjective::KillMonster, 25},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::ReachLevel, 10},
		{EQuestObjective::Rebirth, 1},
		{EQuestObjective::DefeatBoss, 1},
	};
	const TCHAR* MainChainIds[] = {
		TEXT("main_ch1_001"),
		TEXT("main_ch1_002"),
		TEXT("main_ch1_003"),
		TEXT("main_ch1_004"),
		TEXT("main_ch1_005"),
		TEXT("main_ch1_006"),
		TEXT("main_ch1_007"),
		TEXT("main_ch2_001"),
		TEXT("main_ch2_002"),
		TEXT("main_ch2_003"),
		TEXT("main_ch2_004"),
		TEXT("main_ch2_005"),
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(MainChainIds); ++Index)
	{
		Quests->RecordProgress(MainChainProgress[Index].Key, MainChainProgress[Index].Value);
		const FQuestClaimResult Claim = Quests->ClaimQuest(MainChainIds[Index]);
		TestTrue(*FString::Printf(TEXT("%s can be claimed"), MainChainIds[Index]), Claim.bSuccess);
	}

	FQuestState ChapterThreeStart;
	TestTrue(TEXT("Claiming chapter two finale unlocks chapter three start"), Quests->GetQuestState(TEXT("main_ch3_001"), ChapterThreeStart));
	TestEqual(TEXT("Chapter three quest starts on map 3-1"), ChapterThreeStart.ChapterMapId, FString(TEXT("3-1")));

	Quests->RecordProgress(EQuestObjective::KillMonster, 35);
	FQuestClaimResult ChapterThreeFirstClaim = Quests->ClaimQuest(TEXT("main_ch3_001"));
	TestTrue(TEXT("Chapter three first quest can be claimed"), ChapterThreeFirstClaim.bSuccess);
	TestTrue(TEXT("Chapter three first quest unlocks map survey"), ChapterThreeFirstClaim.UnlockedQuestIds.Contains(TEXT("main_ch3_002")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceExpandedUnlockWeeklyResetTest,
	"IdleProject.GameCore.QuestService.ExpandedUnlockWeeklyReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceExpandedUnlockWeeklyResetTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	FQuestState WeeklyBoss;
	TestTrue(TEXT("Weekly boss quest is active on initialize"), Quests->GetQuestState(TEXT("weekly_defeat_bosses"), WeeklyBoss));
	TestEqual(TEXT("Weekly quest uses type enum slot 2"), static_cast<int32>(WeeklyBoss.Type), 2);

	Quests->RecordProgress(static_cast<EQuestObjective>(4), 3);
	TestTrue(TEXT("Weekly boss quest remains available"), Quests->GetQuestState(TEXT("weekly_defeat_bosses"), WeeklyBoss));
	TestTrue(TEXT("Weekly progress completes at target"), WeeklyBoss.bCompleted);

	Quests->ResetWeeklyQuestsIfNeeded(TEXT("2026-W22"));
	TestTrue(TEXT("Same week keeps weekly progress"), Quests->GetQuestState(TEXT("weekly_defeat_bosses"), WeeklyBoss));
	TestEqual(TEXT("Same ISO week keeps progress"), WeeklyBoss.Progress, 3);

	Quests->ResetWeeklyQuestsIfNeeded(TEXT("2026-W23"));
	TestTrue(TEXT("Next week keeps weekly quest active"), Quests->GetQuestState(TEXT("weekly_defeat_bosses"), WeeklyBoss));
	TestEqual(TEXT("Weekly progress resets on next week"), WeeklyBoss.Progress, 0);
	TestFalse(TEXT("Weekly completion resets on next week"), WeeklyBoss.bCompleted);
	TestFalse(TEXT("Weekly claim state resets on next week"), WeeklyBoss.bClaimed);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceReachLevelMaximumProgressTest,
	"IdleProject.GameCore.QuestService.ReachLevelMaximumProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceReachLevelMaximumProgressTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	Quests->RecordProgress(EQuestObjective::ReachLevel, 2);
	Quests->RecordProgress(EQuestObjective::ReachLevel, 3);
	Quests->RecordProgress(EQuestObjective::ReachLevel, 4);

	FQuestState ReachLevelDaily;
	TestTrue(TEXT("Reach level daily quest is active"), Quests->GetQuestState(TEXT("daily_reach_level"), ReachLevelDaily));
	TestEqual(TEXT("ReachLevel records the maximum observed level, not cumulative levels"), ReachLevelDaily.Progress, 4);
	TestFalse(TEXT("Partial maximum level does not complete the quest"), ReachLevelDaily.bCompleted);

	Quests->RecordProgress(EQuestObjective::ReachLevel, 10);
	TestTrue(TEXT("Reach level daily quest remains active"), Quests->GetQuestState(TEXT("daily_reach_level"), ReachLevelDaily));
	TestEqual(TEXT("ReachLevel completes at the reached level target"), ReachLevelDaily.Progress, 10);
	TestTrue(TEXT("Target level completes the quest"), ReachLevelDaily.bCompleted);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceChapterTwoPrerequisiteChainTest,
	"IdleProject.GameCore.QuestService.ChapterTwoPrerequisiteChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceChapterTwoPrerequisiteChainTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	const TArray<TPair<EQuestObjective, int32>> MainChainProgress = {
		{EQuestObjective::KillMonster, 5},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 12},
		{EQuestObjective::ClearMap, 1},
		{EQuestObjective::KillMonster, 20},
		{EQuestObjective::Enhance, 2},
		{static_cast<EQuestObjective>(4), 1},
	};
	const TCHAR* MainChainIds[] = {
		TEXT("main_ch1_001"),
		TEXT("main_ch1_002"),
		TEXT("main_ch1_003"),
		TEXT("main_ch1_004"),
		TEXT("main_ch1_005"),
		TEXT("main_ch1_006"),
		TEXT("main_ch1_007"),
	};

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(MainChainIds); ++Index)
	{
		Quests->RecordProgress(MainChainProgress[Index].Key, MainChainProgress[Index].Value);
		const FQuestClaimResult Claim = Quests->ClaimQuest(MainChainIds[Index]);
		TestTrue(*FString::Printf(TEXT("%s can be claimed"), MainChainIds[Index]), Claim.bSuccess);
	}

	FQuestState ChapterTwoStart;
	TestTrue(TEXT("Claiming chapter one finale unlocks chapter two start"), Quests->GetQuestState(TEXT("main_ch2_001"), ChapterTwoStart));
	TestEqual(TEXT("Chapter two quest starts on map 2-1"), ChapterTwoStart.ChapterMapId, FString(TEXT("2-1")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceDailyResetTest,
	"IdleProject.GameCore.QuestService.DailyReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceDailyResetTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));
	Quests->RecordProgress(EQuestObjective::ClaimOffline, 1);

	FQuestState DailyQuest;
	TestTrue(TEXT("Daily offline quest exists"), Quests->GetQuestState(TEXT("daily_claim_offline"), DailyQuest));
	TestTrue(TEXT("Daily quest completed before reset"), DailyQuest.bCompleted);
	TestEqual(TEXT("Daily reset date starts at seed date"), DailyQuest.DailyResetDate, FString(TEXT("2026-05-26")));

	Quests->ResetDailyQuestsIfNeeded(TEXT("2026-05-27"));

	TestTrue(TEXT("Daily offline quest still exists after reset"), Quests->GetQuestState(TEXT("daily_claim_offline"), DailyQuest));
	TestEqual(TEXT("Daily progress resets on next UTC date"), DailyQuest.Progress, 0);
	TestFalse(TEXT("Daily completion resets on next UTC date"), DailyQuest.bCompleted);
	TestFalse(TEXT("Daily claimed state resets on next UTC date"), DailyQuest.bClaimed);
	TestEqual(TEXT("Daily reset date advances"), DailyQuest.DailyResetDate, FString(TEXT("2026-05-27")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceWeeklySaveRoundTripTest,
	"IdleProject.GameCore.QuestService.WeeklySaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceWeeklySaveRoundTripTest::RunTest(const FString& Parameters)
{
	UQuestService* SourceQuests = NewObject<UQuestService>();
	SourceQuests->InitializeDefaultQuests(TEXT("2026-05-26"));
	SourceQuests->ResetWeeklyQuestsIfNeeded(TEXT("2026-W22"));
	SourceQuests->RecordProgress(static_cast<EQuestObjective>(9), 2500);

	TArray<FQuestSaveEntry> CapturedEntries;
	FString CapturedDailyReset;
	FString CapturedWeeklyReset;
	SourceQuests->CaptureState(CapturedEntries, CapturedDailyReset, CapturedWeeklyReset);

	UQuestService* RestoredQuests = NewObject<UQuestService>();
	RestoredQuests->RestoreState(CapturedEntries, CapturedDailyReset, CapturedWeeklyReset);

	FQuestState WeeklySpend;
	TestTrue(TEXT("Weekly spend quest restores"), RestoredQuests->GetQuestState(TEXT("weekly_spend_gold"), WeeklySpend));
	TestEqual(TEXT("Weekly progress round trips"), WeeklySpend.Progress, 2500);
	TestEqual(TEXT("Weekly reset id round trips"), WeeklySpend.WeeklyResetId, FString(TEXT("2026-W22")));

	RestoredQuests->ResetWeeklyQuestsIfNeeded(TEXT("2026-W23"));
	TestTrue(TEXT("Weekly spend remains active after reset"), RestoredQuests->GetQuestState(TEXT("weekly_spend_gold"), WeeklySpend));
	TestEqual(TEXT("Weekly progress resets after restored week boundary"), WeeklySpend.Progress, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceQuestRewardTest,
	"IdleProject.GameCore.IdleGameInstance.QuestRewardAndHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceQuestRewardTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeQuestServiceForTests(TEXT("2026-05-26"));

	GameInstance->RecordQuestProgress(EQuestObjective::KillMonster, 5);

	FQuestState MainQuest;
	TestTrue(TEXT("Game instance exposes quest state"), GameInstance->GetQuestState(TEXT("main_ch1_001"), MainQuest));
	TestTrue(TEXT("Kill hook completes first main quest"), MainQuest.bCompleted);

	FQuestClaimResult Claim = GameInstance->ClaimQuest(TEXT("main_ch1_001"));
	TestTrue(TEXT("Game instance claims completed quest"), Claim.bSuccess);
	TestEqual(TEXT("Quest claim adds reward gold"), GameInstance->GetGold(), static_cast<int64>(150));
	TestEqual(TEXT("Quest claim adds reward exp"), GameInstance->GetCurrentExp(), static_cast<int64>(80));

	GameInstance->SetLastSeenUnixSec(1000);
	GameInstance->ClaimOfflineRewardsAt(1060, 0);

	FQuestState OfflineDaily;
	TestTrue(TEXT("Offline daily quest exists"), GameInstance->GetQuestState(TEXT("daily_claim_offline"), OfflineDaily));
	TestTrue(TEXT("Offline reward claim hook completes daily quest"), OfflineDaily.bCompleted);

	GameInstance->RecordGearEnhanced();
	GameInstance->RecordGearEnhanced();
	GameInstance->RecordGearEnhanced();

	FQuestState EnhanceDaily;
	TestTrue(TEXT("Enhance daily quest exists"), GameInstance->GetQuestState(TEXT("daily_enhance_gear"), EnhanceDaily));
	TestEqual(TEXT("Enhance hook reaches daily quest target"), EnhanceDaily.Progress, 3);
	TestTrue(TEXT("Enhance hook completes daily quest"), EnhanceDaily.bCompleted);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceExpandedQuestHookTest,
	"IdleProject.GameCore.IdleGameInstance.ExpandedQuestHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceExpandedQuestHookTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeQuestServiceForTests(TEXT("2026-05-26"));
	GameInstance->InitializeStageServiceForTests();
	GameInstance->InitializePetSeasonServicesForTests();

	UStageService* StageService = GameInstance->GetStageService();
	TestNotNull(TEXT("Stage service exists for boss hook"), StageService);
	if (StageService)
	{
		for (int32 Stage = 1; Stage <= UStageService::StagesPerChapter; ++Stage)
		{
			const int32 KillsToAdvance = StageService->GetKillsToAdvance();
			for (int32 Kill = 0; Kill < KillsToAdvance; ++Kill)
			{
				StageService->RecordKill(Stage == UStageService::StagesPerChapter);
			}
		}
	}

	FQuestState WeeklyBoss;
	TestTrue(TEXT("Weekly boss quest exists after stage hook"), GameInstance->GetQuestState(TEXT("weekly_defeat_bosses"), WeeklyBoss));
	TestEqual(TEXT("Chapter boss event records DefeatBoss progress"), WeeklyBoss.Progress, 1);

	GameInstance->AddExp(1000000);
	FQuestState ReachLevelDaily;
	TestTrue(TEXT("Reach level daily exists"), GameInstance->GetQuestState(TEXT("daily_reach_level"), ReachLevelDaily));
	TestTrue(TEXT("Level up hook records highest reached level progress"), ReachLevelDaily.Progress >= 10);

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	GameInstance->AddGold(100000);
	const FShopPurchaseResult GearRoll = GameInstance->TryBuyGearRoll(Inventory);
	TestTrue(TEXT("Gear roll succeeds for hook test"), GearRoll.bPurchased);

	FQuestState GearRollDaily;
	TestTrue(TEXT("Gear roll daily exists"), GameInstance->GetQuestState(TEXT("daily_roll_gear_shop"), GearRollDaily));
	TestEqual(TEXT("Gear roll hook records one roll"), GearRollDaily.Progress, 1);

	const FPetFeedResult PetFeed = GameInstance->TryFeedPet(TEXT("dog"));
	TestTrue(TEXT("Pet feed succeeds for hook test"), PetFeed.bFed);

	FQuestState FeedPetDaily;
	TestTrue(TEXT("Feed pet daily exists"), GameInstance->GetQuestState(TEXT("daily_feed_pet"), FeedPetDaily));
	TestEqual(TEXT("Pet feed hook records one feed"), FeedPetDaily.Progress, 1);

	return true;
}

#endif
