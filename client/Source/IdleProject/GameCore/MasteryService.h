#pragma once

#include "CoreMinimal.h"
#include "GameCore/MasteryFormula.h"
#include "GameCore/MasteryTypes.h"
#include "UObject/Object.h"
#include "MasteryService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMasteryTrackLevelUp, EMasteryTrack, Track, int32, NewLevel);

UCLASS()
class IDLEPROJECT_API UMasteryService : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Idle|Mastery")
	void AddXp(EMasteryTrack Track, int64 Amount);

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	int32 GetTrackLevel(EMasteryTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	int64 GetTrackTotalXp(EMasteryTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	FMasteryLevelInfo GetTrackLevelInfo(EMasteryTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	int64 GetWorldPower() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	FMasteryGlobalBonus GetGlobalBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	float GetLocalBonus(EMasteryTrack Track) const;

	TArray<FMasterySaveEntry> ExportSave() const;
	void ImportSave(const TArray<FMasterySaveEntry>& Entries);

	UPROPERTY(BlueprintAssignable, Category = "Idle|Mastery")
	FOnMasteryTrackLevelUp OnTrackLevelUp;

private:
	int64 TrackXp[FMasteryFormula::TrackCount] = {0, 0, 0, 0, 0, 0};
	int32 CachedLevel[FMasteryFormula::TrackCount] = {0, 0, 0, 0, 0, 0};

	static int32 ToIndex(EMasteryTrack Track);
	void RefreshCachedLevel(int32 Index);
	int32 LevelOf(EMasteryTrack Track) const;
};
