#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/QuestService.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestServiceProgressClaimTest,
	"IdleProject.GameCore.QuestService.ProgressClaimUnlock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestServiceProgressClaimTest::RunTest(const FString& Parameters)
{
	UQuestService* Quests = NewObject<UQuestService>();
	Quests->InitializeDefaultQuests(TEXT("2026-05-26"));

	TestEqual(TEXT("First main quest and three dailies are active"), Quests->GetActiveQuestStates().Num(), 4);

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
	TestEqual(TEXT("Quest definition count matches server quests.ts"), Definitions.Num(), 8);

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
		{TEXT("daily_kill_monsters"), EQuestType::Daily, EQuestObjective::KillMonster, 30, 500, 240, TEXT(""), TEXT("")},
		{TEXT("daily_claim_offline"), EQuestType::Daily, EQuestObjective::ClaimOffline, 1, 300, 180, TEXT(""), TEXT("")},
		{TEXT("daily_enhance_gear"), EQuestType::Daily, EQuestObjective::Enhance, 3, 650, 320, TEXT(""), TEXT("")},
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

#endif
