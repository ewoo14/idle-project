#pragma once

#include "CoreMinimal.h"
#include "GameCore/AchievementFormula.h"
#include "UObject/Object.h"
#include "AchievementService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, const FString&, AchievementId, int32, Tier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAchievementProgress, EAchievementMetric, Metric, int64, Value, int32, TotalPoints);

UCLASS()
class IDLEPROJECT_API UAchievementService : public UObject
{
	GENERATED_BODY()

public:
	void InitializeDefaultAchievements();

	UFUNCTION(BlueprintCallable, Category = "Idle|Achievement")
	void RecordMetric(EAchievementMetric Metric, int64 AmountOrValue);

	void RecordItemCollected(FName ItemId);

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	int64 GetMetricValue(EAchievementMetric Metric) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	int32 GetTotalPoints() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	float GetStatMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	TArray<FAchievementCategoryProgress> GetCategoryProgress() const;

	void CaptureState(TArray<FAchievementMetricSaveEntry>& OutMetrics, TArray<FAchievementSaveEntry>& OutAchievements, TArray<FName>* OutUniqueItemIds = nullptr) const;
	void RestoreState(const TArray<FAchievementMetricSaveEntry>& InMetrics, const TArray<FAchievementSaveEntry>& InAchievements, const TArray<FName>* InUniqueItemIds = nullptr);

	UPROPERTY(BlueprintAssignable, Category = "Idle|Achievement")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Achievement")
	FOnAchievementProgress OnAchievementProgress;

private:
	UPROPERTY()
	TMap<EAchievementMetric, int64> MetricValues;

	UPROPERTY()
	TMap<FString, int32> UnlockedTiers;

	UPROPERTY()
	TSet<FName> UniqueItemIds;

	static bool IsKnownMetric(EAchievementMetric Metric);
	static const FAchievementDefinition* FindDefinitionById(const FString& AchievementId);
	void RecomputeUnlockedTiers(bool bBroadcastUnlocks);
};
