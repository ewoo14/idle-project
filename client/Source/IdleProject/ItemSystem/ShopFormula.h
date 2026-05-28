#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FShopFormula
{
	static int64 GetGearRollCost(int32 GlobalStageIndex);
	static int64 GetProtectionScrollCost(int32 GlobalStageIndex);
	static int64 GetResetCubeCost(int32 GlobalStageIndex);
	static int64 GetRankCubeCost(int32 GlobalStageIndex);
};
