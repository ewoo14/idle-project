#pragma once

#include "CoreMinimal.h"
#include "MasteryTypes.generated.h"

UENUM(BlueprintType)
enum class EMasteryTrack : uint8
{
	Combat = 0 UMETA(DisplayName = "Combat"),
	Equipment = 1 UMETA(DisplayName = "Equipment"),
	Abyss = 2 UMETA(DisplayName = "Abyss"),
	Rune = 3 UMETA(DisplayName = "Rune"),
	Beast = 4 UMETA(DisplayName = "Beast"),
	Explore = 5 UMETA(DisplayName = "Explore")
};

USTRUCT()
struct IDLEPROJECT_API FMasterySaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 Track = 0;

	UPROPERTY()
	int64 TotalXp = 0;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FMasteryGlobalBonus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	float CoreStatMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	float CritRateAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	float DropRateAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	float GoldFindPct = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	float ExpBoostPct = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
	int64 WorldPower = 0;
};
