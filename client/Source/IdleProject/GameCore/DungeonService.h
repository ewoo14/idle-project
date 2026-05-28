#pragma once

#include "CoreMinimal.h"
#include "GameCore/DungeonTypes.h"
#include "UObject/Object.h"
#include "DungeonService.generated.h"

UCLASS(BlueprintType)
class IDLEPROJECT_API UDungeonService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Idle|Dungeon")
	FDungeonRunResult TryRunDungeon(EDungeonType Type, int64 CombatPower, const FString& TodayUtc);

	UFUNCTION(BlueprintPure, Category = "Idle|Dungeon")
	int32 GetRemainingEntries(EDungeonType Type, const FString& TodayUtc) const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Dungeon")
	void EnsureDailyReset(const FString& TodayUtc);

	void CaptureState(TMap<EDungeonType, int32>& OutEntriesUsed, FString& OutDailyReset) const;
	void RestoreState(const TMap<EDungeonType, int32>& InEntriesUsed, const FString& InDailyReset);

private:
	UPROPERTY()
	TMap<EDungeonType, int32> EntriesUsed;

	UPROPERTY()
	FString DailyResetDate;
};
