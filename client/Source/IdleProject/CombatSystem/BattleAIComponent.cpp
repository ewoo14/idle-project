#include "CombatSystem/BattleAIComponent.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
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
		Attack(TargetActor);
		return;
	}

	State = EBattleState::Chase;
	MoveTowards(TargetActor, BattleInterval);
}

void UBattleAIComponent::MoveTowards(AActor* TargetActor, float DeltaSeconds)
{
	AActor* Owner = GetOwner();
	if (!Owner || !TargetActor)
	{
		return;
	}

	const FVector Direction = (TargetActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
	if (AIdleCharacter* IdleCharacter = Cast<AIdleCharacter>(Owner))
	{
		IdleCharacter->AddMovementInput(Direction, 1.0f);
		return;
	}

	const float MoveSpeed = 220.0f;
	const FVector NextLocation = FMath::VInterpConstantTo(Owner->GetActorLocation(), TargetActor->GetActorLocation(), DeltaSeconds, MoveSpeed);
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
	TargetCombat->TakeDamage(FCombatFormulas::ComputeDamage(OwnerCombat->Atk, TargetCombat->Def), GetOwner());
}

UCombatComponent* UBattleAIComponent::GetOwnerCombat() const
{
	return GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
}
