#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "GameFramework/SaveGame.h"
#include "IdleSaveGame.generated.h"

UCLASS()
class IDLEPROJECT_API UIdleSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	bool bHasSave = false;

	UPROPERTY()
	int64 Gold = 0;

	UPROPERTY()
	int32 CharacterLevel = 1;

	UPROPERTY()
	int64 CurrentExp = 0;

	UPROPERTY()
	int64 NextExp = 150;

	UPROPERTY()
	int32 RebirthCount = 0;

	UPROPERTY()
	int32 RebirthBonusPoints = 0;

	UPROPERTY()
	int32 TranscendCount = 0;

	UPROPERTY()
	int32 AvailableStatPoints = 0;

	UPROPERTY()
	FPrimaryStats AllocatedStats;

	UPROPERTY()
	bool bChapter1BossDefeated = false;

	UPROPERTY()
	int64 LastSeenUnixSec = 0;

	UPROPERTY()
	int32 StageChapter = 1;

	UPROPERTY()
	int32 StageStage = 1;

	UPROPERTY()
	int32 StageKillsThisStage = 0;

	UPROPERTY()
	bool bStageFinalChapterCleared = false;

	UPROPERTY()
	int32 StageHighestClearedChapter = 0;

	UPROPERTY()
	int32 TowerHighestFloor = 0;

	UPROPERTY()
	FString EquippedPetId;

	UPROPERTY()
	TMap<FString, int32> PetLevels;
};
