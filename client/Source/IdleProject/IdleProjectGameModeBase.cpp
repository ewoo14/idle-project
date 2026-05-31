#include "IdleProjectGameModeBase.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "CombatSystem/CombatComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/TextureCube.h"
#include "Engine/StaticMeshActor.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/MapThemeLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
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
	// 기본값: 따뜻한 햇살 색(1.0/0.96/0.88), 강도 4.0, 소프트 섀도우(소스 각도 1.5°).
	ThemeSun = World->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 5000.0f), FRotator(-45.0f, 45.0f, 0.0f), Params);
	if (ThemeSun)
	{
		if (UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(ThemeSun->GetLightComponent()))
		{
			SunComp->SetMobility(EComponentMobility::Movable);
			SunComp->SetIntensity(4.0f);
			SunComp->SetUseTemperature(false);
			// 따뜻한 오후 햇살 색조(황금빛 화이트)
			SunComp->SetLightColor(FLinearColor(1.0f, 0.96f, 0.88f));
			// 소프트 섀도우: 광원 각도 1.5° → 그림자 반음영 확대
			SunComp->SetLightSourceAngle(1.5f);
		}
	}

	// 전역 환경광 SkyLight — 밝은 환경광으로 소프트한 Genshin 분위기
	ThemeSky = World->SpawnActor<ASkyLight>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (ThemeSky)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
		{
			SkyComp->SetMobility(EComponentMobility::Movable);
			SkyComp->SetIntensity(1.5f); // 기존 1.0 → 1.5: 밝은 환경광으로 소프트한 느낌
		}
	}

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

	// 안개(대기 무드). 색/밀도는 ApplyMapTheme에서 챕터별 갱신.
	// 기본 밀도를 낮게 설정해 과도한 안개 없이 대기 원근감만 표현.
	ThemeFog = World->SpawnActor<AExponentialHeightFog>(FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Params);
	if (ThemeFog)
	{
		if (UExponentialHeightFogComponent* FogComp = ThemeFog->GetComponent())
		{
			FogComp->SetFogDensity(0.005f); // 낮은 기본 밀도: 대기 원근감만, 과도한 안개 방지
		}
	}

	// 대기 산란 하늘(SkyAtmosphere). 실제 물리 기반 대기 산란 → 낮/석양/밤 자연스러운 색상 전환.
	// ASkyAtmosphere 는 Engine 모듈(SkyAtmosphereComponent.h) 포함 — 별도 Build.cs 모듈 불필요.
	ThemeAtmosphere = World->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	// 태양을 대기 산란 광원으로 연결(SkyAtmosphere 가 이 DirectionalLight 를 태양으로 인식).
	if (ThemeSun)
	{
		if (UDirectionalLightComponent* SunComp = Cast<UDirectionalLightComponent>(ThemeSun->GetLightComponent()))
		{
			SunComp->SetAtmosphereSunLight(true);
			SunComp->MarkRenderStateDirty();
		}
	}

	// 전역 PostProcessVolume(Unbound) — 따뜻한 Genshin 분위기 그레이드.
	// 화이트밸런스 6800K(따뜻), 블룸 부드럽게, SSAO, 약한 비네트, 채도/대비 미세 부스트.
	ThemePostProcess = World->SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (APostProcessVolume* PP = ThemePostProcess)
	{
		PP->bUnbound = true;
		FPostProcessSettings& S = PP->Settings;
		// 화이트밸런스: 6800K → 약간 따뜻한 황금빛 화이트
		S.bOverride_WhiteTemp = true;                  S.WhiteTemp = 6800.0f;
		// 블룸: 부드러운 빛 번짐(강도 0.62, 임계값 1.0)
		S.bOverride_BloomIntensity = true;             S.BloomIntensity = 0.62f;
		S.bOverride_BloomThreshold = true;             S.BloomThreshold = 1.0f;
		// SSAO: 50% 강도 + 80 unit 반경 → 소프트한 음영
		S.bOverride_AmbientOcclusionIntensity = true;  S.AmbientOcclusionIntensity = 0.5f;
		S.bOverride_AmbientOcclusionRadius = true;     S.AmbientOcclusionRadius = 80.0f;
		// 비네트: 약한 테두리 어둠(0.4)
		S.bOverride_VignetteIntensity = true;          S.VignetteIntensity = 0.4f;
		// 채도 1.08: 살짝 선명한 Genshin 색감
		S.bOverride_ColorSaturation = true;            S.ColorSaturation = FVector4(1.08f, 1.08f, 1.08f, 1.0f);
		// 대비 1.05: 극히 미세한 대비 강화
		S.bOverride_ColorContrast = true;              S.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.0f);

		// 툰 아웃라인 포스트프로세스 머티리얼(깊이 에지 윤곽선) 합성. 에셋 미존재 시 스킵.
		if (UMaterialInterface* Outline = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Render/M_ToonOutline.M_ToonOutline")))
		{
			S.AddBlendable(Outline, 1.0f);
		}
	}

	// 스카이 스피어(역방향 배경). M_Sky MID는 ApplyMapTheme에서 SkyTint 갱신.
	if (UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		ThemeSkySphere = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
		if (ThemeSkySphere)
		{
			ThemeSkySphere->SetMobility(EComponentMobility::Movable);
			if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
			{
				SphereComp->SetStaticMesh(SphereMesh);
				SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			ThemeSkySphere->SetActorScale3D(FVector(-100.0f, -100.0f, -100.0f)); // 음수=역방향(안쪽이 보임)
		}
		// SkyAtmosphere 와 이중 하늘 충돌 방지: 절차적 스카이 스피어 숨김(SkyLight=ThemeSky 는 그대로 유지).
		if (ThemeSkySphere)
		{
			ThemeSkySphere->SetActorHiddenInGame(true);
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
	// 테마 머티리얼 1회 lazy 로드(에셋 부재 시 폴백).
	if (!bThemeMaterialLoadAttempted)
	{
		bThemeMaterialLoadAttempted = true;
		static const FSoftObjectPath ThemeMatPath(TEXT("/Game/Maps/M_MapTheme.M_MapTheme"));
		ThemeMaterial = Cast<UMaterialInterface>(ThemeMatPath.TryLoad());
	}

	// 스카이 에셋 1회 lazy 로드(에셋 부재 시 폴백).
	if (!bSkyAssetsLoadAttempted)
	{
		bSkyAssetsLoadAttempted = true;
		SkyMaterial = Cast<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/Maps/M_Sky.M_Sky")).TryLoad());
		SkyCubemap = Cast<UTextureCube>(FSoftObjectPath(TEXT("/Game/Maps/TC_MapSky.TC_MapSky")).TryLoad());
	}

	// 안개 색/밀도(챕터별).
	if (ThemeFog)
	{
		if (UExponentialHeightFogComponent* FogComp = ThemeFog->GetComponent())
		{
			FogComp->SetFogInscatteringColor(Theme.FogColor);
			FogComp->SetFogDensity(Theme.FogDensity);
		}
	}

	// 스카이 스피어 틴트(에셋 유효 시).
	if (ThemeSkySphere && SkyMaterial)
	{
		if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
		{
			SphereComp->SetMaterial(0, SkyMaterial);
			if (UMaterialInstanceDynamic* SkyMID = SphereComp->CreateAndSetMaterialInstanceDynamic(0))
			{
				SkyMID->SetVectorParameterValue(TEXT("SkyTint"), Theme.SkyTint);
			}
		}
	}

	// SkyLight 큐브맵(에셋 유효 시) — 절차 스카이 환경광.
	if (ThemeSky && SkyCubemap)
	{
		if (USkyLightComponent* SkyComp = ThemeSky->GetLightComponent())
		{
			SkyComp->Cubemap = SkyCubemap;
			SkyComp->SourceType = SLS_SpecifiedCubemap;
			SkyComp->SetLightColor(Theme.SkyColor);
			SkyComp->RecaptureSky();
		}
	}

	// 바닥 색(파라미터 머티리얼 있으면 실제 틴트, 없으면 기존 베이스 best-effort).
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			if (ThemeMaterial)
			{
				GroundMesh->SetMaterial(0, ThemeMaterial);
			}
			if (GroundMesh->GetMaterial(0))
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
			if (ThemeMaterial)
			{
				Comp->SetMaterial(0, ThemeMaterial);
			}
			if (Comp->GetMaterial(0))
			{
				UMaterialInstanceDynamic* MID = Comp->CreateAndSetMaterialInstanceDynamic(0);
				if (MID)
				{
					MID->SetVectorParameterValue(TEXT("Color"), Prop.Color);
					MID->SetScalarParameterValue(TEXT("Metallic"), Prop.Metallic);
					MID->SetScalarParameterValue(TEXT("Roughness"), Prop.Roughness);
					MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Prop.EmissiveStrength);
				}
			}
		}
		Actor->SetActorScale3D(Prop.Scale);
		ThemeProps.Add(Actor);
	}

	AppliedThemeChapter = FMath::Clamp(Chapter, 1, FMapThemeLibrary::ThemeCount);
}

UMaterialInterface* AIdleProjectGameModeBase::GetGroundMaterialForTest() const
{
	if (ThemeGround)
	{
		if (UStaticMeshComponent* GroundMesh = ThemeGround->GetStaticMeshComponent())
		{
			return GroundMesh->GetMaterial(0);
		}
	}
	return nullptr;
}

bool AIdleProjectGameModeBase::GetGroundColorForTest(FLinearColor& OutColor) const
{
	if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(GetGroundMaterialForTest()))
	{
		return MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Color")), OutColor);
	}
	return false;
}

float AIdleProjectGameModeBase::GetFogDensityForTest() const
{
	if (ThemeFog)
	{
		if (UExponentialHeightFogComponent* FogComp = ThemeFog->GetComponent())
		{
			return FogComp->FogDensity;
		}
	}
	return -1.0f;
}

bool AIdleProjectGameModeBase::GetSkyTintForTest(FLinearColor& OutTint) const
{
	if (ThemeSkySphere)
	{
		if (UStaticMeshComponent* SphereComp = ThemeSkySphere->GetStaticMeshComponent())
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(SphereComp->GetMaterial(0)))
			{
				return MID->GetVectorParameterValue(FMaterialParameterInfo(TEXT("SkyTint")), OutTint);
			}
		}
	}
	return false;
}

bool AIdleProjectGameModeBase::GetPropScalarForTest(int32 Index, const FName& Param, float& OutValue) const
{
	if (ThemeProps.IsValidIndex(Index) && ThemeProps[Index])
	{
		if (UStaticMeshComponent* Comp = ThemeProps[Index]->GetStaticMeshComponent())
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Comp->GetMaterial(0)))
			{
				return MID->GetScalarParameterValue(FMaterialParameterInfo(Param), OutValue);
			}
		}
	}
	return false;
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
		Monster->SetResistElement(StageInfo.ResistElement);
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
