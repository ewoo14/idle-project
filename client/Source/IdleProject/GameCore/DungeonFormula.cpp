#include "GameCore/DungeonFormula.h"

namespace
{
constexpr double GoldDungeonBaseReward = 20000.0;
constexpr double ExpDungeonBaseReward = 20000.0;
constexpr double EssenceDungeonBaseReward = 12.0;

int64 RoundDungeonRewardAndClampToInt64(double Value)
{
	if (!FMath::IsFinite(Value) || Value >= static_cast<double>(MAX_int64))
	{
		return MAX_int64;
	}
	return FMath::Max<int64>(0, FMath::RoundToInt64(Value));
}

int64 RoundRequirementAndClampToInt64(float Value)
{
	if (!FMath::IsFinite(Value) || static_cast<double>(Value) >= static_cast<double>(MAX_int64))
	{
		return MAX_int64;
	}
	return FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(Value)));
}

double GetBaseReward(EDungeonType Type)
{
	switch (Type)
	{
	case EDungeonType::Gold:
		return GoldDungeonBaseReward;
	case EDungeonType::Exp:
		return ExpDungeonBaseReward;
	case EDungeonType::Essence:
		return EssenceDungeonBaseReward;
	default:
		return 0.0;
	}
}

float GetCpMultiplier(EDungeonType Type, int64 CombatPower)
{
	const int64 MinimumCp = FMath::Max<int64>(1, FDungeonFormula::GetMinimumCp(Type));
	const float CpRatio = static_cast<float>(CombatPower) / static_cast<float>(MinimumCp);
	return FMath::Sqrt(FMath::Max(1.0f, CpRatio));
}

int64 ScaleReward(double BaseReward, float CpMultiplier, int32 TierMultiplier)
{
	const float BaseScaledReward = static_cast<float>(BaseReward * static_cast<double>(CpMultiplier));
	return RoundDungeonRewardAndClampToInt64(static_cast<double>(BaseScaledReward) * TierMultiplier);
}
}

int32 FDungeonFormula::GetDailyEntryLimit(EDungeonType Type)
{
	switch (Type)
	{
	case EDungeonType::Gold:
	case EDungeonType::Exp:
	case EDungeonType::Essence:
		return DailyEntryLimit;
	default:
		return 0;
	}
}

int64 FDungeonFormula::GetMinimumCp(EDungeonType Type)
{
	switch (Type)
	{
	case EDungeonType::Gold:
		return 100;
	case EDungeonType::Exp:
		return 250;
	case EDungeonType::Essence:
		return 500;
	default:
		return 0;
	}
}

int64 FDungeonFormula::GetTierCpRequirement(EDungeonType Type, int32 Tier)
{
	const int64 MinimumCp = GetMinimumCp(Type);
	if (GetDailyEntryLimit(Type) <= 0 || MinimumCp <= 0 || Tier < 1)
	{
		return 0;
	}

	const float Requirement = static_cast<float>(MinimumCp) * FMath::Pow(TierCpFactor, static_cast<float>(Tier - 1));
	return RoundRequirementAndClampToInt64(Requirement);
}

int32 FDungeonFormula::GetMaxAccessibleTier(EDungeonType Type, int64 CombatPower)
{
	const int64 MinimumCp = GetMinimumCp(Type);
	if (GetDailyEntryLimit(Type) <= 0 || MinimumCp <= 0 || CombatPower < MinimumCp)
	{
		return 0;
	}

	const float CpRatio = static_cast<float>(CombatPower) / static_cast<float>(MinimumCp);
	return FMath::FloorToInt(FMath::Loge(CpRatio) / FMath::Loge(TierCpFactor)) + 1;
}

int32 FDungeonFormula::GetTierRewardMultiplier(int32 Tier)
{
	return FMath::Max(1, Tier);
}

FDungeonRunResult FDungeonFormula::GetRewardForCp(EDungeonType Type, int64 CombatPower, int32 Tier)
{
	FDungeonRunResult Result;
	Result.Type = Type;
	Result.Tier = Tier;

	if (GetDailyEntryLimit(Type) <= 0 || CombatPower <= 0 || Tier < 1 || CombatPower < GetTierCpRequirement(Type, Tier))
	{
		return Result;
	}

	const float Multiplier = GetCpMultiplier(Type, CombatPower);
	const int32 RewardMultiplier = GetTierRewardMultiplier(Tier);
	const double BaseReward = GetBaseReward(Type);
	Result.bSuccess = true;
	switch (Type)
	{
	case EDungeonType::Gold:
		Result.GoldReward = ScaleReward(BaseReward, Multiplier, RewardMultiplier);
		break;
	case EDungeonType::Exp:
		Result.ExpReward = ScaleReward(BaseReward, Multiplier, RewardMultiplier);
		break;
	case EDungeonType::Essence:
		Result.EssenceReward = ScaleReward(BaseReward, Multiplier, RewardMultiplier);
		break;
	default:
		Result.bSuccess = false;
		break;
	}

	return Result;
}
