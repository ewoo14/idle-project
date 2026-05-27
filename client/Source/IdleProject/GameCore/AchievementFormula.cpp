#include "GameCore/AchievementFormula.h"

FAchievementDefinition::FAchievementDefinition(
	const TCHAR* InAchievementId,
	EAchievementCategory InCategory,
	EAchievementMetric InMetric,
	EAchievementMetricMode InMetricMode,
	int64 InBaseThreshold,
	float InGrowth,
	int32 InPointsPerTier,
	const TCHAR* InDisplayName)
	: AchievementId(InAchievementId)
	, Category(InCategory)
	, Metric(InMetric)
	, MetricMode(InMetricMode)
	, BaseThreshold(FMath::Max<int64>(1, InBaseThreshold))
	, Growth(FMath::Max(1.01f, InGrowth))
	, PointsPerTier(FMath::Max(1, InPointsPerTier))
	, DisplayName(FText::FromString(InDisplayName))
{
}

const TArray<FAchievementDefinition>& FAchievementFormula::GetDefinitions()
{
	static const TArray<FAchievementDefinition> Definitions = {
		{ TEXT("combat_monster_slayer"), EAchievementCategory::Combat, EAchievementMetric::MonstersKilled, EAchievementMetricMode::Cumulative, 10, DefaultTierGrowth, 1, TEXT("Monster Slayer") },
		{ TEXT("combat_boss_breaker"), EAchievementCategory::Combat, EAchievementMetric::BossesKilled, EAchievementMetricMode::Cumulative, 1, DefaultTierGrowth, 2, TEXT("Boss Breaker") },
		{ TEXT("combat_tower_climber"), EAchievementCategory::Combat, EAchievementMetric::TowerHighestFloor, EAchievementMetricMode::Maximum, 10, DefaultTierGrowth, 2, TEXT("Tower Climber") },
		{ TEXT("progress_level_peak"), EAchievementCategory::Progression, EAchievementMetric::HighestLevelReached, EAchievementMetricMode::Maximum, 10, DefaultTierGrowth, 1, TEXT("Level Peak") },
		{ TEXT("progress_stage_clear"), EAchievementCategory::Progression, EAchievementMetric::StagesCleared, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("Stage Clear") },
		{ TEXT("progress_rebirth_cycle"), EAchievementCategory::Progression, EAchievementMetric::RebirthCount, EAchievementMetricMode::Maximum, 1, DefaultTierGrowth, 3, TEXT("Rebirth Cycle") },
		{ TEXT("progress_transcend_path"), EAchievementCategory::Progression, EAchievementMetric::TranscendCount, EAchievementMetricMode::Maximum, 1, DefaultTierGrowth, 5, TEXT("Transcend Path") },
		{ TEXT("gear_enhancer"), EAchievementCategory::Gear, EAchievementMetric::GearEnhanced, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("Gear Enhancer") },
		{ TEXT("gear_peak_level"), EAchievementCategory::Gear, EAchievementMetric::HighestEnhanceLevel, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("Peak Enhancement") },
		{ TEXT("gear_collector"), EAchievementCategory::Gear, EAchievementMetric::ItemsCollected, EAchievementMetricMode::Cumulative, 10, DefaultTierGrowth, 1, TEXT("Gear Collector") },
		{ TEXT("economy_gold_earner"), EAchievementCategory::Economy, EAchievementMetric::GoldEarned, EAchievementMetricMode::Cumulative, 1000, DefaultTierGrowth, 1, TEXT("Gold Earner") },
		{ TEXT("economy_gold_sink"), EAchievementCategory::Economy, EAchievementMetric::GoldSpent, EAchievementMetricMode::Cumulative, 1000, DefaultTierGrowth, 1, TEXT("Gold Sink") },
		{ TEXT("economy_shop_rolls"), EAchievementCategory::Economy, EAchievementMetric::GearRollsPurchased, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("Shop Regular") },
		{ TEXT("skill_rank_ups"), EAchievementCategory::Skill, EAchievementMetric::SkillRankUps, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("Skill Training") },
		{ TEXT("skill_rank_peak"), EAchievementCategory::Skill, EAchievementMetric::HighestSkillRank, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("Skill Mastery") },
		{ TEXT("pet_feeder"), EAchievementCategory::Pet, EAchievementMetric::PetsFed, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("Pet Care") },
		{ TEXT("pet_level_peak"), EAchievementCategory::Pet, EAchievementMetric::HighestPetLevel, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("Pet Bond") },
		{ TEXT("quest_helper"), EAchievementCategory::Quest, EAchievementMetric::QuestsCompleted, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("Quest Helper") },
		{ TEXT("quest_season_rewards"), EAchievementCategory::Quest, EAchievementMetric::SeasonRewardsClaimed, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("Season Claimer") },
		{ TEXT("collection_unique_items"), EAchievementCategory::Collection, EAchievementMetric::UniqueItemsFound, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 2, TEXT("Unique Finds") },
		{ TEXT("misc_offline_claims"), EAchievementCategory::Misc, EAchievementMetric::OfflineRewardsClaimed, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("Welcome Back") },
		{ TEXT("misc_days_played"), EAchievementCategory::Misc, EAchievementMetric::DaysPlayed, EAchievementMetricMode::Cumulative, 1, DefaultTierGrowth, 1, TEXT("Daily Rhythm") },
	};
	return Definitions;
}

int32 FAchievementFormula::GetTierForValue(const FAchievementDefinition& Definition, int64 Value)
{
	if (Value <= 0 || Definition.BaseThreshold <= 0 || Definition.Growth <= 1.0f)
	{
		return 0;
	}

	int32 Tier = 0;
	double Threshold = static_cast<double>(Definition.BaseThreshold);
	const double SafeValue = static_cast<double>(Value);
	while (Threshold <= SafeValue && Threshold <= static_cast<double>(MAX_int64))
	{
		++Tier;
		Threshold *= static_cast<double>(Definition.Growth);
	}
	return Tier;
}

int32 FAchievementFormula::GetPointsForTiers(const FAchievementDefinition& Definition, int32 Tier)
{
	return FMath::Max(0, Tier) * FMath::Max(1, Definition.PointsPerTier);
}

float FAchievementFormula::GetStatMultiplier(int32 TotalPoints)
{
	return 1.0f + static_cast<float>(FMath::Max(0, TotalPoints)) * AchievementPointsMultiplier;
}
