#include "IdleProjectGameModeBase.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "CombatSystem/CombatComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "TimerManager.h"
#include "UI/IdleHUD.h"
#include "UObject/ConstructorHelpers.h"

AIdleProjectGameModeBase::AIdleProjectGameModeBase()
{
	DefaultPawnClass = AIdleCharacter::StaticClass();
	HUDClass = AIdleHUD::StaticClass();
	MonsterClass = AIdleMonster::StaticClass();
}

void AIdleProjectGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	SpawnDefaultEnvironment();
	SpawnInitialMonsters(NewPlayer);
}

void AIdleProjectGameModeBase::SpawnDefaultEnvironment()
{
	UWorld* World = GetWorld();
	if (!World || bDefaultEnvironmentSpawned)
	{
		return;
	}
	bDefaultEnvironmentSpawned = true;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 태양 역할의 DirectionalLight (Movable + 따뜻한 색온도)
	if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 5000.0f), FRotator(-45.0f, 45.0f, 0.0f), Params))
	{
		if (ULightComponent* SunComp = Sun->GetLightComponent())
		{
			SunComp->SetMobility(EComponentMobility::Movable);
			SunComp->SetIntensity(5.0f);
			SunComp->SetUseTemperature(true);
			SunComp->SetTemperature(5500.0f);
		}
	}

	// 전역 환경광 SkyLight
	if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params))
	{
		if (USkyLightComponent* SkyComp = Sky->GetLightComponent())
		{
			SkyComp->SetMobility(EComponentMobility::Movable);
			SkyComp->SetIntensity(1.0f);
		}
	}

	// SkyAtmosphere 는 UE 5.7 모듈 의존성 추가 필요 — DirectionalLight + SkyLight 만으로 충분히 가시화.
	// 후속 PR 에서 BP 자산 (W_MainMenu.umap 등) 도입 시 함께 추가.

	// 바닥 — Engine 기본 Plane mesh 를 50배 확대 (5000 unit 반경)
	if (UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		if (AStaticMeshActor* Ground = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector(0.0f, 0.0f, -100.0f), FRotator::ZeroRotator, Params))
		{
			Ground->SetMobility(EComponentMobility::Static);
			if (UStaticMeshComponent* GroundMesh = Ground->GetStaticMeshComponent())
			{
				GroundMesh->SetStaticMesh(PlaneMesh);
				Ground->SetActorScale3D(FVector(50.0f, 50.0f, 1.0f));
			}
		}
	}
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
