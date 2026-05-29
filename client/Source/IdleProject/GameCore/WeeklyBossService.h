#pragma once

#include "CoreMinimal.h"
#include "GameCore/WeeklyBossTypes.h"
#include "UObject/Object.h"
#include "WeeklyBossService.generated.h"

UCLASS(BlueprintType)
class IDLEPROJECT_API UWeeklyBossService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Idle|WeeklyBoss")
	void EnsureWeek(const FString& CurrentWeek);

	UFUNCTION(BlueprintCallable, Category = "Idle|WeeklyBoss")
	FWeeklyBossChallengeResult Challenge(int64 CombatPower, const FString& CurrentWeek);

	UFUNCTION(BlueprintCallable, Category = "Idle|WeeklyBoss")
	bool ClaimMilestone(int32 Milestone);

	UFUNCTION(BlueprintPure, Category = "Idle|WeeklyBoss")
	int32 GetRemainingChallenges() const;

	UFUNCTION(BlueprintPure, Category = "Idle|WeeklyBoss")
	int64 GetDamage() const { return Damage; }

	UFUNCTION(BlueprintPure, Category = "Idle|WeeklyBoss")
	int32 GetReachedMilestones() const;

	UFUNCTION(BlueprintPure, Category = "Idle|WeeklyBoss")
	int32 GetClaimedMilestones() const { return ClaimedMilestones; }

	UFUNCTION(BlueprintPure, Category = "Idle|WeeklyBoss")
	const FString& GetWeekId() const { return WeekId; }

	FWeeklyBossSaveState ExportSave() const;
	void ImportSave(const FString& InWeekId, int64 InDamage, int32 InChallengesUsed, int32 InClaimedMilestones);

private:
	UPROPERTY()
	FString WeekId;

	UPROPERTY()
	int64 Damage = 0;

	UPROPERTY()
	int32 ChallengesUsed = 0;

	UPROPERTY()
	int32 ClaimedMilestones = 0;
};
