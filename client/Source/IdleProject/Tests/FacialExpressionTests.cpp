#include "Misc/AutomationTest.h"

#include "CharacterSystem/FacialExpressionComponent.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFacialExpressionSetBattleTest,
	"IdleProject.Character.FacialExpression.SetBattle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFacialExpressionSetBattleTest::RunTest(const FString& Parameters)
{
	UFacialExpressionComponent* Facial = NewObject<UFacialExpressionComponent>();
	TestNotNull(TEXT("Facial component is created"), Facial);

	Facial->SetExpression(EFacialExpression::Battle);
	TestEqual(TEXT("Battle expression is stored"), Facial->GetCurrentExpression(), EFacialExpression::Battle);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFacialExpressionTimedHitTest,
	"IdleProject.Character.FacialExpression.TimedHitStoresExpression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFacialExpressionTimedHitTest::RunTest(const FString& Parameters)
{
	UFacialExpressionComponent* Facial = NewObject<UFacialExpressionComponent>();
	TestNotNull(TEXT("Facial component is created"), Facial);

	Facial->SetExpression(EFacialExpression::Hit, 0.5f);
	TestEqual(TEXT("Hit expression is stored before timer reverts"), Facial->GetCurrentExpression(), EFacialExpression::Hit);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFacialExpressionNoneNoOpTest,
	"IdleProject.Character.FacialExpression.NoneNoOp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFacialExpressionNoneNoOpTest::RunTest(const FString& Parameters)
{
	UFacialExpressionComponent* Facial = NewObject<UFacialExpressionComponent>();
	TestNotNull(TEXT("Facial component is created"), Facial);

	Facial->SetExpression(EFacialExpression::Smile);
	Facial->SetExpression(EFacialExpression::None);
	TestEqual(TEXT("None expression keeps previous expression"), Facial->GetCurrentExpression(), EFacialExpression::Smile);

	return true;
}
