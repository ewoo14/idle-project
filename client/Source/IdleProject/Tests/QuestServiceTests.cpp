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

	return true;
}

#endif
