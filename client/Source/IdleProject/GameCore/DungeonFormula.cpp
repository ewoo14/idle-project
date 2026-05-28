#include "GameCore/DungeonFormula.h"

namespace
{
constexpr float CpRewardScale = 100.0f;

int64 RoundAndClampToInt64(double Value)
{
	if (!FMath::IsFinite(Value) || Value >= static_cast<double>(MAX_int64))
	{
		return MAX_int64;
	}
	return FMath::Max<int64>(0, FMath::RoundToInt64(Value));
}

float GetCpMultiplier(int64 CombatPower)
{
	return FMath::Max(1.0f, static_cast<float>(CombatPower) / CpRewardScale);
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

	const float Multiplier = GetCpMultiplier(CombatPower);
	Result.bSuccess = true;
	switch (Type)
	{
	case EDungeonType::Gold:
		Result.GoldReward = RoundAndClampToInt64(1000.0 * static_cast<double>(Multiplier));
		break;
	case EDungeonType::Exp:
		Result.ExpReward = RoundAndClampToInt64(500.0 * static_cast<double>(Multiplier));
		break;
	case EDungeonType::Essence:
		Result.EssenceReward = RoundAndClampToInt64(10.0 * static_cast<double>(Multiplier));
		break;
	default:
		Result.bSuccess = false;
		break;
	}

	return Result;
}
