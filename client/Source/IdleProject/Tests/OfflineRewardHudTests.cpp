#include "Misc/AutomationTest.h"

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

#endif
