#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

	bool bInitialMonstersSpawned = false;
	bool bDefaultEnvironmentSpawned = false;
};
