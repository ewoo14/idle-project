#include "Misc/AutomationTest.h"

#include "CombatSystem/CombatFormulas.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatFormulasTest,
	"IdleProject.Combat.Formulas.ComputeDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatFormulasTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Atk 100 Def 20 damage"), FCombatFormulas::ComputeDamage(100.0f, 20.0f), 88.0f);
	TestEqual(TEXT("Minimum damage guarantee"), FCombatFormulas::ComputeDamage(10.0f, 100.0f), 0.5f);
	TestEqual(TEXT("Zero defense damage"), FCombatFormulas::ComputeDamage(50.0f, 0.0f), 50.0f);

	return true;
}
