#include "Misc/AutomationTest.h"

#include "CharacterSystem/IdleMonster.h"
#include "GameCore/RewardFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRewardFormulaKillRewardScalingTest,
	"IdleProject.GameCore.RewardFormula.KillRewardScaling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRewardFormulaKillRewardScalingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Stage 1-1 normal EXP keeps legacy baseline"), FRewardFormula::ComputeKillExp(12, 0, false), int64(12));
	TestEqual(TEXT("Stage 1-5 normal EXP uses reward multiplier"), FRewardFormula::ComputeKillExp(12, 4, false), int64(19));
	TestEqual(TEXT("Negative stage EXP clamps to stage 1-1"), FRewardFormula::ComputeKillExp(12, -3, false), int64(12));
	TestEqual(TEXT("Stage 1-1 boss EXP uses boss bonus"), FRewardFormula::ComputeKillExp(12, 0, true), int64(96));
	TestEqual(TEXT("Stage 1-5 boss EXP combines stage multiplier and boss bonus"), FRewardFormula::ComputeKillExp(12, 4, true), int64(154));

	TestEqual(TEXT("Stage 1-1 normal gold keeps legacy baseline"), FRewardFormula::ComputeKillGold(10, 0, false), int64(10));
	TestEqual(TEXT("Stage 1-5 normal gold uses reward multiplier"), FRewardFormula::ComputeKillGold(10, 4, false), int64(16));
	TestEqual(TEXT("Stage 1-1 boss gold uses boss bonus"), FRewardFormula::ComputeKillGold(10, 0, true), int64(80));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRewardFormulaMonsterLevelTest,
	"IdleProject.GameCore.RewardFormula.MonsterLevelForStage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRewardFormulaMonsterLevelTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Stage 1-1 maps to monster level one"), FRewardFormula::GetMonsterLevelForStage(0), 1);
	TestEqual(TEXT("Negative stage maps to monster level one"), FRewardFormula::GetMonsterLevelForStage(-2), 1);
	TestEqual(TEXT("Stage 1-5 maps to monster level five"), FRewardFormula::GetMonsterLevelForStage(4), 5);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleMonsterRewardStageContextTest,
	"IdleProject.Combat.Monster.RewardStageContext",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleMonsterRewardStageContextTest::RunTest(const FString& Parameters)
{
	AIdleMonster* Monster = NewObject<AIdleMonster>();
	TestNotNull(TEXT("Monster is created"), Monster);
	if (!Monster)
	{
		return false;
	}

	TestEqual(TEXT("Monster defaults to stage 1-1 reward context"), Monster->GetStageGlobalIndex(), 0);

	Monster->SetStageGlobalIndex(4);
	TestEqual(TEXT("Monster stores spawn-time stage index"), Monster->GetStageGlobalIndex(), 4);

	Monster->SetStageGlobalIndex(-3);
	TestEqual(TEXT("Monster reward stage index clamps negative values"), Monster->GetStageGlobalIndex(), 0);

	return true;
}

#endif
