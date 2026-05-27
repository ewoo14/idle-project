#include "GameCore/TowerFormula.h"

namespace
{
	constexpr double BaseRequiredPower = 100.0;
	constexpr double RequiredPowerGrowth = 1.15;
	constexpr double BaseRewardPerFloor = 50.0;

	int64 RoundAndClampToInt64(double Value)
	{
		if (!FMath::IsFinite(Value) || Value >= static_cast<double>(MAX_int64))
		{
			return MAX_int64;
		}

		return FMath::Max<int64>(0, FMath::RoundToInt64(Value));
	}
}

int64 FTowerFormula::GetFloorRequiredPower(int32 Floor)
{
	const int32 ClampedFloor = FMath::Max(1, Floor);
	const double RequiredPower = BaseRequiredPower * FMath::Pow(RequiredPowerGrowth, static_cast<double>(ClampedFloor - 1));
	return RoundAndClampToInt64(RequiredPower);
}

bool FTowerFormula::CanClearFloor(int64 CombatPower, int32 Floor)
{
	return CombatPower >= GetFloorRequiredPower(Floor);
}

int64 FTowerFormula::GetFloorReward(int32 Floor)
{
	const int32 ClampedFloor = FMath::Max(1, Floor);
	return RoundAndClampToInt64(BaseRewardPerFloor * static_cast<double>(ClampedFloor));
}
