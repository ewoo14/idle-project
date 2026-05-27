#pragma once

#include "CoreMinimal.h"

/** Permanent global stat bonus formula from the highest cleared tower floor. */
struct IDLEPROJECT_API FTowerMilestoneFormula
{
	static constexpr int32 MilestoneStep = 10;
	static constexpr float MilestoneBonusPerStep = 0.02f;

	static float GetTowerMilestoneMultiplier(int32 HighestFloor);
};
