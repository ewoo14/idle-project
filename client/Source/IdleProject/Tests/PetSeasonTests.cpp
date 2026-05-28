#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/SeasonService.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetServiceDefinitionParityTest,
	"IdleProject.GameCore.PetService.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetServiceDefinitionParityTest::RunTest(const FString& Parameters)
{
	UPetService* Pets = NewObject<UPetService>();
	Pets->InitializeDefaultPets();

	const TArray<FPetDefinition> Definitions = Pets->GetPetDefinitions();
	TestEqual(TEXT("Pet definition count matches server petBonus.ts"), Definitions.Num(), 10);

	TestEqual(TEXT("Dog pet id mirrors server"), Definitions[0].PetId, FString(TEXT("dog")));
	TestEqual(TEXT("Dog grants gold bonus"), Definitions[0].BonusType, EPetBonusType::Gold);
	TestEqual(TEXT("Dog gold bonus percent mirrors server"), Definitions[0].BonusPercent, 20.0f);

	TestEqual(TEXT("Bird pet id mirrors server"), Definitions[1].PetId, FString(TEXT("bird")));
	TestEqual(TEXT("Bird grants drop bonus"), Definitions[1].BonusType, EPetBonusType::Drop);
	TestEqual(TEXT("Bird drop bonus percent mirrors server"), Definitions[1].BonusPercent, 15.0f);

	TestEqual(TEXT("Cat grants exp bonus"), Definitions[2].BonusType, EPetBonusType::Exp);
	TestEqual(TEXT("Wolf grants physical attack percent"), Definitions[3].BonusType, EPetBonusType::PhysAtk);
	TestEqual(TEXT("Owl grants magic attack percent"), Definitions[4].BonusType, EPetBonusType::MagicAtk);
	TestEqual(TEXT("Bear grants HP percent"), Definitions[5].BonusType, EPetBonusType::Hp);
	TestEqual(TEXT("Turtle grants defense percent"), Definitions[6].BonusType, EPetBonusType::Def);
	TestEqual(TEXT("Fox grants upgraded gold percent"), Definitions[7].BonusPercent, 30.0f);
	TestEqual(TEXT("Rabbit grants upgraded drop percent"), Definitions[8].BonusPercent, 25.0f);
	TestEqual(TEXT("Dragon grants all-stat percent"), Definitions[9].BonusType, EPetBonusType::AllStat);
	TestEqual(TEXT("Dragon all-stat percent mirrors server"), Definitions[9].BonusPercent, 8.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetServiceEquipBonusTest,
	"IdleProject.GameCore.PetService.EquipBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetServiceEquipBonusTest::RunTest(const FString& Parameters)
{
	UPetService* Pets = NewObject<UPetService>();
	Pets->InitializeDefaultPets();

	TestTrue(TEXT("V1 starts with dog equipped"), Pets->EquipPet(TEXT("dog")));
	TestEqual(TEXT("Equipped dog grants 20 percent gold"), Pets->GetEquippedPetGoldBonusPercent(), 20.0f);
	TestEqual(TEXT("Equipped dog grants no drop bonus"), Pets->GetEquippedPetDropBonusPercent(), 0.0f);
	TestEqual(TEXT("Gold bonus rounds down after percent multiply"), Pets->ApplyGoldBonus(15), static_cast<int64>(18));

	TestTrue(TEXT("Owned bird can be equipped"), Pets->EquipPet(TEXT("bird")));
	TestEqual(TEXT("Equipped bird grants no gold bonus"), Pets->GetEquippedPetGoldBonusPercent(), 0.0f);
	TestEqual(TEXT("Equipped bird grants 15 percent drop"), Pets->GetEquippedPetDropBonusPercent(), 15.0f);
	TestEqual(TEXT("Drop chance applies percent as multiplier"), Pets->ApplyDropBonusChance(0.05f), 0.0575f);

	TestFalse(TEXT("Unknown pet cannot be equipped"), Pets->EquipPet(TEXT("slime")));
	TestFalse(TEXT("Locked expansion pet cannot be equipped before unlock"), Pets->EquipPet(TEXT("cat")));
	TestTrue(TEXT("Expansion pet unlock succeeds"), Pets->TryUnlockPet(TEXT("cat")));
	TestTrue(TEXT("Unlocked cat can be equipped"), Pets->EquipPet(TEXT("cat")));
	TestEqual(TEXT("Equipped cat grants exp bonus"), Pets->GetEquippedPetExpBonusPercent(), 15.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetServiceExpansionBonusTest,
	"IdleProject.GameCore.PetService.ExpansionBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetServiceExpansionBonusTest::RunTest(const FString& Parameters)
{
	UPetService* Pets = NewObject<UPetService>();
	Pets->InitializeDefaultPets();

	TestTrue(TEXT("Wolf unlock succeeds"), Pets->TryUnlockPet(TEXT("wolf")));
	TestTrue(TEXT("Wolf equip succeeds"), Pets->EquipPet(TEXT("wolf")));
	FPetStatBonus WolfBonus = Pets->GetEquippedPetStatBonus();
	TestEqual(TEXT("Wolf grants physical attack percent as ratio"), WolfBonus.PhysAtkPct, 0.10f);
	TestEqual(TEXT("Wolf does not grant flat HP"), WolfBonus.HpPct, 0.0f);
	TestEqual(TEXT("Wolf does not grant magic attack"), WolfBonus.MagicAtkPct, 0.0f);

	TestTrue(TEXT("Turtle unlock succeeds"), Pets->TryUnlockPet(TEXT("turtle")));
	TestTrue(TEXT("Turtle equip succeeds"), Pets->EquipPet(TEXT("turtle")));
	FPetStatBonus TurtleBonus = Pets->GetEquippedPetStatBonus();
	TestEqual(TEXT("Turtle grants physical defense percent as ratio"), TurtleBonus.PhysDefPct, 0.12f);
	TestEqual(TEXT("Turtle grants magic defense percent as ratio"), TurtleBonus.MagicDefPct, 0.12f);

	TestTrue(TEXT("Dragon unlock succeeds"), Pets->TryUnlockPet(TEXT("dragon")));
	TestTrue(TEXT("Dragon equip succeeds"), Pets->EquipPet(TEXT("dragon")));
	TestTrue(TEXT("Dragon feed scales all-stat percent"), Pets->FeedPet(TEXT("dragon")));
	FPetStatBonus DragonBonus = Pets->GetEquippedPetStatBonus();
	TestEqual(TEXT("Dragon level one scales physical attack percent"), DragonBonus.PhysAtkPct, 0.088f);
	TestEqual(TEXT("Dragon level one scales magic attack percent"), DragonBonus.MagicAtkPct, 0.088f);
	TestEqual(TEXT("Dragon level one scales physical defense percent"), DragonBonus.PhysDefPct, 0.088f);
	TestEqual(TEXT("Dragon level one scales magic defense percent"), DragonBonus.MagicDefPct, 0.088f);
	TestEqual(TEXT("Dragon all-stat does not grant HP"), DragonBonus.HpPct, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetLevelFormulaTest,
	"IdleProject.GameCore.PetLevelFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetLevelFormulaTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Level zero feed cost uses first quadratic step"), FPetLevelFormula::GetFeedCost(0), static_cast<int64>(500));
	TestEqual(TEXT("Level one feed cost uses second quadratic step"), FPetLevelFormula::GetFeedCost(1), static_cast<int64>(2000));
	TestEqual(TEXT("Negative level is clamped to level zero cost"), FPetLevelFormula::GetFeedCost(-1), static_cast<int64>(500));
	TestEqual(TEXT("Max level has no feed cost"), FPetLevelFormula::GetFeedCost(FPetLevelFormula::MaxPetLevel), static_cast<int64>(0));
	TestEqual(TEXT("Above max level has no feed cost"), FPetLevelFormula::GetFeedCost(FPetLevelFormula::MaxPetLevel + 1), static_cast<int64>(0));

	TestEqual(TEXT("Level zero keeps base bonus"), FPetLevelFormula::GetBonusMultiplier(0), 1.0f);
	TestEqual(TEXT("Level five grants 1.5x bonus"), FPetLevelFormula::GetBonusMultiplier(5), 1.5f);
	TestEqual(TEXT("Level ten grants 2.0x bonus"), FPetLevelFormula::GetBonusMultiplier(10), 2.0f);
	TestEqual(TEXT("Negative level keeps base bonus"), FPetLevelFormula::GetBonusMultiplier(-1), 1.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetServiceGrowthTest,
	"IdleProject.GameCore.PetService.Growth",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetServiceGrowthTest::RunTest(const FString& Parameters)
{
	UPetService* Pets = NewObject<UPetService>();
	Pets->InitializeDefaultPets();

	TestEqual(TEXT("Known pet starts at level zero"), Pets->GetPetLevel(TEXT("dog")), 0);
	TestEqual(TEXT("Unknown pet reports level zero"), Pets->GetPetLevel(TEXT("slime")), 0);
	TestTrue(TEXT("Owned pet can be fed"), Pets->FeedPet(TEXT("dog")));
	TestEqual(TEXT("Fed pet gains one level"), Pets->GetPetLevel(TEXT("dog")), 1);
	TestEqual(TEXT("Level one dog scales 20 percent gold bonus to 22 percent"), Pets->GetEquippedPetGoldBonusPercent(), 22.0f);
	TestEqual(TEXT("Level one dog gold bonus applies scaled percent"), Pets->ApplyGoldBonus(100), static_cast<int64>(122));

	TestTrue(TEXT("Owned bird can be equipped"), Pets->EquipPet(TEXT("bird")));
	TestTrue(TEXT("Bird can be fed"), Pets->FeedPet(TEXT("bird")));
	TestEqual(TEXT("Level one bird scales 15 percent drop bonus to 16.5 percent"), Pets->GetEquippedPetDropBonusPercent(), 16.5f);
	TestEqual(TEXT("Level one bird drop bonus applies scaled percent"), Pets->ApplyDropBonusChance(0.1f), 0.1165f);
	TestEqual(TEXT("Level one bird drop chance stays clamped at one"), Pets->ApplyDropBonusChance(1.0f), 1.0f);

	TestFalse(TEXT("Unknown pet cannot be fed"), Pets->FeedPet(TEXT("slime")));
	for (int32 FeedCount = Pets->GetPetLevel(TEXT("dog")); FeedCount < FPetLevelFormula::MaxPetLevel; ++FeedCount)
	{
		TestTrue(TEXT("Dog can be fed until max level"), Pets->FeedPet(TEXT("dog")));
	}
	TestEqual(TEXT("Dog reaches max pet level"), Pets->GetPetLevel(TEXT("dog")), FPetLevelFormula::MaxPetLevel);
	TestFalse(TEXT("Max level pet cannot be fed again"), Pets->FeedPet(TEXT("dog")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstancePetFeedTest,
	"IdleProject.GameCore.IdleGameInstance.PetFeed",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstancePetFeedTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializePetSeasonServicesForTests();

	FPetFeedResult NoGoldResult = GameInstance->TryFeedPet(TEXT("dog"));
	TestFalse(TEXT("Insufficient gold does not feed pet"), NoGoldResult.bFed);
	TestEqual(TEXT("Insufficient gold spends nothing"), NoGoldResult.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Insufficient gold leaves level zero"), NoGoldResult.NewLevel, 0);
	TestEqual(TEXT("Insufficient gold leaves balance unchanged"), GameInstance->GetGold(), static_cast<int64>(0));

	GameInstance->AddGold(FPetLevelFormula::GetFeedCost(0));
	FPetFeedResult FedResult = GameInstance->TryFeedPet(TEXT("dog"));
	TestTrue(TEXT("Enough gold feeds pet"), FedResult.bFed);
	TestEqual(TEXT("Feed result reports spent gold"), FedResult.GoldSpent, static_cast<int64>(500));
	TestEqual(TEXT("Feed result reports new level"), FedResult.NewLevel, 1);
	TestEqual(TEXT("Feed spends gold exactly once"), GameInstance->GetGold(), static_cast<int64>(0));
	TestEqual(TEXT("GameInstance exposes scaled pet bonus after feed"), GameInstance->GetEquippedPetGoldBonusPercent(), 22.0f);
	TestEqual(TEXT("GameInstance applies scaled pet gold bonus"), GameInstance->ApplyEquippedPetGoldBonus(100), static_cast<int64>(122));

	FPetFeedResult UnknownResult = GameInstance->TryFeedPet(TEXT("slime"));
	TestFalse(TEXT("Unknown pet cannot be fed"), UnknownResult.bFed);
	TestEqual(TEXT("Unknown pet reports zero level"), UnknownResult.NewLevel, 0);
	TestEqual(TEXT("Unknown pet spends nothing"), UnknownResult.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Unknown pet leaves gold unchanged"), GameInstance->GetGold(), static_cast<int64>(0));

	int64 TotalCostToMax = 0;
	for (int32 Level = 1; Level < FPetLevelFormula::MaxPetLevel; ++Level)
	{
		TotalCostToMax += FPetLevelFormula::GetFeedCost(Level);
	}
	GameInstance->AddGold(TotalCostToMax + 5000);
	for (int32 Level = 1; Level < FPetLevelFormula::MaxPetLevel; ++Level)
	{
		TestTrue(TEXT("Dog can be fed through max level"), GameInstance->TryFeedPet(TEXT("dog")).bFed);
	}
	const int64 GoldAtMax = GameInstance->GetGold();
	FPetFeedResult MaxResult = GameInstance->TryFeedPet(TEXT("dog"));
	TestFalse(TEXT("Max-level pet cannot be fed"), MaxResult.bFed);
	TestEqual(TEXT("Max-level pet reports current level"), MaxResult.NewLevel, FPetLevelFormula::MaxPetLevel);
	TestEqual(TEXT("Max-level pet spends nothing"), MaxResult.GoldSpent, static_cast<int64>(0));
	TestEqual(TEXT("Max-level pet leaves gold unchanged"), GameInstance->GetGold(), GoldAtMax);

	TestTrue(TEXT("GameInstance equips bird"), GameInstance->EquipPet(TEXT("bird")));
	GameInstance->AddGold(FPetLevelFormula::GetFeedCost(0));
	FPetFeedResult BirdFedResult = GameInstance->TryFeedPet(TEXT("bird"));
	TestTrue(TEXT("Bird feed succeeds through GameInstance"), BirdFedResult.bFed);
	TestEqual(TEXT("GameInstance exposes scaled pet drop bonus after feed"), GameInstance->GetEquippedPetDropBonusPercent(), 16.5f);
	TestEqual(TEXT("GameInstance applies scaled pet drop bonus"), GameInstance->ApplyEquippedPetDropBonusChance(0.1f), 0.1165f);

	TestFalse(TEXT("GameInstance blocks locked cat before unlock"), GameInstance->EquipPet(TEXT("cat")));
	GameInstance->AddGold(FPetLevelFormula::GetFeedCost(0));
	const int64 GoldBeforeLockedFeed = GameInstance->GetGold();
	FPetFeedResult LockedCatFeedResult = GameInstance->TryFeedPet(TEXT("cat"));
	TestFalse(TEXT("GameInstance blocks locked cat feed"), LockedCatFeedResult.bFed);
	TestEqual(TEXT("Locked cat feed spends no gold"), GameInstance->GetGold(), GoldBeforeLockedFeed);
	TestTrue(TEXT("GameInstance pet service unlocks cat"), GameInstance->GetPetService()->TryUnlockPet(TEXT("cat")));
	TestTrue(TEXT("GameInstance equips unlocked cat"), GameInstance->EquipPet(TEXT("cat")));
	TestEqual(TEXT("GameInstance exposes pet exp bonus"), GameInstance->GetEquippedPetExpBonusPercent(), 15.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeasonServiceTierClaimTest,
	"IdleProject.GameCore.SeasonService.TierClaim",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSeasonServiceTierClaimTest::RunTest(const FString& Parameters)
{
	USeasonService* Season = NewObject<USeasonService>();
	Season->InitializeDefaultSeason();

	TestEqual(TEXT("Season tier definition count mirrors server season.ts"), Season->GetSeasonTiers().Num(), 10);
	TestEqual(TEXT("Initial tokens are zero"), Season->GetSeasonTokens(), 0);
	TestEqual(TEXT("Initial reached tier is zero"), Season->GetReachedTier(), 0);

	Season->AddSeasonTokens(25);
	TestEqual(TEXT("Tokens accumulate"), Season->GetSeasonTokens(), 25);
	TestEqual(TEXT("Reached tier follows required tokens"), Season->GetReachedTier(), 2);

	FSeasonClaimResult TierTwoClaim = Season->ClaimSeasonReward(2);
	TestTrue(TEXT("Reached tier can be claimed"), TierTwoClaim.bSuccess);
	TestEqual(TEXT("Tier two grants gold"), TierTwoClaim.RewardType, ESeasonRewardType::Gold);
	TestEqual(TEXT("Tier two reward amount mirrors server"), TierTwoClaim.RewardAmount, static_cast<int64>(1000));

	FSeasonClaimResult DuplicateClaim = Season->ClaimSeasonReward(2);
	TestFalse(TEXT("Tier cannot be claimed twice"), DuplicateClaim.bSuccess);
	TestEqual(TEXT("Duplicate claim reports already claimed"), DuplicateClaim.Message, FString(TEXT("season_reward_already_claimed")));

	FSeasonClaimResult LockedClaim = Season->ClaimSeasonReward(3);
	TestFalse(TEXT("Unreached tier cannot be claimed"), LockedClaim.bSuccess);
	TestEqual(TEXT("Locked claim reports not reached"), LockedClaim.Message, FString(TEXT("season_tier_not_reached")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSeasonServiceDefinitionParityTest,
	"IdleProject.GameCore.SeasonService.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSeasonServiceDefinitionParityTest::RunTest(const FString& Parameters)
{
	USeasonService* Season = NewObject<USeasonService>();
	Season->InitializeDefaultSeason();

	const TArray<FSeasonTierDefinition>& Tiers = Season->GetSeasonTiers();
	TestEqual(TEXT("Season id mirrors server currentSeasonId"), Season->GetSeasonId(), 1);
	TestEqual(TEXT("Season free track has 10 tiers"), Tiers.Num(), 10);

	struct FExpectedTier
	{
		int32 Tier;
		int32 RequiredTokens;
		ESeasonRewardType RewardType;
		int64 RewardAmount;
	};

	const FExpectedTier ExpectedTiers[] = {
		{1, 10, ESeasonRewardType::Gold, 500},
		{2, 25, ESeasonRewardType::Gold, 1000},
		{3, 45, ESeasonRewardType::Exp, 300},
		{4, 70, ESeasonRewardType::Gold, 1800},
		{5, 100, ESeasonRewardType::Exp, 650},
		{6, 135, ESeasonRewardType::Gold, 3000},
		{7, 175, ESeasonRewardType::Exp, 1100},
		{8, 220, ESeasonRewardType::Gold, 4800},
		{9, 270, ESeasonRewardType::Exp, 1750},
		{10, 325, ESeasonRewardType::Gold, 7500},
	};

	const int32 ExpectedTierCount = UE_ARRAY_COUNT(ExpectedTiers);
	for (int32 Index = 0; Index < Tiers.Num() && Index < ExpectedTierCount; ++Index)
	{
		const FString Prefix = FString::Printf(TEXT("Tier %d mirrors server season.ts"), Index + 1);
		TestEqual(*(Prefix + TEXT(" tier")), Tiers[Index].Tier, ExpectedTiers[Index].Tier);
		TestEqual(*(Prefix + TEXT(" required tokens")), Tiers[Index].RequiredTokens, ExpectedTiers[Index].RequiredTokens);
		TestEqual(*(Prefix + TEXT(" reward type")), Tiers[Index].RewardType, ExpectedTiers[Index].RewardType);
		TestEqual(*(Prefix + TEXT(" reward amount")), Tiers[Index].RewardAmount, ExpectedTiers[Index].RewardAmount);
	}

	TestEqual(TEXT("Quest claim token reward mirrors V1 contract"), USeasonService::QuestClaimSeasonTokenReward, 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstancePetSeasonHooksTest,
	"IdleProject.GameCore.IdleGameInstance.PetSeasonHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstancePetSeasonHooksTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeQuestServiceForTests(TEXT("2026-05-26"));
	GameInstance->InitializePetSeasonServicesForTests();

	TestTrue(TEXT("GameInstance equips dog"), GameInstance->EquipPet(TEXT("dog")));
	TestEqual(TEXT("GameInstance exposes gold pet bonus"), GameInstance->GetEquippedPetGoldBonusPercent(), 20.0f);
	TestEqual(TEXT("GameInstance applies gold pet bonus"), GameInstance->ApplyEquippedPetGoldBonus(10), static_cast<int64>(12));

	GameInstance->RecordQuestProgress(EQuestObjective::KillMonster, 5);
	FQuestClaimResult QuestClaim = GameInstance->ClaimQuest(TEXT("main_ch1_001"));
	TestTrue(TEXT("Quest claim succeeds"), QuestClaim.bSuccess);
	TestEqual(TEXT("Successful quest claim grants V1 season tokens"), GameInstance->GetSeasonTokens(), 10);
	TestEqual(TEXT("Ten tokens reaches season tier one"), GameInstance->GetReachedSeasonTier(), 1);

	FSeasonClaimResult SeasonClaim = GameInstance->ClaimSeasonReward(1);
	TestTrue(TEXT("Reached season reward can be claimed through GameInstance"), SeasonClaim.bSuccess);
	TestEqual(TEXT("Tier one gold reward is added"), GameInstance->GetGold(), static_cast<int64>(650));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPetSeasonHudViewModelTest,
	"IdleProject.UI.HUD.PetSeasonViewModels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPetSeasonHudViewModelTest::RunTest(const FString& Parameters)
{
	UPetService* Pets = NewObject<UPetService>();
	Pets->InitializeDefaultPets();
	TestTrue(TEXT("Dog is equipped for the pet HUD seed"), Pets->EquipPet(TEXT("dog")));
	TestTrue(TEXT("Dog is fed once for the pet growth HUD seed"), Pets->FeedPet(TEXT("dog")));
	for (int32 Level = 0; Level < FPetLevelFormula::MaxPetLevel; ++Level)
	{
		TestTrue(TEXT("Bird reaches max level for disabled feed state"), Pets->FeedPet(TEXT("bird")));
	}

	const FIdleHUDPetPanelViewModel PetPanel = IdleProject::UI::BuildPetPanelViewModel(
		Pets->GetPetDefinitions(),
		Pets->GetEquippedPetId(),
		Pets->GetEquippedPetGoldBonusPercent(),
		Pets->GetEquippedPetDropBonusPercent(),
		2000,
		[Pets](const FString& PetId)
		{
			return Pets->GetPetLevel(PetId);
		},
		[Pets](const FString& PetId)
		{
			return Pets->IsPetOwned(PetId);
		});

	TestEqual(TEXT("Pet HUD shows expanded pet catalog"), PetPanel.Rows.Num(), 10);
	TestTrue(TEXT("Dog row is marked equipped"), PetPanel.Rows[0].bEquipped);
	TestFalse(TEXT("Bird row is not equipped"), PetPanel.Rows[1].bEquipped);
	TestTrue(TEXT("Dog row shows level one"), PetPanel.Rows[0].LevelLabel.ToString().Contains(TEXT("1")));
	TestTrue(TEXT("Dog row shows the next feed cost"), PetPanel.Rows[0].FeedCostLabel.ToString().Contains(TEXT("2,000")));
	TestTrue(TEXT("Dog row can be fed when gold covers the next cost"), PetPanel.Rows[0].bCanFeed);
	TestTrue(TEXT("Dog row exposes feed action label"), PetPanel.Rows[0].FeedActionLabel.ToString().Len() > 0);
	TestTrue(TEXT("Bird row cannot be fed at max level"), PetPanel.Rows[1].bFeedDisabled);
	TestFalse(TEXT("Locked cat row cannot be equipped"), PetPanel.Rows[2].bCanEquip);
	TestTrue(TEXT("Locked cat row disables feed"), PetPanel.Rows[2].bFeedDisabled);
	TestTrue(TEXT("Equipped pet summary includes scaled gold bonus"), PetPanel.GoldBonusLabel.ToString().Contains(TEXT("22%")));
	TestTrue(TEXT("Equipped pet summary includes drop bonus"), PetPanel.DropBonusLabel.ToString().Contains(TEXT("0%")));

	USeasonService* Season = NewObject<USeasonService>();
	Season->InitializeDefaultSeason();
	Season->AddSeasonTokens(10);

	const FIdleHUDSeasonPassViewModel SeasonPanel = IdleProject::UI::BuildSeasonPassViewModel(
		Season->GetSeasonTiers(),
		Season->GetSeasonTokens(),
		Season->GetReachedTier(),
		[Season](int32 Tier)
		{
			return Season->IsTierClaimed(Tier);
		});

	TestEqual(TEXT("Season HUD mirrors ten V1 tiers"), SeasonPanel.Rows.Num(), 10);
	TestEqual(TEXT("Season HUD token summary"), SeasonPanel.TokenLabel.ToString(), FString(TEXT("10 / 325")));
	TestTrue(TEXT("Tier one can be claimed"), SeasonPanel.Rows[0].bCanClaim);
	TestFalse(TEXT("Tier two is still locked"), SeasonPanel.Rows[1].bReached);
	TestTrue(TEXT("Tier one action is claim"), SeasonPanel.Rows[0].ActionLabel.ToString().Contains(TEXT("받기")));

	FSeasonClaimResult Claim = Season->ClaimSeasonReward(1);
	TestTrue(TEXT("Season service claim succeeds"), Claim.bSuccess);

	const FIdleHUDSeasonPassViewModel ClaimedPanel = IdleProject::UI::BuildSeasonPassViewModel(
		Season->GetSeasonTiers(),
		Season->GetSeasonTokens(),
		Season->GetReachedTier(),
		[Season](int32 Tier)
		{
			return Season->IsTierClaimed(Tier);
		});

	TestTrue(TEXT("Claimed tier is marked claimed"), ClaimedPanel.Rows[0].bClaimed);
	TestFalse(TEXT("Claimed tier cannot be claimed again"), ClaimedPanel.Rows[0].bCanClaim);
	TestTrue(TEXT("Claimed action label is final"), ClaimedPanel.Rows[0].ActionLabel.ToString().Contains(TEXT("수령 완료")));

	return true;
}

#endif
