#pragma once

#include "CoreMinimal.h"
#include "GameCore/DungeonTypes.h"

struct IDLEPROJECT_API FDungeonFormula
{
	static constexpr int32 DailyEntryLimit = 3;
	static constexpr float TierCpFactor = 2.0f;

	static int32 GetDailyEntryLimit(EDungeonType Type);
	static int64 GetMinimumCp(EDungeonType Type);
	static int64 GetTierCpRequirement(EDungeonType Type, int32 Tier);
	static int32 GetMaxAccessibleTier(EDungeonType Type, int64 CombatPower);
	static int32 GetTierRewardMultiplier(int32 Tier);
	static FDungeonRunResult GetRewardForCp(EDungeonType Type, int64 CombatPower, int32 Tier = 1);
};
