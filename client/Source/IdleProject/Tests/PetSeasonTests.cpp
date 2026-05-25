#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/PetService.h"
#include "GameCore/SeasonService.h"

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

#endif
