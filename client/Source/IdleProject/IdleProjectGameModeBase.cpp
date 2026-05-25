#include "IdleProjectGameModeBase.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "CombatSystem/CombatComponent.h"
#include "TimerManager.h"
#include "UI/IdleHUD.h"

AIdleProjectGameModeBase::AIdleProjectGameModeBase()
{
	DefaultPawnClass = AIdleCharacter::StaticClass();
	HUDClass = AIdleHUD::StaticClass();
	MonsterClass = AIdleMonster::StaticClass();
}

void AIdleProjectGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	SpawnInitialMonsters(NewPlayer);
}

void AIdleProjectGameModeBase::SpawnInitialMonsters(AController* NewPlayer)
{
	if (bInitialMonstersSpawned || !NewPlayer)
	{
		return;
	}

	APawn* PlayerPawn = NewPlayer->GetPawn();
	if (!PlayerPawn)
	{
		return;
	}

	bInitialMonstersSpawned = true;
	const FVector Center = PlayerPawn->GetActorLocation();
	const float Radius = 800.0f;
	for (int32 Index = 0; Index < InitialMonsterCount; ++Index)
	{
		const float Angle = (2.0f * PI * static_cast<float>(Index)) / FMath::Max(1.0f, static_cast<float>(InitialMonsterCount));
		const FVector Offset(FMath::Cos(Angle) * Radius, 0.0f, FMath::Sin(Angle) * 120.0f);
		SpawnMonsterAt(Center + Offset);
	}
}

AIdleMonster* AIdleProjectGameModeBase::SpawnMonsterAt(const FVector& SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World || !MonsterClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AIdleMonster* Monster = World->SpawnActor<AIdleMonster>(MonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParameters);
	if (Monster && Monster->GetCombat())
	{
		Monster->GetCombat()->OnDeath.AddDynamic(this, &AIdleProjectGameModeBase::ScheduleRespawn);
	}
	return Monster;
}

void AIdleProjectGameModeBase::ScheduleRespawn(AActor* DyingActor)
{
	UWorld* World = GetWorld();
	if (!World || !DyingActor)
	{
		return;
	}

	const FVector RespawnLocation = DyingActor->GetActorLocation();
	FTimerHandle RespawnTimerHandle;
	World->GetTimerManager().SetTimer(RespawnTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, RespawnLocation]()
	{
		SpawnMonsterAt(RespawnLocation);
	}), MonsterRespawnDelay, false);
}
