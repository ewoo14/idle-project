#include "Misc/AutomationTest.h"
#include "GameCore/AutomationPolicyService.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationProgressionDecisionTest,
	"IdleProject.GameCore.Automation.ProgressionDecision",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationProgressionDecisionTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;

	// Advance: 일반 전진
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::Advance, 5, 5, 3, true, false);
		TestEqual(TEXT("advance action"), (int32)D.Action, (int32)EProgressionAction::Advance);
		TestEqual(TEXT("advance target"), D.TargetGlobalStage, 6);
	}
	// Advance: 보스 자동도전 OFF → hold
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::Advance, 5, 5, 3, false, true);
		TestEqual(TEXT("boss off action"), (int32)D.Action, (int32)EProgressionAction::Hold);
		TestEqual(TEXT("boss off target"), D.TargetGlobalStage, 5);
	}
	// FarmLock: 미도달 클램프
	{
		const FProgressionDecision D = S::DecideOnClear(EProgressionMode::FarmLock, 5, 5, 99, true, false);
		TestEqual(TEXT("farmlock action"), (int32)D.Action, (int32)EProgressionAction::Hold);
		TestEqual(TEXT("farmlock clamp"), D.TargetGlobalStage, 6);
	}
	// AutoRetreat: 임계 도달 후퇴
	{
		const FProgressionDecision D = S::DecideOnDeath(EProgressionMode::AutoRetreat, 8, 3, 3);
		TestEqual(TEXT("retreat action"), (int32)D.Action, (int32)EProgressionAction::Retreat);
		TestEqual(TEXT("retreat target"), D.TargetGlobalStage, 7);
	}
	// AutoRetreat: 1스테이지 하한
	{
		const FProgressionDecision D = S::DecideOnDeath(EProgressionMode::AutoRetreat, 1, 5, 3);
		TestEqual(TEXT("retreat floor"), D.TargetGlobalStage, 1);
		TestEqual(TEXT("retreat floor hold"), (int32)D.Action, (int32)EProgressionAction::Hold);
	}
	// 해금 게이팅
	{
		TestTrue(TEXT("progression unlocked"), S::IsFeatureUnlocked(EAutomationFeature::Progression, 0, 0));
		TestFalse(TEXT("skill locked ch2"), S::IsFeatureUnlocked(EAutomationFeature::SkillTactics, 2, 0));
		TestTrue(TEXT("skill unlocked ch3"), S::IsFeatureUnlocked(EAutomationFeature::SkillTactics, 3, 0));
	}
	// 효율 비용 곡선
	{
		TestEqual(TEXT("cost lv0"), S::EfficiencyUpgradeCost(1000, 1.5f, 0), (int64)1000);
		TestEqual(TEXT("cost lv2"), S::EfficiencyUpgradeCost(1000, 1.5f, 2), (int64)2250);
		TestEqual(TEXT("cost neg guard"), S::EfficiencyUpgradeCost(1000, 1.5f, -3), (int64)1000);
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
