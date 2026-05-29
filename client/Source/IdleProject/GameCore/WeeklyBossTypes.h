#pragma once

#include "CoreMinimal.h"
#include "WeeklyBossTypes.generated.h"

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FWeeklyBossSaveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	FString WeekId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int64 Damage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int32 ChallengesUsed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int32 ClaimedMilestones = 0;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FWeeklyBossChallengeResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	FString WeekId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int64 DamageDealt = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int64 TotalDamage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int32 ChallengesUsed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int32 RemainingChallenges = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|WeeklyBoss")
	int32 ReachedMilestones = 0;
};
