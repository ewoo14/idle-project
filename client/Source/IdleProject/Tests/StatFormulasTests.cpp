#include "Misc/AutomationTest.h"
#include "CharacterSystem/StatFormulas.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatFormulasDefaultPrimaryTest,
	"IdleProject.Character.StatFormulas.DefaultPrimaryStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatFormulasDefaultPrimaryTest::RunTest(const FString& Parameters)
{
	const FPrimaryStats WarriorLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	TestEqual(TEXT("전사 레벨 1 STR"), WarriorLevel1.Str, 12);
	TestEqual(TEXT("전사 레벨 1 DEX"), WarriorLevel1.Dex, 6);
	TestEqual(TEXT("전사 레벨 1 INT"), WarriorLevel1.Int_, 3);
	TestEqual(TEXT("전사 레벨 1 WIS"), WarriorLevel1.Wis, 3);
	TestEqual(TEXT("전사 레벨 1 CON"), WarriorLevel1.Con, 10);
	TestEqual(TEXT("전사 레벨 1 LUK"), WarriorLevel1.Luk, 4);

	const FPrimaryStats MageLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Mage, 1);
	TestEqual(TEXT("마법사 레벨 1 INT"), MageLevel1.Int_, 12);
	TestEqual(TEXT("마법사 레벨 1 WIS"), MageLevel1.Wis, 10);
	TestEqual(TEXT("마법사 레벨 1 STR"), MageLevel1.Str, 3);

	const FPrimaryStats ArcherLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Archer, 1);
	TestEqual(TEXT("궁수 레벨 1 DEX"), ArcherLevel1.Dex, 12);
	TestEqual(TEXT("궁수 레벨 1 LUK"), ArcherLevel1.Luk, 10);
	TestEqual(TEXT("궁수 레벨 1 CON"), ArcherLevel1.Con, 6);

	const FPrimaryStats WarriorLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 50);
	TestEqual(TEXT("전사 레벨 50 STR"), WarriorLevel50.Str, 105.1f);
	TestEqual(TEXT("전사 레벨 50 CON"), WarriorLevel50.Con, 88.4f);

	const FPrimaryStats WarriorLevel100 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 100);
	TestEqual(TEXT("전사 레벨 100 STR"), WarriorLevel100.Str, 200.1f);
	TestEqual(TEXT("전사 레벨 100 DEX"), WarriorLevel100.Dex, 95.1f);
	TestEqual(TEXT("전사 레벨 100 CON"), WarriorLevel100.Con, 168.4f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatFormulasDerivedTest,
	"IdleProject.Character.StatFormulas.DeriveStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatFormulasDerivedTest::RunTest(const FString& Parameters)
{
	const FPrimaryStats WarriorLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	const FDerivedStats DerivedLevel1 = FStatFormulas::DeriveStats(WarriorLevel1, 1);
	TestEqual(TEXT("전사 레벨 1 HP"), DerivedLevel1.Hp, 120.0f);
	TestEqual(TEXT("전사 레벨 1 PhysAtk"), DerivedLevel1.PhysAtk, 24.0f);
	TestEqual(TEXT("Accuracy base includes 0.75"), DerivedLevel1.Accuracy, 0.762f);

	const FPrimaryStats WarriorLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 50);
	const FDerivedStats DerivedLevel50 = FStatFormulas::DeriveStats(WarriorLevel50, 50);
	TestEqual(TEXT("전사 레벨 50 HP"), DerivedLevel50.Hp, 1884.0f);
	TestEqual(TEXT("전사 레벨 50 PhysAtk"), DerivedLevel50.PhysAtk, 210.0f);

	FDerivedStats EquipmentBonus;
	EquipmentBonus.CritDmg = 9.0f;
	const FDerivedStats ClampedHighCrit = FStatFormulas::DeriveStats(WarriorLevel1, 1, EquipmentBonus);
	TestEqual(TEXT("CritDmg upper clamp"), ClampedHighCrit.CritDmg, 3.0f);

	EquipmentBonus.CritDmg = -9.0f;
	const FDerivedStats ClampedLowCrit = FStatFormulas::DeriveStats(WarriorLevel1, 1, EquipmentBonus);
	TestEqual(TEXT("CritDmg lower clamp"), ClampedLowCrit.CritDmg, 1.0f);

	return true;
}

#endif
