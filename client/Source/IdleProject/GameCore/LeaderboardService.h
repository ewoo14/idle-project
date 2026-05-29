#pragma once

#include "CoreMinimal.h"
#include "GameCore/LeaderboardTypes.h"
#include "UObject/Object.h"
#include "LeaderboardService.generated.h"

UCLASS(BlueprintType)
class IDLEPROJECT_API ULeaderboardService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Idle|Leaderboard")
	TArray<FLeaderboardEntry> ParseListJson(const FString& JsonBody, ELeaderboardKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Idle|Leaderboard")
	FLeaderboardEntry ParseMyRankJson(const FString& JsonBody, ELeaderboardKind Kind);

	UFUNCTION(BlueprintPure, Category = "Idle|Leaderboard")
	TArray<FLeaderboardEntry> GetEntries(ELeaderboardKind Kind) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Leaderboard")
	FLeaderboardEntry GetMyEntry(ELeaderboardKind Kind) const;

private:
	UPROPERTY()
	TArray<FLeaderboardEntry> PowerEntries;

	UPROPERTY()
	TArray<FLeaderboardEntry> RebirthEntries;

	UPROPERTY()
	FLeaderboardEntry PowerMyEntry;

	UPROPERTY()
	FLeaderboardEntry RebirthMyEntry;
};
