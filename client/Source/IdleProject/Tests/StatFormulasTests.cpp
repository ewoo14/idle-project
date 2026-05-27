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
	TestEqual(TEXT("전사 레벨 1 STR"), WarriorLevel1.Str, 12.0f);
	TestEqual(TEXT("전사 레벨 1 DEX"), WarriorLevel1.Dex, 6.0f);
	TestEqual(TEXT("전사 레벨 1 INT"), WarriorLevel1.Int_, 3.0f);
	TestEqual(TEXT("전사 레벨 1 WIS"), WarriorLevel1.Wis, 3.0f);
	TestEqual(TEXT("전사 레벨 1 CON"), WarriorLevel1.Con, 10.0f);
	TestEqual(TEXT("전사 레벨 1 LUK"), WarriorLevel1.Luk, 4.0f);

	const FPrimaryStats MageLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Mage, 1);
	TestEqual(TEXT("마법사 레벨 1 INT"), MageLevel1.Int_, 12.0f);
	TestEqual(TEXT("마법사 레벨 1 WIS"), MageLevel1.Wis, 10.0f);
	TestEqual(TEXT("마법사 레벨 1 STR"), MageLevel1.Str, 3.0f);

	const FPrimaryStats ArcherLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Archer, 1);
	TestEqual(TEXT("궁수 레벨 1 DEX"), ArcherLevel1.Dex, 12.0f);
	TestEqual(TEXT("궁수 레벨 1 LUK"), ArcherLevel1.Luk, 10.0f);
	TestEqual(TEXT("궁수 레벨 1 CON"), ArcherLevel1.Con, 6.0f);

	const FPrimaryStats PaladinLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Paladin, 1);
	TestEqual(TEXT("Paladin level 1 STR"), PaladinLevel1.Str, 11.0f);
	TestEqual(TEXT("Paladin level 1 CON"), PaladinLevel1.Con, 13.0f);
	TestEqual(TEXT("Paladin level 1 WIS"), PaladinLevel1.Wis, 6.0f);

	const FPrimaryStats BerserkerLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Berserker, 1);
	TestEqual(TEXT("Berserker level 1 STR"), BerserkerLevel1.Str, 14.0f);
	TestEqual(TEXT("Berserker level 1 CON"), BerserkerLevel1.Con, 7.0f);
	TestEqual(TEXT("Berserker level 1 WIS"), BerserkerLevel1.Wis, 2.0f);

	const FPrimaryStats SummonerLevel1 = FStatFormulas::DefaultPrimaryStats(EClassId::Summoner, 1);
	TestEqual(TEXT("Summoner level 1 INT"), SummonerLevel1.Int_, 13.0f);
	TestEqual(TEXT("Summoner level 1 WIS"), SummonerLevel1.Wis, 11.0f);
	TestEqual(TEXT("Summoner level 1 CON"), SummonerLevel1.Con, 5.0f);

	const FPrimaryStats WarriorLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 50);
	TestEqual(TEXT("전사 레벨 50 STR"), WarriorLevel50.Str, 105.1f);
	TestEqual(TEXT("전사 레벨 50 CON"), WarriorLevel50.Con, 88.4f);

	const FPrimaryStats WarriorLevel100 = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 100);
	TestEqual(TEXT("전사 레벨 100 STR"), WarriorLevel100.Str, 200.1f);
	TestEqual(TEXT("전사 레벨 100 DEX"), WarriorLevel100.Dex, 95.1f);
	TestEqual(TEXT("전사 레벨 100 CON"), WarriorLevel100.Con, 168.4f);

	const FPrimaryStats PaladinLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Paladin, 50);
	TestEqual(TEXT("Paladin level 50 STR"), PaladinLevel50.Str, 94.3f);
	TestEqual(TEXT("Paladin level 50 CON"), PaladinLevel50.Con, 118.4f);

	const FPrimaryStats BerserkerLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Berserker, 50);
	TestEqual(TEXT("Berserker level 50 STR"), BerserkerLevel50.Str, 112.0f);
	TestEqual(TEXT("Berserker level 50 CON"), BerserkerLevel50.Con, 65.8f);
	TestEqual(TEXT("Berserker level 50 LUK"), BerserkerLevel50.Luk, 65.8f);

	const FPrimaryStats SummonerLevel50 = FStatFormulas::DefaultPrimaryStats(EClassId::Summoner, 50);
	TestEqual(TEXT("Summoner level 50 INT"), SummonerLevel50.Int_, 108.6f);
	TestEqual(TEXT("Summoner level 50 WIS"), SummonerLevel50.Wis, 99.2f);
	TestEqual(TEXT("Summoner level 50 CON"), SummonerLevel50.Con, 55.0f);

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

	const FDerivedStats Rebirth0 = FStatFormulas::DeriveStats(WarriorLevel1, 1, FDerivedStats(), 0);
	TestEqual(TEXT("Rebirth 0 points keeps base HP"), Rebirth0.Hp, 120.0f);
	TestEqual(TEXT("Rebirth 0 points keeps base PhysAtk"), Rebirth0.PhysAtk, 24.0f);

	const FDerivedStats Rebirth5 = FStatFormulas::DeriveStats(WarriorLevel1, 1, FDerivedStats(), 5);
	TestEqual(TEXT("Rebirth 5 points adds 50 HP"), Rebirth5.Hp, 170.0f);
	TestEqual(TEXT("Rebirth 5 points adds 10 PhysAtk"), Rebirth5.PhysAtk, 34.0f);

	const FDerivedStats Rebirth10 = FStatFormulas::DeriveStats(WarriorLevel1, 1, FDerivedStats(), 10);
	TestEqual(TEXT("Rebirth 10 points adds 100 HP"), Rebirth10.Hp, 220.0f);
	TestEqual(TEXT("Rebirth 10 points adds 20 PhysAtk"), Rebirth10.PhysAtk, 44.0f);

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

	FDerivedStats AffixBonus;
	AffixBonus.CritRate = 0.02f;
	AffixBonus.AtkSpeed = 0.10f;
	AffixBonus.MagicAtk = 5.0f;
	const FDerivedStats AffixDerived = FStatFormulas::DeriveStats(WarriorLevel1, 1, AffixBonus);
	TestEqual(TEXT("Equipment crit affix increases final crit rate"), AffixDerived.CritRate, 0.028f);
	TestEqual(TEXT("Equipment attack speed affix increases final attack speed"), AffixDerived.AtkSpeed, 1.1f);
	TestEqual(TEXT("Equipment magic attack affix increases final magic attack"), AffixDerived.MagicAtk, 13.0f);

	FDerivedStats OversizedAffixBonus;
	OversizedAffixBonus.CritRate = 3.0f;
	OversizedAffixBonus.AtkSpeed = 9.0f;
	const FDerivedStats ClampedAffixDerived = FStatFormulas::DeriveStats(WarriorLevel1, 1, OversizedAffixBonus);
	TestEqual(TEXT("Equipment crit affix keeps final crit clamp"), ClampedAffixDerived.CritRate, 1.0f);
	TestEqual(TEXT("Equipment attack speed affix keeps final attack speed clamp"), ClampedAffixDerived.AtkSpeed, 3.0f);

	return true;
}

#endif
