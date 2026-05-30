#include "Misc/AutomationTest.h"
#include "GameCore/AutomationPolicyService.h"
#include "GameCore/StageService.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationProgressionAdvanceFlowTest,
	"IdleProject.GameCore.Automation.AdvanceFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationProgressionAdvanceFlowTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	// FarmLock 고정: 클리어해도 항상 동일 스테이지 hold
	const FProgressionDecision D1 = S::DecideOnClear(EProgressionMode::FarmLock, 4, 7, 4, true, false);
	TestEqual(TEXT("farmlock hold stays"), D1.TargetGlobalStage, 4);
	// AutoRetreat: 임계 미만 사망은 유지, 클리어 시엔 전진
	const FProgressionDecision D2 = S::DecideOnDeath(EProgressionMode::AutoRetreat, 6, 1, 3);
	TestEqual(TEXT("autoretreat hold below threshold"), (int32)D2.Action, (int32)EProgressionAction::Hold);
	const FProgressionDecision D3 = S::DecideOnClear(EProgressionMode::AutoRetreat, 6, 6, 1, true, false);
	TestEqual(TEXT("autoretreat advances on clear"), D3.TargetGlobalStage, 7);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageJumpTest,
	"IdleProject.GameCore.Automation.StageJump",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageJumpTest::RunTest(const FString& Parameters)
{
	UStageService* Stage = NewObject<UStageService>();
	Stage->InitializeDefaultStages();
	Stage->JumpToGlobalStage(1);
	TestEqual(TEXT("jump floor chapter"), Stage->GetCurrentChapter(), 1);
	TestEqual(TEXT("jump floor stage"), Stage->GetCurrentStage(), 1);
	// 9스테이지(다음이 10=보스)
	Stage->JumpToGlobalStage(9);
	TestTrue(TEXT("stage9 next is boss"), Stage->IsNextStageBoss());
	// 10스테이지(보스): 다음은 새 챕터 1 → 보스 아님
	Stage->JumpToGlobalStage(10);
	TestFalse(TEXT("boss stage next not boss"), Stage->IsNextStageBoss());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
