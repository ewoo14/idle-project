#include "Misc/AutomationTest.h"

#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "CombatSystem/SkillComponent.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillCooldownTest,
	"IdleProject.Combat.Skills.CooldownReadiness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillCooldownTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	TestTrue(TEXT("Heavy strike is ready before first cast"), Skills->IsReady(HeavyStrike, 10.0f));
	TestEqual(TEXT("Ready skill has zero cooldown remaining"), Skills->GetCooldownRemaining(HeavyStrike, 10.0f), 0.0f);
	TestEqual(TEXT("Ready skill has zero cooldown ratio"), Skills->GetCooldownRatio(HeavyStrike, 10.0f), 0.0f);

	TestTrue(TEXT("Cast starts cooldown"), Skills->MarkSkillCast(HeavyStrike, 10.0f));
	TestFalse(TEXT("Heavy strike is not ready during cooldown"), Skills->IsReady(HeavyStrike, 12.0f));
	TestEqual(TEXT("Cooldown remaining is seconds until ready"), Skills->GetCooldownRemaining(HeavyStrike, 12.0f), 2.0f);
	TestEqual(TEXT("Cooldown ratio reflects elapsed cooldown"), Skills->GetCooldownRatio(HeavyStrike, 12.0f), 0.5f);
	TestTrue(TEXT("Heavy strike is ready when cooldown expires"), Skills->IsReady(HeavyStrike, 14.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillGaugeTest,
	"IdleProject.Combat.Skills.UltimateGauge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillGaugeTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	TestEqual(TEXT("Initial gauge is empty"), Skills->GetCurrentGauge(), 0.0f);
	Skills->AddGauge(60.0f);
	Skills->AddGauge(60.0f);
	TestEqual(TEXT("Gauge is clamped at 100"), Skills->GetCurrentGauge(), 100.0f);
	TestTrue(TEXT("Ultimate is ready at 100 gauge"), Skills->IsUltimateReady());

	TestTrue(TEXT("Ultimate cast consumes gauge"), Skills->TryConsumeUltimateGauge());
	TestEqual(TEXT("Gauge resets after ultimate"), Skills->GetCurrentGauge(), 0.0f);
	TestFalse(TEXT("Ultimate is no longer ready after reset"), Skills->IsUltimateReady());

	Skills->AddGauge(-20.0f);
	TestEqual(TEXT("Gauge cannot go below zero"), Skills->GetCurrentGauge(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillPassiveStatsTest,
	"IdleProject.Combat.Skills.PassiveStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillPassiveStatsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	FDerivedStats Stats;
	Stats.Hp = 1000.0f;
	Stats.PhysAtk = 200.0f;
	Stats.PhysDef = 50.0f;

	Skills->ApplyPassivesToStats(Stats);

	TestEqual(TEXT("Weapon mastery grants 15 percent physical attack"), Stats.PhysAtk, 230.0f);
	TestEqual(TEXT("Toughness grants 20 percent max HP"), Stats.Hp, 1200.0f);
	TestEqual(TEXT("Unrelated stats stay unchanged"), Stats.PhysDef, 50.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillUltimateBuffTest,
	"IdleProject.Combat.Skills.UltimateBuff",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillUltimateBuffTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 20.0f, 1.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();
	Skills->AddGauge(100.0f);

	TArray<AActor*> AoeTargets;
	Skills->TickSkills(30.0f, Target, AoeTargets);

	TestEqual(TEXT("Ultimate resets gauge"), Skills->GetCurrentGauge(), 0.0f);
	TestEqual(TEXT("Ultimate grants 30 percent attack speed buff"), OwnerCombat->AtkSpeed, 1.3f);
	TestTrue(TEXT("Ultimate deals damage to target"), TargetCombat->CurrentHp < TargetCombat->MaxHp);

	Skills->TickSkills(35.0f, Target, AoeTargets);
	TestEqual(TEXT("Ultimate attack speed buff expires"), OwnerCombat->AtkSpeed, 1.0f);

	return true;
}
