#include "Misc/AutomationTest.h"

#include "CharacterSystem/StatFormulas.h"
#include "CharacterSystem/StatPointFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatPointFormulaLevelGrantTest,
	"IdleProject.Character.StatPointFormula.LevelGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatPointFormulaLevelGrantTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Level 1 has no level-up grant"), FStatPointFormula::GetStatPointsForLevelUp(1), 0);
	TestEqual(TEXT("Level 2 grants five stat points"), FStatPointFormula::GetStatPointsForLevelUp(2), 5);
	TestEqual(TEXT("Level cap transition still grants five stat points"), FStatPointFormula::GetStatPointsForLevelUp(100), 5);
	TestEqual(TEXT("Invalid level grant is clamped to zero"), FStatPointFormula::GetStatPointsForLevelUp(0), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatPointFormulaTotalGrantTest,
	"IdleProject.Character.StatPointFormula.TotalGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatPointFormulaTotalGrantTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Level 0 total clamps to zero"), FStatPointFormula::GetTotalStatPointsForLevel(0), 0);
	TestEqual(TEXT("Level 1 total is zero"), FStatPointFormula::GetTotalStatPointsForLevel(1), 0);
	TestEqual(TEXT("Level 2 total is five"), FStatPointFormula::GetTotalStatPointsForLevel(2), 5);
	TestEqual(TEXT("Level 10 total is forty-five"), FStatPointFormula::GetTotalStatPointsForLevel(10), 45);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAllocatedPrimaryStatsDerivedImpactTest,
	"IdleProject.Character.StatPointFormula.AllocatedPrimaryStatsAffectDerivedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAllocatedPrimaryStatsDerivedImpactTest::RunTest(const FString& Parameters)
{
	FPrimaryStats Primary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	const FDerivedStats BaseDerived = FStatFormulas::DeriveStats(Primary, 1);

	Primary.Str += 1.0f;
	const FDerivedStats AllocatedDerived = FStatFormulas::DeriveStats(Primary, 1);

	TestEqual(TEXT("One allocated STR raises physical attack by two"), AllocatedDerived.PhysAtk, BaseDerived.PhysAtk + 2.0f);
	TestEqual(TEXT("One allocated STR does not alter max HP"), AllocatedDerived.Hp, BaseDerived.Hp);

	return true;
}

#endif
