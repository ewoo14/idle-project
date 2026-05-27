#pragma once

#include "CoreMinimal.h"
#include "AchievementFormula.generated.h"

UENUM(BlueprintType)
enum class EAchievementCategory : uint8
{
	Combat,
	Progression,
	Gear,
	Economy,
	Skill,
	Pet,
	Quest,
	Collection,
	Misc
};

UENUM(BlueprintType)
enum class EAchievementMetric : uint8
{
	MonstersKilled,
	BossesKilled,
	HighestLevelReached,
	StagesCleared,
	RebirthCount,
	TranscendCount,
	TowerHighestFloor,
	GearEnhanced,
	HighestEnhanceLevel,
	ItemsCollected,
	UniqueItemsFound,
	GoldEarned,
	GoldSpent,
	GearRollsPurchased,
	SkillRankUps,
	HighestSkillRank,
	PetsFed,
	HighestPetLevel,
	QuestsCompleted,
	SeasonRewardsClaimed,
	OfflineRewardsClaimed,
	DaysPlayed
};

UENUM()
enum class EAchievementMetricMode : uint8
{
	Cumulative,
	Maximum
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FAchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	FString AchievementId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	EAchievementCategory Category = EAchievementCategory::Misc;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	EAchievementMetric Metric = EAchievementMetric::MonstersKilled;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	EAchievementMetricMode MetricMode = EAchievementMetricMode::Cumulative;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	int64 BaseThreshold = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	float Growth = 2.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	int32 PointsPerTier = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	FString DisplayNameKey;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	FText DisplayName;

	FAchievementDefinition() = default;

	FAchievementDefinition(
		const TCHAR* InAchievementId,
		EAchievementCategory InCategory,
		EAchievementMetric InMetric,
		EAchievementMetricMode InMetricMode,
		int64 InBaseThreshold,
		float InGrowth,
		int32 InPointsPerTier,
		const TCHAR* InDisplayNameKey,
		const TCHAR* InDisplayName);
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FAchievementCategoryProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	EAchievementCategory Category = EAchievementCategory::Misc;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	int32 UnlockedPoints = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	int32 HighestUnlockedTier = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Achievement")
	int64 CurrentValue = 0;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FAchievementMetricSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	EAchievementMetric Metric = EAchievementMetric::MonstersKilled;

	UPROPERTY()
	int64 Value = 0;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FAchievementSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString AchievementId;

	UPROPERTY()
	int32 Tier = 0;
};

struct IDLEPROJECT_API FAchievementFormula
{
	static constexpr float AchievementPointsMultiplier = 0.01f;
	static constexpr int32 MultiplierSoftCapStartPoints = 100;
	static constexpr float MultiplierSoftCapBonusPoints = 50.0f;
	static constexpr float DefaultTierGrowth = 2.0f;

	static const TArray<FAchievementDefinition>& GetDefinitions();
	static int32 GetTierForValue(const FAchievementDefinition& Definition, int64 Value);
	static int32 GetPointsForTiers(const FAchievementDefinition& Definition, int32 Tier);
	static float GetStatMultiplier(int32 TotalPoints);
};
