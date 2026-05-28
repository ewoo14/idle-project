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

FDungeonRunResult FDungeonFormula::GetRewardForCp(EDungeonType Type, int64 CombatPower)
{
	FDungeonRunResult Result;
	Result.Type = Type;

	const int64 MinimumCp = GetMinimumCp(Type);
	if (GetDailyEntryLimit(Type) <= 0 || CombatPower < MinimumCp || CombatPower <= 0)
	{
		return Result;
	}

	const float Multiplier = GetCpMultiplier(Type, CombatPower);
	const double BaseReward = GetBaseReward(Type);
	Result.bSuccess = true;
	switch (Type)
	{
	case EDungeonType::Gold:
		Result.GoldReward = RoundDungeonRewardAndClampToInt64(BaseReward * static_cast<double>(Multiplier));
		break;
	case EDungeonType::Exp:
		Result.ExpReward = RoundDungeonRewardAndClampToInt64(BaseReward * static_cast<double>(Multiplier));
		break;
	case EDungeonType::Essence:
		Result.EssenceReward = RoundDungeonRewardAndClampToInt64(BaseReward * static_cast<double>(Multiplier));
		break;
	default:
		Result.bSuccess = false;
		break;
	}

	return Result;
}
