#include "Misc/AutomationTest.h"

#include "RuneSystem/RuneSetFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetFormulaTierTest,
	"IdleProject.Rune.SetFormula.Tiers",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetFormulaTierTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("One rune has no set bonus"), FRuneSetFormula::GetSetTierBonus(1), 0.0f);
	TestEqual(TEXT("Two runes activate tier one"), FRuneSetFormula::GetSetTierBonus(2), 0.05f);
	TestEqual(TEXT("Four runes activate tier two"), FRuneSetFormula::GetSetTierBonus(4), 0.12f);
	TestEqual(TEXT("Six runes activate tier three"), FRuneSetFormula::GetSetTierBonus(6), 0.25f);
	TestEqual(TEXT("More than six runes keep tier three"), FRuneSetFormula::GetSetTierBonus(7), 0.25f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetFormulaComputeTest,
	"IdleProject.Rune.SetFormula.Compute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetFormulaComputeTest::RunTest(const FString& Parameters)
{
	TMap<ERuneSet, int32> Counts;
	Counts.Add(ERuneSet::Offense, 6);
	Counts.Add(ERuneSet::Bastion, 4);
	Counts.Add(ERuneSet::Vitality, 2);
	Counts.Add(ERuneSet::Fortune, 1);

	FRuneCoreMultipliers Core;
	FRuneUtilValues Util;
	FRuneSetFormula::ComputeSetBonus(Counts, Core, Util);

	TestEqual(TEXT("Offense adds physical attack set bonus"), Core.PhysAtk, 0.25f);
	TestEqual(TEXT("Offense adds magic attack set bonus"), Core.MagicAtk, 0.25f);
	TestEqual(TEXT("Bastion adds physical defense set bonus"), Core.PhysDef, 0.12f);
	TestEqual(TEXT("Bastion adds magic defense set bonus"), Core.MagicDef, 0.12f);
	TestEqual(TEXT("Vitality adds HP set bonus"), Core.Hp, 0.05f);
	TestEqual(TEXT("Vitality adds offline efficiency set bonus"), Util.OfflineEff, 0.05f);
	TestEqual(TEXT("Under-threshold Fortune adds no gold find"), Util.GoldFind, 0.0f);
	TestEqual(TEXT("Under-threshold Fortune adds no exp boost"), Util.ExpBoost, 0.0f);
	TestEqual(TEXT("Under-threshold Fortune adds no crit damage"), Util.CritDamage, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRuneSetFormulaRollTest,
	"IdleProject.Rune.SetFormula.Roll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRuneSetFormulaRollTest::RunTest(const FString& Parameters)
{
	FRandomStream CommonSeed(64);
	TestEqual(TEXT("Common runes roll no set"), FRuneSetFormula::RollRuneSet(EItemRarity::Common, CommonSeed), ERuneSet::None);

	FRandomStream First(6401);
	FRandomStream Second(6401);
	const ERuneSet FirstSet = FRuneSetFormula::RollRuneSet(EItemRarity::Rare, First);
	const ERuneSet SecondSet = FRuneSetFormula::RollRuneSet(EItemRarity::Rare, Second);
	TestEqual(TEXT("Rune set roll is deterministic by seed"), FirstSet, SecondSet);
	TestTrue(TEXT("Rare+ rune set roll stays in valid enum range"), FirstSet >= ERuneSet::None && FirstSet <= ERuneSet::Fortune);

	return true;
}

#endif
