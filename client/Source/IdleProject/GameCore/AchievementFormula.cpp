#include "GameCore/AchievementFormula.h"

FAchievementDefinition::FAchievementDefinition(
	const TCHAR* InAchievementId,
	EAchievementCategory InCategory,
	EAchievementMetric InMetric,
	EAchievementMetricMode InMetricMode,
	int64 InBaseThreshold,
	float InGrowth,
	int32 InPointsPerTier,
	const TCHAR* InDisplayNameKey,
	const TCHAR* InDisplayName)
	: AchievementId(InAchievementId)
	, Category(InCategory)
	, Metric(InMetric)
	, MetricMode(InMetricMode)
	, BaseThreshold(FMath::Max<int64>(1, InBaseThreshold))
	, Growth(FMath::Max(1.01f, InGrowth))
	, PointsPerTier(FMath::Max(1, InPointsPerTier))
	, DisplayNameKey(InDisplayNameKey)
	, DisplayName(FText::FromString(InDisplayName))
{
}

const TArray<FAchievementDefinition>& FAchievementFormula::GetDefinitions()
{
	static const TArray<FAchievementDefinition> Definitions = {
		{ TEXT("combat_monster_slayer"), EAchievementCategory::Combat, EAchievementMetric::MonstersKilled, EAchievementMetricMode::Cumulative, 10, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_COMBAT_MONSTER_SLAYER"), TEXT("Monster Slayer") },
		{ TEXT("combat_boss_breaker"), EAchievementCategory::Combat, EAchievementMetric::BossesKilled, EAchievementMetricMode::Cumulative, 1, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_COMBAT_BOSS_BREAKER"), TEXT("Boss Breaker") },
		{ TEXT("combat_tower_climber"), EAchievementCategory::Combat, EAchievementMetric::TowerHighestFloor, EAchievementMetricMode::Maximum, 10, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_COMBAT_TOWER_CLIMBER"), TEXT("Tower Climber") },
		{ TEXT("progress_level_peak"), EAchievementCategory::Progression, EAchievementMetric::HighestLevelReached, EAchievementMetricMode::Maximum, 10, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_PROGRESS_LEVEL_PEAK"), TEXT("Level Peak") },
		{ TEXT("progress_stage_clear"), EAchievementCategory::Progression, EAchievementMetric::StagesCleared, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_PROGRESS_STAGE_CLEAR"), TEXT("Stage Clear") },
		{ TEXT("progress_rebirth_cycle"), EAchievementCategory::Progression, EAchievementMetric::RebirthCount, EAchievementMetricMode::Maximum, 1, DefaultTierGrowth, 3, TEXT("ACHIEVEMENT_NAME_PROGRESS_REBIRTH_CYCLE"), TEXT("Rebirth Cycle") },
		{ TEXT("progress_transcend_path"), EAchievementCategory::Progression, EAchievementMetric::TranscendCount, EAchievementMetricMode::Maximum, 1, DefaultTierGrowth, 5, TEXT("ACHIEVEMENT_NAME_PROGRESS_TRANSCEND_PATH"), TEXT("Transcend Path") },
		{ TEXT("gear_enhancer"), EAchievementCategory::Gear, EAchievementMetric::GearEnhanced, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_GEAR_ENHANCER"), TEXT("Gear Enhancer") },
		{ TEXT("gear_peak_level"), EAchievementCategory::Gear, EAchievementMetric::HighestEnhanceLevel, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_GEAR_PEAK_LEVEL"), TEXT("Peak Enhancement") },
		{ TEXT("gear_collector"), EAchievementCategory::Gear, EAchievementMetric::ItemsCollected, EAchievementMetricMode::Cumulative, 10, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_GEAR_COLLECTOR"), TEXT("Gear Collector") },
		{ TEXT("economy_gold_earner"), EAchievementCategory::Economy, EAchievementMetric::GoldEarned, EAchievementMetricMode::Cumulative, 1000, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_ECONOMY_GOLD_EARNER"), TEXT("Gold Earner") },
		{ TEXT("economy_gold_sink"), EAchievementCategory::Economy, EAchievementMetric::GoldSpent, EAchievementMetricMode::Cumulative, 1000, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_ECONOMY_GOLD_SINK"), TEXT("Gold Sink") },
		{ TEXT("economy_shop_rolls"), EAchievementCategory::Economy, EAchievementMetric::GearRollsPurchased, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_ECONOMY_SHOP_ROLLS"), TEXT("Shop Regular") },
		{ TEXT("skill_rank_ups"), EAchievementCategory::Skill, EAchievementMetric::SkillRankUps, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_SKILL_RANK_UPS"), TEXT("Skill Training") },
		{ TEXT("skill_rank_peak"), EAchievementCategory::Skill, EAchievementMetric::HighestSkillRank, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_SKILL_RANK_PEAK"), TEXT("Skill Mastery") },
		{ TEXT("pet_feeder"), EAchievementCategory::Pet, EAchievementMetric::PetsFed, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_PET_FEEDER"), TEXT("Pet Care") },
		{ TEXT("pet_level_peak"), EAchievementCategory::Pet, EAchievementMetric::HighestPetLevel, EAchievementMetricMode::Maximum, 5, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_PET_LEVEL_PEAK"), TEXT("Pet Bond") },
		{ TEXT("quest_helper"), EAchievementCategory::Quest, EAchievementMetric::QuestsCompleted, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_QUEST_HELPER"), TEXT("Quest Helper") },
		{ TEXT("quest_season_rewards"), EAchievementCategory::Quest, EAchievementMetric::SeasonRewardsClaimed, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_QUEST_SEASON_REWARDS"), TEXT("Season Claimer") },
		{ TEXT("collection_unique_items"), EAchievementCategory::Collection, EAchievementMetric::UniqueItemsFound, EAchievementMetricMode::Cumulative, 5, DefaultTierGrowth, 2, TEXT("ACHIEVEMENT_NAME_COLLECTION_UNIQUE_ITEMS"), TEXT("Unique Finds") },
		{ TEXT("misc_offline_claims"), EAchievementCategory::Misc, EAchievementMetric::OfflineRewardsClaimed, EAchievementMetricMode::Cumulative, 3, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_MISC_OFFLINE_CLAIMS"), TEXT("Welcome Back") },
		{ TEXT("misc_days_played"), EAchievementCategory::Misc, EAchievementMetric::DaysPlayed, EAchievementMetricMode::Cumulative, 1, DefaultTierGrowth, 1, TEXT("ACHIEVEMENT_NAME_MISC_DAYS_PLAYED"), TEXT("Daily Rhythm") },
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
	while (Threshold <= SafeValue)
	{
		++Tier;
		if (Threshold > static_cast<double>(MAX_int64) / static_cast<double>(Definition.Growth))
		{
			break;
		}
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
	const int32 SafePoints = FMath::Max(0, TotalPoints);
	const float EffectivePoints = SafePoints <= MultiplierSoftCapStartPoints
		? static_cast<float>(SafePoints)
		: static_cast<float>(MultiplierSoftCapStartPoints) +
			MultiplierSoftCapBonusPoints *
				(1.0f - FMath::Exp(
					-static_cast<float>(SafePoints - MultiplierSoftCapStartPoints) /
					MultiplierSoftCapBonusPoints));
	return 1.0f + EffectivePoints * AchievementPointsMultiplier;
}
