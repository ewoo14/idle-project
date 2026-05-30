#include "IdleProjectGameModeBase.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "CombatSystem/CombatComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/MapThemeLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameCore/StageFormula.h"
#include "GameCore/StageService.h"
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

bool AIdleProjectGameModeBase::ShouldSpawnMonsterAsBoss(bool bRequestedBoss, bool bHasStageService, const FStageInfo& StageInfo)
{
	return bHasStageService ? StageInfo.bBossStage : bRequestedBoss;
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

	// 태양 역할의 DirectionalLight (Movable + 따뜻한 색온도). 멤버에 보관해 테마 적용 시 갱신.
	ThemeSun = World->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 5000.0f), FRotator(-45.0f, 45.0f, 0.0f), Params);
	if (ThemeSun)
	{
		if (ULightComponent* SunComp = ThemeSun->GetLightComponent())
		{
			SunComp->SetMobility(EComponentMobility::Movable);
			SunComp->SetIntensity(5.0f);
			SunComp->SetUseTemperature(true);
			SunComp->SetTemperature(5500.0f);
		}
	}

	// 전역 환경광 SkyLight
	ThemeSky = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (ThemeSky)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
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
		ThemeGround = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector(0.0f, 0.0f, -100.0f), FRotator::ZeroRotator, Params);
		if (ThemeGround)
		{
			ThemeGround->SetMobility(EComponentMobility::Static);
			if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
			{
				GroundMesh->SetStaticMesh(PlaneMesh);
				ThemeGround->SetActorScale3D(FVector(50.0f, 50.0f, 1.0f));
			}
		}
	}

	// 챕터 테마 초기 적용 + 챕터 전환 바인딩. Sun/Sky/Ground 가 이미 spawn 된 뒤이며,
	// StageService 미확보(테스트 등)면 바인딩 skip — ApplyMapTheme 직접 호출로 검증 가능.
	if (UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>())
	{
		if (UStageService* StageService = GameInstance->GetStageService())
		{
			StageService->OnStageChanged.AddDynamic(this, &AIdleProjectGameModeBase::HandleStageChangedForTheme);
			ApplyMapTheme(StageService->GetCurrentChapter());
		}
	}
}

void AIdleProjectGameModeBase::ApplyMapTheme(int32 Chapter)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const FMapTheme Theme = FMapThemeLibrary::GetTheme(Chapter);

	// 조명(주 무드).
	if (ThemeSun)
	{
		if (ULightComponent* SunComp = ThemeSun->GetLightComponent())
		{
			SunComp->SetUseTemperature(false);
			SunComp->SetLightColor(Theme.SunColor);
			SunComp->SetIntensity(Theme.SunIntensity);
		}
		ThemeSun->SetActorRotation(Theme.SunRotation);
	}
	if (ThemeSky)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
		{
			SkyComp->SetIntensity(Theme.SkyIntensity);
			SkyComp->SetLightColor(Theme.SkyColor);
		}
	}
	// 바닥 색(best-effort MID 틴트 — 파라미터 머티리얼 없으면 시각 no-op).
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			if (UMaterialInterface* Base = GroundMesh->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = GroundMesh->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Theme.GroundColor);
				}
			}
		}
	}

	// 프롭 교체: 기존 제거 → 신규 spawn.
	for (AStaticMeshActor* Old : ThemeProps)
	{
		if (Old)
		{
			Old->Destroy();
		}
	}
	ThemeProps.Reset();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	for (const FMapProp& Prop : Theme.Props)
	{
		UStaticMesh* Mesh = Prop.Mesh.LoadSynchronous();
		if (!Mesh)
		{
			continue; // 로드 실패 skip(크래시 가드)
		}
		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Prop.Location, Prop.Rotation, Params);
		if (!Actor)
		{
			continue;
		}
		Actor->SetMobility(EComponentMobility::Movable);
		if (UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent())
		{
			Comp->SetStaticMesh(Mesh);
			Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 게임플레이 무간섭
			if (UMaterialInterface* Base = Comp->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = Comp->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Prop.Color);
				}
			}
		}
		Actor->SetActorScale3D(Prop.Scale);
		ThemeProps.Add(Actor);
	}

	AppliedThemeChapter = FMath::Clamp(Chapter, 1, FMapThemeLibrary::ThemeCount);
}

void AIdleProjectGameModeBase::HandleStageChangedForTheme(FStageInfo NewStageInfo)
{
	if (NewStageInfo.Chapter != AppliedThemeChapter)
	{
		ApplyMapTheme(NewStageInfo.Chapter);
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

	// 횡스크롤 평면 스테이지: 몬스터를 플레이어와 같은 지면선(Z)에 좌우로 번갈아 X 로 퍼뜨린다.
	// (기존 sin(angle)*120 Z 변동은 일부 몬스터를 공중/바닥 아래에 spawn → 중력으로 월드 밖까지 떨어져 사라짐.)
	const FVector PlayerLocation = PlayerPawn->GetActorLocation();
	float PlayerHalfHeight = 88.0f;
	if (const ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerPawn))
	{
		PlayerHalfHeight = PlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	// 몬스터 캡슐 half-height(44)의 발을 플레이어 발 높이에 맞춘다.
	constexpr float MonsterHalfHeight = 44.0f;
	const float GroundAlignedZ = PlayerLocation.Z - (PlayerHalfHeight - MonsterHalfHeight);

	for (int32 Index = 0; Index < InitialMonsterCount; ++Index)
	{
		const float Side = (Index % 2 == 0) ? 1.0f : -1.0f;
		const int32 Rank = Index / 2;
		const float DistanceX = 350.0f + 300.0f * static_cast<float>(Rank);
		const FVector SpawnLocation(PlayerLocation.X + Side * DistanceX, PlayerLocation.Y, GroundAlignedZ);
		SpawnMonsterAt(SpawnLocation, Index == InitialMonsterCount - 1);
	}
}

AIdleMonster* AIdleProjectGameModeBase::SpawnMonsterAt(const FVector& SpawnLocation, bool bSpawnBoss)
{
	UWorld* World = GetWorld();
	if (!World || !MonsterClass)
	{
		return nullptr;
	}

	AIdleMonster* Monster = World->SpawnActorDeferred<AIdleMonster>(
		MonsterClass,
		FTransform(FRotator::ZeroRotator, SpawnLocation),
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (Monster)
	{
		const UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>();
		const UStageService* StageService = GameInstance ? GameInstance->GetStageService() : nullptr;
		const FStageInfo StageInfo = StageService ? StageService->GetCurrentStageInfo() : FStageInfo();
		const bool bEffectiveBoss = ShouldSpawnMonsterAsBoss(bSpawnBoss, StageService != nullptr, StageInfo);

		Monster->SetBoss(bEffectiveBoss);
		Monster->SetElite(StageInfo.bEliteStage);
		Monster->SetStageStatMultiplier(FStageFormula::ComputeMonsterStatMultiplier(StageInfo.GlobalStageIndex));
		Monster->SetStageGlobalIndex(StageInfo.GlobalStageIndex);
		Monster->SetWeakElement(StageInfo.WeakElement);
		Monster->FinishSpawning(FTransform(FRotator::ZeroRotator, SpawnLocation));
		if (Monster->GetCombat())
		{
			Monster->GetCombat()->OnDeath.AddDynamic(this, &AIdleProjectGameModeBase::ScheduleRespawn);
		}
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
	const AIdleMonster* DyingMonster = Cast<AIdleMonster>(DyingActor);
	const bool bWasBoss = DyingMonster && DyingMonster->IsBoss();

	if (UIdleGameInstance* GameInstance = GetGameInstance<UIdleGameInstance>())
	{
		if (UStageService* StageService = GameInstance->GetStageService())
		{
			const int32 PreviousGlobalStageIndex = StageService->GetGlobalStageIndex();
			const bool bHadFinalChapterCleared = StageService->HasFinalChapterCleared();
			const bool bNextWasBoss = StageService->IsNextStageBoss();
			StageService->RecordKill(bWasBoss);
			const int32 NewGlobalStageIndex = StageService->GetGlobalStageIndex();
			const bool bAdvanced = NewGlobalStageIndex > PreviousGlobalStageIndex;
			if (NewGlobalStageIndex != PreviousGlobalStageIndex
				|| (!bHadFinalChapterCleared && StageService->HasFinalChapterCleared()))
			{
				GameInstance->RecordAchievementMetric(EAchievementMetric::StagesCleared, 1);
			}
			if (bAdvanced)
			{
				GameInstance->ApplyProgressionPolicyAfterAdvance(PreviousGlobalStageIndex, bNextWasBoss);
			}
		}
		if (bWasBoss)
		{
			GameInstance->RecordAchievementMetric(EAchievementMetric::BossesKilled, 1);
		}
		GameInstance->RecordMonsterKilled();
		// 자동 버프 유지(P4): ON 시 만료 버프를 보유분으로 자동 재사용.
		GameInstance->MaintainBuffsIfEnabled();
	}

	FTimerHandle RespawnTimerHandle;
	World->GetTimerManager().SetTimer(RespawnTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, RespawnLocation]()
	{
		SpawnMonsterAt(RespawnLocation, false);
	}), MonsterRespawnDelay, false);
}
