#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SeasonService.generated.h"

UENUM(BlueprintType)
enum class ESeasonRewardType : uint8
{
	None = 0 UMETA(Hidden),
	Gold = 1,
	Exp = 2
};

USTRUCT(BlueprintType)
struct FSeasonTierDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Season")
	int32 Tier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Season")
	int32 RequiredTokens = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Season")
	ESeasonRewardType RewardType = ESeasonRewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Season")
	int64 RewardAmount = 0;
};

USTRUCT(BlueprintType)
struct FSeasonClaimResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Season")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Season")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Season")
	int32 Tier = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Season")
	ESeasonRewardType RewardType = ESeasonRewardType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Season")
	int64 RewardAmount = 0;
};

UCLASS(BlueprintType)
class IDLEPROJECT_API USeasonService : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentSeasonId = 1;
	static constexpr int32 QuestClaimSeasonTokenReward = 10;

	UFUNCTION(BlueprintCallable, Category = "Idle|Season")
	void InitializeDefaultSeason();

	UFUNCTION(BlueprintCallable, Category = "Idle|Season")
	void AddSeasonTokens(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Season")
	FSeasonClaimResult ClaimSeasonReward(int32 Tier);

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	const TArray<FSeasonTierDefinition>& GetSeasonTiers() const { return Tiers; }

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	int32 GetSeasonId() const { return CurrentSeasonId; }

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	int32 GetSeasonTokens() const { return SeasonTokens; }

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	int32 GetReachedTier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	bool IsTierClaimed(int32 Tier) const { return ClaimedTiers.Contains(Tier); }

private:
	TArray<FSeasonTierDefinition> Tiers;
	TMap<int32, FSeasonTierDefinition> TierByNumber;
	TSet<int32> ClaimedTiers;
	int32 SeasonTokens = 0;

	void BuildDefaultTiers();
};
