#include "Misc/AutomationTest.h"

#include "RuneSystem/ClassRuneFormula.h"
#include "RuneSystem/RuneFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClassRuneFormulaMappingTest,
	"IdleProject.Rune.ClassFormula.Mapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClassRuneFormulaMappingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Class rune slot index is the seventh slot"), FClassRuneFormula::ClassRuneSlotIndex, 6);

	const float Unit = FRuneFormula::GetCoreRuneMultiplier(EItemRarity::Rare, 3);
	const FRuneCoreMultipliers Warrior = FClassRuneFormula::GetClassMasteryMultipliers(EClassId::Warrior, EItemRarity::Rare, 3);
	TestEqual(TEXT("Warrior class rune grants physical attack"), Warrior.PhysAtk, Unit);
	TestEqual(TEXT("Warrior class rune grants physical defense"), Warrior.PhysDef, Unit);
	TestEqual(TEXT("Warrior class rune does not grant magic attack"), Warrior.MagicAtk, 0.0f);

	const FRuneCoreMultipliers Mage = FClassRuneFormula::GetClassMasteryMultipliers(EClassId::Mage, EItemRarity::Rare, 3);
	TestEqual(TEXT("Mage class rune grants magic attack"), Mage.MagicAtk, Unit);
	TestEqual(TEXT("Mage class rune does not grant physical attack"), Mage.PhysAtk, 0.0f);

	const FRuneCoreMultipliers Cleric = FClassRuneFormula::GetClassMasteryMultipliers(EClassId::Cleric, EItemRarity::Rare, 3);
	TestEqual(TEXT("Cleric class rune grants magic attack"), Cleric.MagicAtk, Unit);
	TestEqual(TEXT("Cleric class rune grants HP"), Cleric.Hp, Unit);

	const FRuneCoreMultipliers Invalid = FClassRuneFormula::GetClassMasteryMultipliers(EClassId::None, EItemRarity::Rare, 3);
	TestEqual(TEXT("Invalid class grants no physical attack"), Invalid.PhysAtk, 0.0f);
	TestEqual(TEXT("Invalid class grants no magic attack"), Invalid.MagicAtk, 0.0f);
	TestEqual(TEXT("Invalid class grants no HP"), Invalid.Hp, 0.0f);

	return true;
}

#endif
