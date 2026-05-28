#pragma once

#include "CoreMinimal.h"
#include "GameCore/DungeonTypes.h"

struct IDLEPROJECT_API FDungeonFormula
{
	static constexpr int32 DailyEntryLimit = 3;

	static int32 GetDailyEntryLimit(EDungeonType Type);
	static int64 GetMinimumCp(EDungeonType Type);
	static FDungeonRunResult GetRewardForCp(EDungeonType Type, int64 CombatPower);
};
