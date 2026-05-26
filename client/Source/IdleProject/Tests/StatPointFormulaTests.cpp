#include "Misc/AutomationTest.h"

#include "CharacterSystem/StatFormulas.h"
#include "CharacterSystem/StatPointFormula.h"
#include "UI/IdleHUD.h"

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
	Primary.Int_ += 1.0f;
	Primary.Wis += 1.0f;
	Primary.Con += 1.0f;
	Primary.Luk += 1.0f;
	const FDerivedStats AllocatedDerived = FStatFormulas::DeriveStats(Primary, 1);

	TestEqual(TEXT("One allocated STR raises physical attack by two"), AllocatedDerived.PhysAtk, BaseDerived.PhysAtk + 2.0f);
	TestEqual(TEXT("One allocated INT and WIS raise rounded magic attack"), AllocatedDerived.MagicAtk, BaseDerived.MagicAtk + 2.0f);
	TestEqual(TEXT("One allocated CON raises max HP by ten"), AllocatedDerived.Hp, BaseDerived.Hp + 10.0f);
	TestEqual(TEXT("One allocated LUK raises crit rate by 0.002"), AllocatedDerived.CritRate, BaseDerived.CritRate + 0.002f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatAllocationHudViewModelTest,
	"IdleProject.UI.HUD.StatAllocationPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatAllocationHudViewModelTest::RunTest(const FString& Parameters)
{
	const FPrimaryStats BaseStats(12.0f, 8.0f, 7.0f, 6.0f, 11.0f, 5.0f);
	const FPrimaryStats AllocatedStats(1.0f, 0.0f, 2.0f, 0.0f, 0.0f, 3.0f);

	const FIdleHUDStatPanelViewModel WithPoints = IdleProject::UI::BuildStatPanelViewModel(BaseStats, AllocatedStats, 4);
	TestEqual(TEXT("Stat panel exposes six primary stats"), WithPoints.Rows.Num(), 6);
	TestEqual(TEXT("STR row is first"), WithPoints.Rows[0].Stat, EPrimaryStat::Str);
	TestEqual(TEXT("INT row maps to FPrimaryStats::Int_"), WithPoints.Rows[2].AllocatedValue, 2);
	TestEqual(TEXT("LUK row includes allocated points"), WithPoints.Rows[5].TotalValue, 8);
	TestEqual(TEXT("STR allocation hitbox is stable"), WithPoints.Rows[0].AllocationHitBoxName, FName(TEXT("StatAlloc_0")));
	TestEqual(TEXT("Reset hitbox is stable"), WithPoints.ResetHitBoxName, FName(TEXT("StatReset")));
	TestEqual(TEXT("Available points are clamped into the view model"), WithPoints.AvailablePoints, 4);
	TestTrue(TEXT("Rows can allocate while points remain"), WithPoints.Rows[0].bCanAllocate);
	TestTrue(TEXT("Reset is enabled when any stat is allocated"), WithPoints.bCanReset);

	const FIdleHUDStatPanelViewModel NoPoints = IdleProject::UI::BuildStatPanelViewModel(BaseStats, AllocatedStats, 0);
	TestFalse(TEXT("Rows disable allocation with no available points"), NoPoints.Rows[0].bCanAllocate);
	TestTrue(TEXT("Reset remains enabled with spent points and no available points"), NoPoints.bCanReset);

	const FIdleHUDStatPanelViewModel Empty = IdleProject::UI::BuildStatPanelViewModel(BaseStats, FPrimaryStats(), -3);
	TestEqual(TEXT("Negative available points clamp to zero"), Empty.AvailablePoints, 0);
	TestFalse(TEXT("Reset is disabled when no stat points are allocated"), Empty.bCanReset);

	return true;
}

#endif
