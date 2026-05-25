#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
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
	TestEqual(TEXT("Pet definition count matches server pets.ts"), Definitions.Num(), 2);

	TestEqual(TEXT("Dog pet id mirrors server"), Definitions[0].PetId, FString(TEXT("dog")));
	TestEqual(TEXT("Dog grants gold bonus"), Definitions[0].BonusType, EPetBonusType::Gold);
	TestEqual(TEXT("Dog gold bonus percent mirrors server"), Definitions[0].BonusPercent, 20.0f);

	TestEqual(TEXT("Bird pet id mirrors server"), Definitions[1].PetId, FString(TEXT("bird")));
	TestEqual(TEXT("Bird grants drop bonus"), Definitions[1].BonusType, EPetBonusType::Drop);
	TestEqual(TEXT("Bird drop bonus percent mirrors server"), Definitions[1].BonusPercent, 15.0f);

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

	const FIdleHUDPetPanelViewModel PetPanel = IdleProject::UI::BuildPetPanelViewModel(
		Pets->GetPetDefinitions(),
		Pets->GetEquippedPetId(),
		Pets->GetEquippedPetGoldBonusPercent(),
		Pets->GetEquippedPetDropBonusPercent());

	TestEqual(TEXT("Pet HUD shows both V1 pets"), PetPanel.Rows.Num(), 2);
	TestTrue(TEXT("Dog row is marked equipped"), PetPanel.Rows[0].bEquipped);
	TestFalse(TEXT("Bird row is not equipped"), PetPanel.Rows[1].bEquipped);
	TestTrue(TEXT("Equipped pet summary includes gold bonus"), PetPanel.GoldBonusLabel.ToString().Contains(TEXT("20%")));
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
