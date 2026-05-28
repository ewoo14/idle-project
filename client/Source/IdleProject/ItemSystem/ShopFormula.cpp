#include "ItemSystem/ShopFormula.h"

#include "GameCore/StageFormula.h"

namespace
{
int64 GetScaledShopCost(int32 GlobalStageIndex, double BaseCost)
{
	const int32 SafeStageIndex = FMath::Max(0, GlobalStageIndex);
	const double ScaledCost = BaseCost * static_cast<double>(FStageFormula::ComputeRewardMultiplier(SafeStageIndex));
	return FMath::Max<int64>(1, FMath::RoundToInt64(ScaledCost));
}
}

int64 FShopFormula::GetGearRollCost(int32 GlobalStageIndex)
{
	return GetScaledShopCost(GlobalStageIndex, 300.0);
}

int64 FShopFormula::GetProtectionScrollCost(int32 GlobalStageIndex)
{
	return GetScaledShopCost(GlobalStageIndex, 300.0);
}

int64 FShopFormula::GetResetCubeCost(int32 GlobalStageIndex)
{
	return GetScaledShopCost(GlobalStageIndex, 800.0);
}

int64 FShopFormula::GetRankCubeCost(int32 GlobalStageIndex)
{
	return GetScaledShopCost(GlobalStageIndex, 4000.0);
}
