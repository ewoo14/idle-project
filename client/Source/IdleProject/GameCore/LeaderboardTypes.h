#pragma once

#include "CoreMinimal.h"
#include "LeaderboardTypes.generated.h"

UENUM(BlueprintType)
enum class ELeaderboardKind : uint8
{
	Power = 0,
	Rebirth = 1,
	WeeklyDamage = 2
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Leaderboard")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Leaderboard")
	int64 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Leaderboard")
	int32 Rank = 0;
};
