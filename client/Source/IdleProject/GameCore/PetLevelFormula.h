#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FPetLevelFormula
{
	static constexpr int32 MaxPetLevel = 10;

	static int64 GetFeedCost(int32 CurrentLevel);
	static float GetBonusMultiplier(int32 Level);
};
