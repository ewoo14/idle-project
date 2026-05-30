#include "Misc/AutomationTest.h"
#include "GameCore/AutomationPolicyService.h"
#include "GameCore/BuffService.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/IdleGameInstance.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationSkillRuleTest,
	"IdleProject.GameCore.Automation.SkillRule",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationSkillRuleTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	// Always
	TestTrue(TEXT("always"), S::EvaluateSkillRule(ESkillAutoCondition::Always, 0.3f, 1.0f, false, false));
	// BossEliteOnly
	TestTrue(TEXT("boss true"), S::EvaluateSkillRule(ESkillAutoCondition::BossEliteOnly, 0.3f, 1.0f, true, false));
	TestFalse(TEXT("boss false"), S::EvaluateSkillRule(ESkillAutoCondition::BossEliteOnly, 0.3f, 1.0f, false, false));
	// HpBelow
	TestTrue(TEXT("hp at threshold"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 0.3f, 0.3f, false, false));
	TestFalse(TEXT("hp above"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 0.3f, 0.31f, false, false));
	// HpBelow 클램프
	TestTrue(TEXT("hp clamp hi"), S::EvaluateSkillRule(ESkillAutoCondition::HpBelow, 5.0f, 1.0f, false, false));
	// MaintainBuff
	TestTrue(TEXT("buff inactive"), S::EvaluateSkillRule(ESkillAutoCondition::MaintainBuff, 0.3f, 1.0f, false, false));
	TestFalse(TEXT("buff active"), S::EvaluateSkillRule(ESkillAutoCondition::MaintainBuff, 0.3f, 1.0f, false, true));
	// SetSkillRule 교체
	UAutomationPolicyService* Svc = NewObject<UAutomationPolicyService>();
	FSkillAutoRule R; R.SkillId = FName(TEXT("heavy_strike")); R.Condition = ESkillAutoCondition::BossEliteOnly;
	Svc->SetSkillRule(R);
	TestEqual(TEXT("one rule"), Svc->GetSkillRules().Num(), 1);
	R.Condition = ESkillAutoCondition::HpBelow; Svc->SetSkillRule(R);
	TestEqual(TEXT("replace not add"), Svc->GetSkillRules().Num(), 1);
	TestEqual(TEXT("replaced cond"), (int32)Svc->GetSkillRules()[0].Condition, (int32)ESkillAutoCondition::HpBelow);
	Svc->ClearSkillRule(FName(TEXT("heavy_strike")));
	TestEqual(TEXT("cleared"), Svc->GetSkillRules().Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAutomationSellUpgradeTest,
	"IdleProject.GameCore.Automation.SellUpgrade",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAutomationSellUpgradeTest::RunTest(const FString& Parameters)
{
	using S = UAutomationPolicyService;
	TestEqual(TEXT("mult lv0"), S::GetSellValueMultiplier(0), 1.0f);
	TestTrue(TEXT("mult lv10"), FMath::IsNearlyEqual(S::GetSellValueMultiplier(10), 1.2f, 1e-4f));
	TestEqual(TEXT("mult neg guard"), S::GetSellValueMultiplier(-5), 1.0f);
	TestEqual(TEXT("cost lv0 base"), S::SellUpgradeNextCost(0), (int64)50000);
	TestTrue(TEXT("cost grows"), S::SellUpgradeNextCost(5) > S::SellUpgradeNextCost(4));
	// 상태 setter 클램프
	UAutomationPolicyService* Svc = NewObject<UAutomationPolicyService>();
	Svc->SetSellValueUpgradeLevel(-3);
	TestEqual(TEXT("level clamp"), Svc->GetSellValueUpgradeLevel(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
