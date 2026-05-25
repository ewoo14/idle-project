#include "Misc/AutomationTest.h"

#include "GameCore/QuestService.h"
#include "GameCore/OfflineRewardFormula.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOfflineRewardHudViewModelTest,
	"IdleProject.UI.HUD.OfflineRewardModalViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOfflineRewardHudViewModelTest::RunTest(const FString& Parameters)
{
	FOfflineRewardResult Reward;
	Reward.CappedSeconds = 3661;
	Reward.Gold = 12345;
	Reward.Exp = 678;
	Reward.TimeBonusMultiplier = 1.02;

	const FIdleHUDOfflineRewardViewModel ViewModel = IdleProject::UI::BuildOfflineRewardViewModel(Reward);
	TestTrue(TEXT("Reward with gold or exp is visible"), ViewModel.bVisible);
	TestEqual(TEXT("Title uses approved Korean copy"), ViewModel.Title.ToString(), FString(TEXT("오프라인 보상")));
	TestEqual(TEXT("Elapsed time is formatted as H:MM"), ViewModel.ElapsedLabel.ToString(), FString(TEXT("경과 시간 1:01")));
	TestEqual(TEXT("Gold line is formatted"), ViewModel.GoldLabel.ToString(), FString(TEXT("골드 +12,345")));
	TestEqual(TEXT("EXP line is formatted"), ViewModel.ExpLabel.ToString(), FString(TEXT("EXP +678")));
	TestEqual(TEXT("Claim button uses approved Korean copy"), ViewModel.ClaimLabel.ToString(), FString(TEXT("수령")));

	FOfflineRewardResult EmptyReward;
	EmptyReward.CappedSeconds = 3600;
	const FIdleHUDOfflineRewardViewModel EmptyViewModel = IdleProject::UI::BuildOfflineRewardViewModel(EmptyReward);
	TestFalse(TEXT("Reward with no gold and no exp is hidden"), EmptyViewModel.bVisible);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestLogHudViewModelTest,
	"IdleProject.UI.HUD.QuestLogViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestLogHudViewModelTest::RunTest(const FString& Parameters)
{
	TArray<FQuestState> States;

	FQuestState MainQuest;
	MainQuest.QuestId = TEXT("main_ch1_001");
	MainQuest.Type = EQuestType::Main;
	MainQuest.Title = FText::FromString(TEXT("Find the Broken Gate"));
	MainQuest.TargetCount = 5;
	MainQuest.Progress = 3;
	MainQuest.RewardGold = 150;
	MainQuest.RewardExp = 80;
	States.Add(MainQuest);

	FQuestState DailyQuest;
	DailyQuest.QuestId = TEXT("daily_claim_offline");
	DailyQuest.Type = EQuestType::Daily;
	DailyQuest.Title = FText::FromString(TEXT("Claim Rest Rewards"));
	DailyQuest.TargetCount = 1;
	DailyQuest.Progress = 1;
	DailyQuest.bCompleted = true;
	DailyQuest.RewardGold = 300;
	DailyQuest.RewardExp = 180;
	States.Add(DailyQuest);

	const FIdleHUDQuestLogViewModel ViewModel = IdleProject::UI::BuildQuestLogViewModel(States);
	TestEqual(TEXT("Quest log uses approved title copy"), ViewModel.Title.ToString(), FString(TEXT("퀘스트")));
	TestEqual(TEXT("Quest log exposes all active quests"), ViewModel.Rows.Num(), 2);
	TestEqual(TEXT("Main quest row uses approved Korean type label"), ViewModel.Rows[0].TypeLabel.ToString(), FString(TEXT("메인")));
	TestEqual(TEXT("Daily quest row uses approved Korean type label"), ViewModel.Rows[1].TypeLabel.ToString(), FString(TEXT("일일")));
	TestEqual(TEXT("Progress line shows current and target count"), ViewModel.Rows[0].ProgressLabel.ToString(), FString(TEXT("진행 3 / 5")));
	TestEqual(TEXT("Completed unclaimed quest is claimable"), ViewModel.Rows[1].ActionLabel.ToString(), FString(TEXT("수령")));
	TestTrue(TEXT("Completed unclaimed quest enables claim button"), ViewModel.Rows[1].bCanClaim);

	DailyQuest.bClaimed = true;
	States[1] = DailyQuest;
	const FIdleHUDQuestLogViewModel ClaimedViewModel = IdleProject::UI::BuildQuestLogViewModel(States);
	TestEqual(TEXT("Claimed quest action shows received copy"), ClaimedViewModel.Rows[1].ActionLabel.ToString(), FString(TEXT("완료")));
	TestFalse(TEXT("Claimed quest disables claim button"), ClaimedViewModel.Rows[1].bCanClaim);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthHudViewModelTest,
	"IdleProject.UI.HUD.RebirthPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthHudViewModelTest::RunTest(const FString& Parameters)
{
	const FIdleHUDRebirthViewModel LockedViewModel = IdleProject::UI::BuildRebirthViewModel(false, false, 80, 2, 10);
	TestEqual(TEXT("Panel title uses approved Korean copy"), LockedViewModel.Title.ToString(), FString(TEXT("환생")));
	TestEqual(TEXT("Locked status explains rebirth is unavailable"), LockedViewModel.StatusLabel.ToString(), FString(TEXT("환생 조건 미달")));
	TestEqual(TEXT("Boss gate uses approved copy"), LockedViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 필요")));
	TestEqual(TEXT("Level gate shows current progress"), LockedViewModel.LevelLabel.ToString(), FString(TEXT("레벨 80 / 100")));
	TestFalse(TEXT("Locked state disables rebirth button"), LockedViewModel.bCanRebirth);

	const FIdleHUDRebirthViewModel ReadyViewModel = IdleProject::UI::BuildRebirthViewModel(true, true, 100, 2, 10);
	TestEqual(TEXT("Ready status uses approved Korean copy"), ReadyViewModel.StatusLabel.ToString(), FString(TEXT("환생 가능")));
	TestEqual(TEXT("Boss defeated state uses approved copy"), ReadyViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 완료")));
	TestEqual(TEXT("Current count is exposed"), ReadyViewModel.CountLabel.ToString(), FString(TEXT("환생 2회")));
	TestEqual(TEXT("Permanent bonus uses approved copy"), ReadyViewModel.BonusLabel.ToString(), FString(TEXT("영구 보너스 10 포인트")));
	TestEqual(TEXT("Next bonus explains fixed reward"), ReadyViewModel.NextBonusLabel.ToString(), FString(TEXT("환생 진행 시 +5 포인트")));
	TestEqual(TEXT("Button uses approved Korean copy"), ReadyViewModel.ButtonLabel.ToString(), FString(TEXT("환생 진행")));
	TestTrue(TEXT("Ready state enables rebirth button"), ReadyViewModel.bCanRebirth);

	return true;
}

#endif
