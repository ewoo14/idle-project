#include "ItemSystem/ShopFormula.h"

#include "GameCore/StageFormula.h"

int64 FShopFormula::GetGearRollCost(int32 GlobalStageIndex)
{
	const int32 SafeStageIndex = FMath::Max(0, GlobalStageIndex);
	const double ScaledCost = 300.0 * static_cast<double>(FStageFormula::ComputeRewardMultiplier(SafeStageIndex));
	return FMath::Max<int64>(1, FMath::RoundToInt64(ScaledCost));
}
