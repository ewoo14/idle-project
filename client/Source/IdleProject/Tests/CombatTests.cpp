#include "Misc/AutomationTest.h"

#include "CombatSystem/BattleAIComponent.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIGroundChaseTest,
	"IdleProject.Combat.BattleAI.GroundChaseLocation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIGroundChaseTest::RunTest(const FString& Parameters)
{
	// FVector 성분은 double(FVector3d) 이므로 기대값·허용오차도 double 로 맞춘다 (오버로드 모호성 회피).
	constexpr double Tolerance = 0.01;

	// 몬스터(Z=-56)가 더 높은 캡슐 중심의 타깃(Z=-12, Y=50)을 추격해도
	// 자신의 Z·Y 는 유지하고 X 로만 전진해야 한다 (위로 떠오르지 않음).
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(1000.0, 50.0, -12.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("Z 는 추격 주체 값 유지 (위로 끌려가지 않음)"), Next.Z, -56.0, Tolerance);
		TestEqual(TEXT("Y 는 추격 주체 값 유지 (깊이 고정)"), Next.Y, 0.0, Tolerance);
		TestEqual(TEXT("X 는 속도×시간(=44)만큼 타깃 방향 전진"), Next.X, 44.0, Tolerance);
	}

	// 음(-)의 X 방향 타깃: X 만 음수로 전진, Z 유지.
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(-1000.0, 0.0, 300.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("음수 X 방향 전진"), Next.X, -44.0, Tolerance);
		TestEqual(TEXT("높은 타깃이어도 Z 유지"), Next.Z, -56.0, Tolerance);
	}

	// 한 스텝 내 도달 가능한 가까운 타깃: X 로 정확히 도달, Z 유지.
	{
		const FVector Owner(0.0, 0.0, -56.0);
		const FVector Target(10.0, 0.0, 500.0);
		const FVector Next = UBattleAIComponent::ComputeGroundChaseLocation(Owner, Target, 0.2f, 220.0f);

		TestEqual(TEXT("가까운 타깃 X 도달"), Next.X, 10.0, Tolerance);
		TestEqual(TEXT("도달 시에도 Z 유지"), Next.Z, -56.0, Tolerance);
	}

	return true;
}
