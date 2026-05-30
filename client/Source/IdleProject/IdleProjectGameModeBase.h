#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameCore/MapThemeTypes.h"
#include "GameCore/StageService.h"
#include "IdleProjectGameModeBase.generated.h"

class AIdleMonster;

/**
 * IdleProject 기본 게임 모드입니다.
 * PR #4에서는 전사 캐릭터 1종과 기본 Pawn 연결만 담당합니다.
 */
UCLASS()
class IDLEPROJECT_API AIdleProjectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIdleProjectGameModeBase();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	static bool ShouldSpawnMonsterAsBoss(bool bRequestedBoss, bool bHasStageService, const FStageInfo& StageInfo);

	// 챕터 테마 적용(조명/바닥색/프롭 교체). 헤드리스 테스트 진입점.
	void ApplyMapTheme(int32 Chapter);

	// 헤드리스 테스트용 thin 진입점/게터.
	void SpawnDefaultEnvironmentForTest() { SpawnDefaultEnvironment(); }
	int32 GetThemePropCountForTest() const { return ThemeProps.Num(); }

	// 테스트: 적용된 바닥 컴포넌트의 현재 머티리얼(0) 반환(MID 또는 베이스).
	class UMaterialInterface* GetGroundMaterialForTest() const;
	// 테스트: 바닥 MID의 "Color" 벡터 파라미터 값 조회(MID 아니면 false).
	bool GetGroundColorForTest(FLinearColor& OutColor) const;

	// 테스트: 안개 컴포넌트의 현재 밀도(없으면 -1).
	float GetFogDensityForTest() const;
	// 테스트: 스카이 스피어 MID의 "SkyTint" 벡터 파라미터 값 조회(MID 아니면 false).
	bool GetSkyTintForTest(FLinearColor& OutTint) const;
	// 테스트: 프롭 Index MID의 스칼라 파라미터 값 조회(MID 아니면 false).
	bool GetPropScalarForTest(int32 Index, const FName& Param, float& OutValue) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Battle")
	TSubclassOf<AIdleMonster> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Battle")
	int32 InitialMonsterCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Battle")
	float MonsterRespawnDelay = 5.0f;

private:
	void SpawnInitialMonsters(AController* NewPlayer);
	AIdleMonster* SpawnMonsterAt(const FVector& SpawnLocation, bool bSpawnBoss = false);

	UFUNCTION()
	void ScheduleRespawn(AActor* DyingActor);

	/**
	 * 빈 default world 에 조명/하늘/바닥을 자동 spawn 한다.
	 * Game.umap (BP 자산) 미생성 상태에서도 PIE 가 까만 화면이 아닌 정상 시각화를 보이도록.
	 * PR #7 (hotfix-06-light-sky) 추가. BP 맵 자산 도입 시 본 함수 호출 제거 가능.
	 */
	void SpawnDefaultEnvironment();

	UFUNCTION()
	void HandleStageChangedForTheme(FStageInfo NewStageInfo);

	UPROPERTY()
	TObjectPtr<class ADirectionalLight> ThemeSun = nullptr;
	UPROPERTY()
	TObjectPtr<class ASkyLight> ThemeSky = nullptr;
	UPROPERTY()
	TObjectPtr<class AStaticMeshActor> ThemeGround = nullptr;
	UPROPERTY()
	TArray<TObjectPtr<class AStaticMeshActor>> ThemeProps;

	int32 AppliedThemeChapter = -1;

	// 맵 테마 파라미터 머티리얼(/Game/Maps/M_MapTheme). lazy 로드 캐시.
	// 무효(에셋 부재/로드 실패) 시 기존 베이스 머티리얼 폴백.
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> ThemeMaterial = nullptr;

	bool bThemeMaterialLoadAttempted = false;

	// 안개(ExponentialHeightFog). ApplyMapTheme 에서 챕터별 색/밀도 갱신.
	UPROPERTY()
	TObjectPtr<class AExponentialHeightFog> ThemeFog = nullptr;

	// 전역 PostProcessVolume(Unbound). 따뜻한 Genshin 분위기 그레이드(화이트밸런스/블룸/AO/비네트/채도/대비).
	UPROPERTY(Transient)
	TObjectPtr<class APostProcessVolume> ThemePostProcess = nullptr;

	// 스카이 스피어(역방향 배경, ThemeSky=SkyLight 와 별개). M_Sky MID로 SkyTint 갱신.
	UPROPERTY()
	TObjectPtr<class AStaticMeshActor> ThemeSkySphere = nullptr;

	// 스카이 머티리얼(/Game/Maps/M_Sky) lazy 로드 캐시. 무효 시 폴백.
	UPROPERTY()
	TObjectPtr<class UMaterialInterface> SkyMaterial = nullptr;

	// 스카이 큐브맵(/Game/Maps/TC_MapSky) lazy 로드 캐시. 무효 시 폴백.
	UPROPERTY()
	TObjectPtr<class UTextureCube> SkyCubemap = nullptr;

	bool bSkyAssetsLoadAttempted = false;

	bool bInitialMonstersSpawned = false;
	bool bDefaultEnvironmentSpawned = false;
};
