#include "Misc/AutomationTest.h"

#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "CharacterSystem/IdleCharacter.h"
#include "Components/SceneComponent.h"
#include "DamageReceivedTestReceiver.h"
#include "Engine/World.h"
#include "CharacterSystem/IdleMonster.h"
#include "Internationalization/IdleLocalization.h"
#include "UI/IdleHUD.h"

namespace
{
int32 CountSkillsByType(const USkillComponent& Skills, ESkillType Type)
{
	return Skills.Skills.FilterByPredicate([Type](const FSkillDefinition& Skill)
	{
		return Skill.Type == Type;
	}).Num();
}

bool HasSkill(const USkillComponent& Skills, FName SkillId)
{
	return Skills.Skills.ContainsByPredicate([SkillId](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == SkillId;
	});
}

struct FExpectedSkillDefinition
{
	FName SkillId;
	EClassId ClassId;
	ESkillType Type;
	ESkillEffectType EffectType;
	float Cooldown;
	float DamageCoeff;
	float BuffMagnitude;
	float BuffDuration;
	float GaugeGainOnHit;
	float GaugeGainOnTakeDamage;
};

bool TestSkillDefinitionParity(FAutomationTestBase& Test, const USkillComponent& Skills, const FExpectedSkillDefinition& Expected)
{
	const FSkillDefinition* Actual = Skills.Skills.FindByPredicate([&Expected](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == Expected.SkillId;
	});

	Test.TestNotNull(*FString::Printf(TEXT("%s exists"), *Expected.SkillId.ToString()), Actual);
	if (!Actual)
	{
		return false;
	}

	Test.TestEqual(*FString::Printf(TEXT("%s ClassId"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->ClassId), static_cast<int32>(Expected.ClassId));
	Test.TestEqual(*FString::Printf(TEXT("%s Type"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->Type), static_cast<int32>(Expected.Type));
	Test.TestEqual(*FString::Printf(TEXT("%s EffectType"), *Expected.SkillId.ToString()), static_cast<int32>(Actual->EffectType), static_cast<int32>(Expected.EffectType));
	Test.TestEqual(*FString::Printf(TEXT("%s Cooldown"), *Expected.SkillId.ToString()), Actual->Cooldown, Expected.Cooldown);
	Test.TestEqual(*FString::Printf(TEXT("%s DamageCoeff"), *Expected.SkillId.ToString()), Actual->DamageCoeff, Expected.DamageCoeff);
	Test.TestEqual(*FString::Printf(TEXT("%s BuffMagnitude"), *Expected.SkillId.ToString()), Actual->BuffMagnitude, Expected.BuffMagnitude);
	Test.TestEqual(*FString::Printf(TEXT("%s BuffDuration"), *Expected.SkillId.ToString()), Actual->BuffDuration, Expected.BuffDuration);
	Test.TestEqual(*FString::Printf(TEXT("%s GaugeGainOnHit"), *Expected.SkillId.ToString()), Actual->GaugeGainOnHit, Expected.GaugeGainOnHit);
	Test.TestEqual(*FString::Printf(TEXT("%s GaugeGainOnTakeDamage"), *Expected.SkillId.ToString()), Actual->GaugeGainOnTakeDamage, Expected.GaugeGainOnTakeDamage);
	return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatFormulasTest,
	"IdleProject.Combat.Formulas.ComputeDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatFormulasTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Atk 100 Def 20 damage"), FCombatFormulas::ComputeDamage(100.0f, 20.0f), 88.0f);
	TestEqual(TEXT("Minimum damage guarantee"), FCombatFormulas::ComputeDamage(10.0f, 100.0f), 0.5f);
	TestEqual(TEXT("Zero defense damage"), FCombatFormulas::ComputeDamage(50.0f, 0.0f), 50.0f);
	TestEqual(TEXT("Magic damage uses the same curve"), FCombatFormulas::ComputeMagicDamage(80.0f, 20.0f), 68.0f);

	FRandomStream NeverCritStream(12345);
	FRandomStream AlwaysCritStream(12345);
	TestFalse(TEXT("Zero crit rate never crits"), FCombatFormulas::RollCrit(0.0f, NeverCritStream));
	TestTrue(TEXT("Full crit rate always crits"), FCombatFormulas::RollCrit(1.0f, AlwaysCritStream));
	TestEqual(TEXT("Non crit leaves base damage unchanged"), FCombatFormulas::ApplyCrit(34.0f, false, 1.8f), 34.0f);
	TestEqual(TEXT("Crit multiplies base damage"), FCombatFormulas::ApplyCrit(34.0f, true, 1.8f), 61.2f);
	TestEqual(TEXT("Crit damage below one cannot reduce damage"), FCombatFormulas::ApplyCrit(34.0f, true, 0.5f), 34.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatDamageReceivedEventTest,
	"IdleProject.Combat.Component.DamageReceivedEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatDamageReceivedEventTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* Combat = NewObject<UCombatComponent>(Owner);
	Owner->AddInstanceComponent(Combat);
	Combat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	Combat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);

	Combat->TakeDamageTyped(25.0f, Owner, true, EDamageKind::Magic);

	TestEqual(TEXT("Damage event amount matches final damage"), Receiver->LastAmount, 25.0f);
	TestTrue(TEXT("Damage event captures crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Damage event captures damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatAnyDamageReceivedEventTest,
	"IdleProject.Combat.Component.AnyDamageReceivedEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatAnyDamageReceivedEventTest::RunTest(const FString& Parameters)
{
	AActor* FirstOwner = NewObject<AActor>();
	UCombatComponent* FirstCombat = NewObject<UCombatComponent>(FirstOwner);
	FirstOwner->AddInstanceComponent(FirstCombat);
	FirstCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* RespawnedOwner = NewObject<AActor>();
	UCombatComponent* RespawnedCombat = NewObject<UCombatComponent>(RespawnedOwner);
	RespawnedOwner->AddInstanceComponent(RespawnedCombat);
	RespawnedCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* LastDamagedActor = nullptr;
	float LastAmount = 0.0f;
	bool bLastWasCrit = false;
	EDamageKind LastKind = EDamageKind::Physical;
	int32 EventCount = 0;

	const FDelegateHandle Handle = UCombatComponent::OnAnyDamageReceived.AddLambda(
		[&LastDamagedActor, &LastAmount, &bLastWasCrit, &LastKind, &EventCount](
			AActor* DamagedActor,
			float Amount,
			bool bWasCrit,
			EDamageKind Kind)
		{
			LastDamagedActor = DamagedActor;
			LastAmount = Amount;
			bLastWasCrit = bWasCrit;
			LastKind = Kind;
			++EventCount;
		});

	FirstCombat->TakeDamageTyped(12.0f, FirstOwner, false, EDamageKind::Physical);
	RespawnedCombat->TakeDamageTyped(34.0f, FirstOwner, true, EDamageKind::Magic);

	UCombatComponent::OnAnyDamageReceived.Remove(Handle);

	TestEqual(TEXT("Global damage event captures both original and respawned targets"), EventCount, 2);
	TestEqual(TEXT("Global damage event reports latest damaged actor"), LastDamagedActor, RespawnedOwner);
	TestEqual(TEXT("Global damage event reports amount"), LastAmount, 34.0f);
	TestTrue(TEXT("Global damage event reports crit flag"), bLastWasCrit);
	TestEqual(TEXT("Global damage event reports damage kind"), static_cast<int32>(LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatClassDamageTest,
	"IdleProject.Combat.Formulas.ClassDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatClassDamageTest::RunTest(const FString& Parameters)
{
	FDerivedStats MageStats;
	MageStats.PhysAtk = 12.0f;
	MageStats.MagicAtk = 40.0f;
	TestEqual(TEXT("Mage damage uses magic attack and magic defense"), FCombatFormulas::ComputeDamage(MageStats, EClassId::Mage, 20.0f, 10.0f), 34.0f);

	FDerivedStats ArcherStats;
	ArcherStats.PhysAtk = 40.0f;
	ArcherStats.MagicAtk = 12.0f;
	ArcherStats.CritRate = 0.25f;
	ArcherStats.CritDmg = 1.8f;
	TestEqual(TEXT("Archer damage uses physical attack and physical defense"), FCombatFormulas::ComputeDamage(ArcherStats, EClassId::Archer, 10.0f, 80.0f), 34.0f);

	FDerivedStats WarriorStats;
	WarriorStats.PhysAtk = 40.0f;
	WarriorStats.MagicAtk = 80.0f;
	TestEqual(TEXT("Warrior damage keeps physical attack and physical defense"), FCombatFormulas::ComputeDamage(WarriorStats, EClassId::Warrior, 10.0f, 80.0f), 34.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatComponentExtendedStatsTest,
	"IdleProject.Combat.Component.ExtendedStats",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatComponentExtendedStatsTest::RunTest(const FString& Parameters)
{
	UCombatComponent* Combat = NewObject<UCombatComponent>();
	Combat->InitializeCombat(1000.0f, 50.0f, 20.0f, 1.0f, 80.0f, 30.0f, 0.25f, 1.8f);

	TestEqual(TEXT("Magic attack is initialized"), Combat->MagicAtk, 80.0f);
	TestEqual(TEXT("Magic defense is initialized"), Combat->MagicDef, 30.0f);
	TestEqual(TEXT("Crit rate is initialized"), Combat->CritRate, 0.25f);
	TestEqual(TEXT("Crit damage is initialized"), Combat->CritDmg, 1.8f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillMagicDamageTest,
	"IdleProject.Combat.Skills.MagicDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillMagicDamageTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f, 80.0f, 0.0f, 0.0f, 1.5f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 10.0f, 1.0f, 0.0f, 30.0f, 0.0f, 1.5f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	TargetCombat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);
	Skills->LoadDefaultMageSkills();

	Skills->MarkSkillCast(TEXT("chain_lightning"), 10.0f);
	Skills->MarkSkillCast(TEXT("mana_shield"), 10.0f);
	Skills->MarkSkillCast(TEXT("meteor"), 10.0f);
	Skills->TickSkills(12.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Arcane bolt uses MagicAtk vs MagicDef"), TargetCombat->CurrentHp, 1000.0f - 174.0f);
	TestEqual(TEXT("Magic skill broadcasts final damage amount"), Receiver->LastAmount, 174.0f);
	TestFalse(TEXT("Magic skill broadcasts non-crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Magic skill broadcasts magic damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Magic));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillCritDamageTest,
	"IdleProject.Combat.Skills.CritDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillCritDamageTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 40.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 2.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 10.0f, 1.0f);
	UDamageReceivedTestReceiver* Receiver = NewObject<UDamageReceivedTestReceiver>();
	TargetCombat->OnDamageReceived.AddDynamic(Receiver, &UDamageReceivedTestReceiver::Capture);
	Skills->LoadDefaultArcherSkills();

	Skills->MarkSkillCast(TEXT("arrow_rain"), 10.0f);
	Skills->MarkSkillCast(TEXT("focus"), 10.0f);
	Skills->MarkSkillCast(TEXT("piercing_arrow"), 10.0f);
	Skills->TickSkills(12.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Precision shot applies guaranteed crit after physical mitigation"), TargetCombat->CurrentHp, 1000.0f - 164.0f);
	TestEqual(TEXT("Physical skill broadcasts crit damage amount"), Receiver->LastAmount, 164.0f);
	TestTrue(TEXT("Physical skill broadcasts crit flag"), Receiver->bLastWasCrit);
	TestEqual(TEXT("Physical skill broadcasts physical damage kind"), static_cast<int32>(Receiver->LastKind), static_cast<int32>(EDamageKind::Physical));

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
	FSkillRankPointsTest,
	"IdleProject.Combat.Skills.RankPoints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankPointsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	TestEqual(TEXT("Initial skill points are zero"), Skills->GetSkillPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Initial skill rank is zero"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(0));
	TestFalse(TEXT("Unknown skill cannot rank up"), Skills->CanRankUp(TEXT("missing_skill")));

	Skills->GrantSkillPoint(2);
	Skills->GrantSkillPoint(-1);
	TestEqual(TEXT("Positive grants add points and negative grants are ignored"), Skills->GetSkillPoints(), static_cast<int32>(2));
	TestTrue(TEXT("Known skill can rank up when points are available"), Skills->CanRankUp(HeavyStrike));

	TestTrue(TEXT("First rank up succeeds"), Skills->RankUpSkill(HeavyStrike));
	TestTrue(TEXT("Second rank up succeeds"), Skills->RankUpSkill(HeavyStrike));
	TestEqual(TEXT("Rank up spends one point per rank"), Skills->GetSkillPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Rank is stored per skill"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(2));
	TestFalse(TEXT("Cannot rank up without points"), Skills->RankUpSkill(HeavyStrike));

	Skills->GrantSkillPoint(10);
	for (int32 Index = 0; Index < 8; ++Index)
	{
		Skills->RankUpSkill(HeavyStrike);
	}
	TestEqual(TEXT("Rank is capped at max rank five"), Skills->GetSkillRank(HeavyStrike), static_cast<int32>(5));
	TestFalse(TEXT("Max-rank skill cannot rank up"), Skills->CanRankUp(HeavyStrike));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRankEffectiveValuesTest,
	"IdleProject.Combat.Skills.RankEffectiveValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankEffectiveValuesTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();

	const FName HeavyStrike(TEXT("heavy_strike"));
	Skills->GrantSkillPoint(2);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);

	TestEqual(TEXT("Rank two damage coeff adds twenty percent"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 3.0f);
	TestEqual(TEXT("Rank two cooldown reduces ten percent"), Skills->GetEffectiveCooldown(HeavyStrike), 3.6f);

	Skills->MarkSkillCast(HeavyStrike, 10.0f);
	TestEqual(TEXT("Cooldown remaining uses effective cooldown"), Skills->GetCooldownRemaining(HeavyStrike, 12.0f), 1.6f);
	TestEqual(TEXT("Cooldown ratio uses effective cooldown"), Skills->GetCooldownRatio(HeavyStrike, 12.0f), 1.6f / 3.6f);

	Skills->GrantSkillPoint(3);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);
	Skills->RankUpSkill(HeavyStrike);

	TestEqual(TEXT("Max rank damage coeff adds fifty percent"), Skills->GetEffectiveDamageCoeff(HeavyStrike), 3.75f);
	TestEqual(TEXT("Max rank cooldown reduces twenty five percent"), Skills->GetEffectiveCooldown(HeavyStrike), 3.0f);
	TestEqual(TEXT("Zero-cooldown ultimate remains zero"), Skills->GetEffectiveCooldown(TEXT("berserkers_fury")), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillRankDamageApplicationTest,
	"IdleProject.Combat.Skills.RankDamageApplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillRankDamageApplicationTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* Target = NewObject<AActor>();
	UCombatComponent* TargetCombat = NewObject<UCombatComponent>(Target);
	Target->AddInstanceComponent(TargetCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 0.0f, 1.0f);
	TargetCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();
	Skills->GrantSkillPoint(1);
	Skills->RankUpSkill(TEXT("heavy_strike"));
	Skills->MarkSkillCast(TEXT("whirlwind"), 19.0f);
	Skills->MarkSkillCast(TEXT("shield_up"), 19.0f);
	Skills->MarkSkillCast(TEXT("charge"), 19.0f);

	Skills->TickSkills(20.0f, Target, TArray<AActor*>());

	TestEqual(TEXT("Ranked heavy strike uses effective damage coefficient"), TargetCombat->CurrentHp, 725.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterLevelUpSkillPointTest,
	"IdleProject.Character.LevelUp.GrantsSkillPoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterLevelUpSkillPointTest::RunTest(const FString& Parameters)
{
	AIdleCharacter* Character = NewObject<AIdleCharacter>();
	TestNotNull(TEXT("Idle character is created"), Character);
	if (!Character)
	{
		return false;
	}

	USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	TestNotNull(TEXT("Skill component exists"), Skills);
	if (!Skills)
	{
		return false;
	}

	const int32 BeforePoints = Skills->GetSkillPoints();
	Character->HandleLevelUp(2);

	TestEqual(TEXT("Level-up handler grants one skill point"), Skills->GetSkillPoints(), BeforePoints + 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillClassDefaultsTest,
	"IdleProject.Combat.Skills.ClassDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillClassDefaultsTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();

	Skills->LoadDefaultMageSkills();
	TestEqual(TEXT("Mage has seven skills"), Skills->Skills.Num(), 7);
	TestEqual(TEXT("Mage has four active skills"), CountSkillsByType(*Skills, ESkillType::Active), 4);
	TestEqual(TEXT("Mage has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Mage has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Mage loads arcane bolt"), HasSkill(*Skills, TEXT("arcane_bolt")));
	TestTrue(TEXT("Mage loads meteor"), HasSkill(*Skills, TEXT("meteor")));

	Skills->LoadDefaultArcherSkills();
	TestEqual(TEXT("Archer has seven skills"), Skills->Skills.Num(), 7);
	TestEqual(TEXT("Archer has four active skills"), CountSkillsByType(*Skills, ESkillType::Active), 4);
	TestEqual(TEXT("Archer has two passive skills"), CountSkillsByType(*Skills, ESkillType::Passive), 2);
	TestEqual(TEXT("Archer has one ultimate skill"), CountSkillsByType(*Skills, ESkillType::Ultimate), 1);
	TestTrue(TEXT("Archer loads precision shot"), HasSkill(*Skills, TEXT("precision_shot")));
	TestTrue(TEXT("Archer loads arrow_rain"), HasSkill(*Skills, TEXT("arrow_rain")));

	Skills->LoadSkillsForClass(EClassId::Warrior);
	TestTrue(TEXT("Class loader selects warrior skills"), HasSkill(*Skills, TEXT("heavy_strike")));
	Skills->LoadSkillsForClass(EClassId::Mage);
	TestTrue(TEXT("Class loader selects mage skills"), HasSkill(*Skills, TEXT("arcane_bolt")));
	Skills->LoadSkillsForClass(EClassId::Archer);
	TestTrue(TEXT("Class loader selects archer skills"), HasSkill(*Skills, TEXT("precision_shot")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillDefinitionParityTest,
	"IdleProject.Combat.Skills.DefinitionParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillDefinitionParityTest::RunTest(const FString& Parameters)
{
	USkillComponent* Skills = NewObject<USkillComponent>();

	Skills->LoadDefaultMageSkills();
	const TArray<FExpectedSkillDefinition> MageSkills = {
		{TEXT("arcane_bolt"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.4f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("chain_lightning"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.7f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("mana_shield"), EClassId::Mage, ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.35f, 4.0f, 0.0f, 0.0f},
		{TEXT("meteor"), EClassId::Mage, ESkillType::Active, ESkillEffectType::DamageAoe, 14.0f, 2.8f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("spell_mastery"), EClassId::Mage, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f},
		{TEXT("mana_flow"), EClassId::Mage, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f},
		{TEXT("arcane_overload"), EClassId::Mage, ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.5f, 0.25f, 4.0f, 9.0f, 3.0f},
	};

	for (const FExpectedSkillDefinition& Expected : MageSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

	Skills->LoadDefaultArcherSkills();
	const TArray<FExpectedSkillDefinition> ArcherSkills = {
		{TEXT("precision_shot"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.2f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("arrow_rain"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("focus"), EClassId::Archer, ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f, 0.0f, 0.0f},
		{TEXT("piercing_arrow"), EClassId::Archer, ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{TEXT("critical_eye"), EClassId::Archer, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f},
		{TEXT("quick_draw"), EClassId::Archer, ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f},
		{TEXT("eagle_eye"), EClassId::Archer, ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.0f, 0.25f, 4.0f, 10.0f, 2.0f},
	};

	for (const FExpectedSkillDefinition& Expected : ArcherSkills)
	{
		TestSkillDefinitionParity(*this, *Skills, Expected);
	}

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

	Skills->LoadDefaultMageSkills();
	Stats = FDerivedStats();
	Stats.MagicAtk = 200.0f;
	Stats.Mp = 1000.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Mage spell mastery grants 15 percent magic attack"), Stats.MagicAtk, 230.0f);
	TestEqual(TEXT("Mage mana flow grants 20 percent max MP"), Stats.Mp, 1200.0f);

	Skills->LoadDefaultArcherSkills();
	Stats = FDerivedStats();
	Stats.CritRate = 0.10f;
	Stats.AtkSpeed = 1.0f;
	Skills->ApplyPassivesToStats(Stats);
	TestEqual(TEXT("Archer critical eye grants five percentage points crit"), Stats.CritRate, 0.15f);
	TestEqual(TEXT("Archer quick draw grants 10 percent attack speed"), Stats.AtkSpeed, 1.1f);

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillHudDisplayModelTest,
	"IdleProject.UI.HUD.SkillDisplayModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillHudDisplayModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));

	USkillComponent* Skills = NewObject<USkillComponent>();
	Skills->LoadDefaultWarriorSkills();
	Skills->GrantSkillPoint(1);
	Skills->RankUpSkill(TEXT("heavy_strike"));
	Skills->GrantSkillPoint(1);

	constexpr float CastTime = 10.0f;
	constexpr float Now = 12.0f;
	Skills->MarkSkillCast(TEXT("heavy_strike"), CastTime);
	Skills->AddGauge(100.0f);

	const TArray<FIdleHUDSkillSlotViewModel> Slots = IdleProject::UI::BuildSkillSlotViewModels(*Skills, Now);

	TestEqual(TEXT("Only active skills are shown in HUD slots"), Slots.Num(), 4);
	TestEqual(TEXT("First active skill keeps localized display name"), Slots[0].DisplayName.ToString(), FString(TEXT("강타")));
	TestEqual(TEXT("Cooldown ratio mirrors skill component"), Slots[0].CooldownRatio, Skills->GetCooldownRatio(TEXT("heavy_strike"), Now));
	TestEqual(TEXT("Cooldown remaining mirrors skill component"), Slots[0].CooldownRemaining, Skills->GetCooldownRemaining(TEXT("heavy_strike"), Now));
	TestFalse(TEXT("Cooling skill is not ready"), Slots[0].bReady);
	TestEqual(TEXT("Skill point balance is exposed per slot"), Slots[0].AvailableSkillPoints, static_cast<int32>(1));
	TestEqual(TEXT("Current skill rank is exposed"), Slots[0].Rank, static_cast<int32>(1));
	TestEqual(TEXT("Max skill rank is exposed"), Slots[0].MaxRank, static_cast<int32>(5));
	TestTrue(TEXT("Rank-up availability is exposed"), Slots[0].bCanRankUp);
	TestEqual(TEXT("Unranked active skill still shows zero rank"), Slots[1].Rank, static_cast<int32>(0));

	const FIdleHUDUltimateViewModel Ultimate = IdleProject::UI::BuildUltimateViewModel(*Skills);
	TestEqual(TEXT("Gauge ratio is normalized"), Ultimate.GaugeRatio, 1.0f);
	TestTrue(TEXT("Ultimate ready flag is exposed"), Ultimate.bReady);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FClassSelectionHudDisplayModelTest,
	"IdleProject.UI.HUD.ClassSelectionDisplayModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClassSelectionHudDisplayModelTest::RunTest(const FString& Parameters)
{
	const TArray<FIdleHUDClassSelectionOptionViewModel> Options = IdleProject::UI::BuildClassSelectionOptions(EClassId::Mage);

	TestEqual(TEXT("Class selector exposes three V1 classes"), Options.Num(), 3);
	TestEqual(TEXT("First class is warrior"), static_cast<int32>(Options[0].ClassId), static_cast<int32>(EClassId::Warrior));
	TestEqual(TEXT("Warrior summary highlights STR and CON"), Options[0].StatSummary.ToString(), FString(TEXT("STR/CON")));
	TestEqual(TEXT("Mage summary highlights INT"), Options[1].StatSummary.ToString(), FString(TEXT("INT")));
	TestEqual(TEXT("Archer summary highlights DEX"), Options[2].StatSummary.ToString(), FString(TEXT("DEX")));
	TestTrue(TEXT("Current class is marked selected"), Options[1].bSelected);
	TestFalse(TEXT("Other classes are not selected"), Options[0].bSelected);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterClassSelectionTest,
	"IdleProject.Character.ClassSelection.AppliesSkills",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterClassSelectionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Archer);
	USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();

	TestEqual(TEXT("ClassId stores selected class"), static_cast<int32>(Character->GetClassId()), static_cast<int32>(EClassId::Archer));
	TestNotNull(TEXT("Skill component exists"), Skills);
	TestTrue(TEXT("Archer skill set is loaded"), Skills && HasSkill(*Skills, TEXT("precision_shot")));
	TestFalse(TEXT("Warrior skill set is replaced"), Skills && HasSkill(*Skills, TEXT("heavy_strike")));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSkillAoeTargetsTest,
	"IdleProject.Combat.Skills.AoeTargets",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSkillAoeTargetsTest::RunTest(const FString& Parameters)
{
	AActor* Owner = NewObject<AActor>();
	UCombatComponent* OwnerCombat = NewObject<UCombatComponent>(Owner);
	USkillComponent* Skills = NewObject<USkillComponent>(Owner);
	Owner->AddInstanceComponent(OwnerCombat);
	Owner->AddInstanceComponent(Skills);

	AActor* PrimaryTarget = NewObject<AActor>();
	UCombatComponent* PrimaryCombat = NewObject<UCombatComponent>(PrimaryTarget);
	PrimaryTarget->AddInstanceComponent(PrimaryCombat);

	AActor* SecondaryTarget = NewObject<AActor>();
	UCombatComponent* SecondaryCombat = NewObject<UCombatComponent>(SecondaryTarget);
	SecondaryTarget->AddInstanceComponent(SecondaryCombat);

	OwnerCombat->InitializeCombat(1000.0f, 100.0f, 0.0f, 1.0f);
	PrimaryCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	SecondaryCombat->InitializeCombat(1000.0f, 10.0f, 0.0f, 1.0f);
	Skills->LoadDefaultWarriorSkills();

	Skills->MarkSkillCast(TEXT("heavy_strike"), 10.0f);
	Skills->MarkSkillCast(TEXT("shield_up"), 10.0f);
	Skills->MarkSkillCast(TEXT("charge"), 10.0f);

	TArray<AActor*> AoeTargets;
	AoeTargets.Add(PrimaryTarget);
	AoeTargets.Add(SecondaryTarget);
	Skills->TickSkills(12.0f, PrimaryTarget, AoeTargets);

	TestTrue(TEXT("Whirlwind damages primary target"), PrimaryCombat->CurrentHp < PrimaryCombat->MaxHp);
	TestTrue(TEXT("Whirlwind damages secondary target"), SecondaryCombat->CurrentHp < SecondaryCombat->MaxHp);
	TestEqual(TEXT("Whirlwind applies the same damage to every AoE target"), PrimaryCombat->CurrentHp, SecondaryCombat->CurrentHp);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBattleAIAoeTargetGatheringTest,
	"IdleProject.Combat.BattleAI.AoeTargetGathering",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBattleAIAoeTargetGatheringTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto SpawnPositionedActor = [&World, &SpawnParams](const FVector& Location)
	{
		AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
		USceneComponent* Root = NewObject<USceneComponent>(Actor);
		Actor->SetRootComponent(Root);
		Root->RegisterComponent();
		Actor->SetActorLocation(Location);
		return Actor;
	};

	AActor* Owner = SpawnPositionedActor(FVector(0.0, 0.0, 0.0));
	UBattleAIComponent* BattleAI = NewObject<UBattleAIComponent>(Owner);
	Owner->AddInstanceComponent(BattleAI);
	BattleAI->RegisterComponent();
	BattleAI->TargetActorClass = AActor::StaticClass();

	AActor* NearTarget = SpawnPositionedActor(FVector(150.0, 0.0, 500.0));
	UCombatComponent* NearCombat = NewObject<UCombatComponent>(NearTarget);
	NearTarget->AddInstanceComponent(NearCombat);
	NearCombat->RegisterComponent();
	NearCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* EdgeTarget = SpawnPositionedActor(FVector(399.0, 0.0, -200.0));
	UCombatComponent* EdgeCombat = NewObject<UCombatComponent>(EdgeTarget);
	EdgeTarget->AddInstanceComponent(EdgeCombat);
	EdgeCombat->RegisterComponent();
	EdgeCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* FarTarget = SpawnPositionedActor(FVector(450.0, 0.0, 0.0));
	UCombatComponent* FarCombat = NewObject<UCombatComponent>(FarTarget);
	FarTarget->AddInstanceComponent(FarCombat);
	FarCombat->RegisterComponent();
	FarCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);

	AActor* DeadTarget = SpawnPositionedActor(FVector(100.0, 0.0, 0.0));
	UCombatComponent* DeadCombat = NewObject<UCombatComponent>(DeadTarget);
	DeadTarget->AddInstanceComponent(DeadCombat);
	DeadCombat->RegisterComponent();
	DeadCombat->InitializeCombat(100.0f, 10.0f, 0.0f, 1.0f);
	DeadCombat->TakeDamage(200.0f, Owner);

	const TArray<AActor*> AoeTargets = BattleAI->FindEnemiesInRange(400.0f);

	TestEqual(TEXT("Far target stays outside 400 units on X"), FarTarget->GetActorLocation().X, 450.0, 0.01);
	TestTrue(TEXT("AoE gathering includes a near target"), AoeTargets.Contains(NearTarget));
	TestTrue(TEXT("AoE gathering uses 2D distance and includes edge target"), AoeTargets.Contains(EdgeTarget));
	TestFalse(TEXT("AoE gathering excludes far target"), AoeTargets.Contains(FarTarget));
	TestFalse(TEXT("AoE gathering excludes dead target"), AoeTargets.Contains(DeadTarget));
	TestFalse(TEXT("AoE gathering excludes owner"), AoeTargets.Contains(Owner));

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleMonsterBossConfigTest,
	"IdleProject.Combat.Monster.BossConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleMonsterBossConfigTest::RunTest(const FString& Parameters)
{
	AIdleMonster* Monster = NewObject<AIdleMonster>();
	TestNotNull(TEXT("Monster is created"), Monster);
	if (!Monster)
	{
		return false;
	}

	TestFalse(TEXT("Default monster is not boss"), Monster->IsBoss());
	Monster->SetBoss(true);
	TestTrue(TEXT("Boss flag is stored"), Monster->IsBoss());
	TestTrue(TEXT("Boss max HP is stronger than normal monster HP"), Monster->GetConfiguredMaxHp() > 50.0f);
	TestTrue(TEXT("Boss attack is stronger than normal monster attack"), Monster->GetConfiguredAttack() > 8.0f);

	return true;
}
