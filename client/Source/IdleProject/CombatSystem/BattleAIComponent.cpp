#include "CombatSystem/BattleAIComponent.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UBattleAIComponent::UBattleAIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBattleAIComponent::StartBattle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	State = EBattleState::Idle;
	World->GetTimerManager().SetTimer(BattleTimerHandle, this, &UBattleAIComponent::UpdateBattle, BattleInterval, true, BattleInterval);
}

void UBattleAIComponent::StopBattle()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BattleTimerHandle);
	}
	State = EBattleState::Idle;
}

AActor* UBattleAIComponent::FindClosestEnemy()
{
	if (!TargetActorClass)
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return nullptr;
	}

	AActor* ClosestActor = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();

	for (TActorIterator<AActor> It(World, TargetActorClass); It; ++It)
	{
		AActor* Candidate = *It;
		if (!IsValid(Candidate) || Candidate == Owner)
		{
			continue;
		}

		const UCombatComponent* CandidateCombat = Candidate->FindComponentByClass<UCombatComponent>();
		if (CandidateCombat && CandidateCombat->IsDead())
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestActor = Candidate;
		}
	}

	return ClosestActor;
}

TArray<AActor*> UBattleAIComponent::FindEnemiesInRange(float Radius) const
{
	TArray<AActor*> Enemies;
	if (!TargetActorClass)
	{
		return Enemies;
	}

	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World)
	{
		return Enemies;
	}

	const float RadiusSq = FMath::Square(FMath::Max(0.0f, Radius));
	for (TActorIterator<AActor> It(World, TargetActorClass); It; ++It)
	{
		AActor* Candidate = *It;
		if (!IsValid(Candidate) || Candidate == Owner)
		{
			continue;
		}

		const UCombatComponent* CandidateCombat = Candidate->FindComponentByClass<UCombatComponent>();
		if (CandidateCombat && CandidateCombat->IsDead())
		{
			continue;
		}

		if (FVector::DistSquared2D(Owner->GetActorLocation(), Candidate->GetActorLocation()) <= RadiusSq)
		{
			Enemies.Add(Candidate);
		}
	}

	return Enemies;
}

void UBattleAIComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBattle();
	Super::EndPlay(EndPlayReason);
}

void UBattleAIComponent::UpdateBattle()
{
	UCombatComponent* OwnerCombat = GetOwnerCombat();
	if (!OwnerCombat || OwnerCombat->IsDead())
	{
		State = EBattleState::Dead;
		StopBattle();
		return;
	}

	AActor* TargetActor = FindClosestEnemy();
	if (!TargetActor)
	{
		State = EBattleState::Idle;
		return;
	}

	const float Distance = FVector::Dist2D(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation());
	if (Distance <= OwnerCombat->AttackRange)
	{
		if (USkillComponent* Skills = GetOwner()->FindComponentByClass<USkillComponent>())
		{
			TArray<AActor*> AoeTargets = FindEnemiesInRange(FMath::Max(OwnerCombat->AttackRange, 400.0f));
			if (AoeTargets.IsEmpty())
			{
				AoeTargets.Add(TargetActor);
			}
			Skills->TickSkills(GetWorld()->GetTimeSeconds(), TargetActor, AoeTargets);
		}
		Attack(TargetActor);
		return;
	}

	State = EBattleState::Chase;
	MoveTowards(TargetActor, BattleInterval);
}

FVector UBattleAIComponent::ComputeGroundChaseLocation(
	const FVector& OwnerLocation,
	const FVector& TargetLocation,
	float DeltaSeconds,
	float Speed)
{
	// 추격 목표는 타깃의 X(화면 가로) 만 취하고, Z(중력축) 와 Y(깊이) 는 추격 주체 자신의 값을 유지한다.
	// → 타깃 캡슐 중심(높음)으로 떠오르지 않고 지면을 따라 수평 이동하며, 높이는 중력/지면이 처리한다.
	const FVector HorizontalTarget(TargetLocation.X, OwnerLocation.Y, OwnerLocation.Z);
	return FMath::VInterpConstantTo(OwnerLocation, HorizontalTarget, DeltaSeconds, Speed);
}

void UBattleAIComponent::MoveTowards(AActor* TargetActor, float DeltaSeconds)
{
	AActor* Owner = GetOwner();
	if (!Owner || !TargetActor)
	{
		return;
	}

	if (AIdleCharacter* IdleCharacter = Cast<AIdleCharacter>(Owner))
	{
		// 플레이어(컨트롤러 보유)는 걷기 이동 입력으로 추격한다 (걷기 모드는 Z 입력을 무시 → 지면 유지).
		// 횡스크롤 X-Z 평면 (Y축 plane normal): 3D 정규화 후 Y 성분만 0 처리.
		FVector Direction = (TargetActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
		Direction.Y = 0.0f;
		Direction = Direction.GetSafeNormal();
		IdleCharacter->AddMovementInput(Direction, 1.0f);
		return;
	}

	// 몬스터(컨트롤러 없음 → AddMovementInput 불가)는 위치 보간으로 추격한다.
	// 타깃 전체 3D 위치로 보간하면 플레이어 캡슐 중심(더 높은 Z)으로 끌려 올라가므로,
	// 수평(X)만 추격하고 Z·Y 는 몬스터 자신의 지면선 값을 유지한다 (몬스터는 GravityScale=0 으로 낙하 없음).
	const FVector NextLocation = ComputeGroundChaseLocation(
		Owner->GetActorLocation(), TargetActor->GetActorLocation(), DeltaSeconds, NonCharacterMoveSpeed);
	Owner->SetActorLocation(NextLocation, true);
}

void UBattleAIComponent::Attack(AActor* TargetActor)
{
	UWorld* World = GetWorld();
	UCombatComponent* OwnerCombat = GetOwnerCombat();
	UCombatComponent* TargetCombat = TargetActor ? TargetActor->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!World || !OwnerCombat || !TargetCombat || TargetCombat->IsDead())
	{
		return;
	}

	const float Cooldown = 1.0f / FMath::Max(0.1f, OwnerCombat->AtkSpeed);
	if (World->GetTimeSeconds() - LastAttackTime < Cooldown)
	{
		State = EBattleState::Attack;
		return;
	}

	LastAttackTime = World->GetTimeSeconds();
	State = EBattleState::Attack;
	const float BaseDamage = FCombatFormulas::ComputeDamage(OwnerCombat->Atk, TargetCombat->Def);
	const bool bWasCrit = OwnerCombat->RollCrit();
	const float FinalDamage = FCombatFormulas::ApplyCrit(BaseDamage, bWasCrit, OwnerCombat->CritDmg);
	TargetCombat->TakeDamageTyped(FinalDamage, GetOwner(), bWasCrit, EDamageKind::Physical);
	if (USkillComponent* Skills = GetOwner()->FindComponentByClass<USkillComponent>())
	{
		Skills->AddGauge(Skills->GetGaugeGainOnHit());
	}
}

UCombatComponent* UBattleAIComponent::GetOwnerCombat() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
}
