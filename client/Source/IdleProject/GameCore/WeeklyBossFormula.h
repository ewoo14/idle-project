#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FWeeklyBossFormula
{
	static constexpr int32 WeeklyChallengeLimit = 7;

	static int64 GetChallengeDamage(int64 CombatPower);
	static int64 MilestoneThreshold(int32 Milestone);
	static int32 GetReachedMilestones(int64 AccumDamage);
	static int64 MilestoneGoldReward(int32 Milestone);
	static int64 MilestoneEssenceReward(int32 Milestone);
};
