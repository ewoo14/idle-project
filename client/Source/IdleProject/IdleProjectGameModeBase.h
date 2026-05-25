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
	AIdleMonster* SpawnMonsterAt(const FVector& SpawnLocation);

	UFUNCTION()
	void ScheduleRespawn(AActor* DyingActor);

	bool bInitialMonstersSpawned = false;
};
