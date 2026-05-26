#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FRewardFormula
{
	static constexpr double BossRewardBonus = 8.0;

	static int64 ComputeKillExp(int64 BaseExp, int32 GlobalStageIndex, bool bIsBoss);
	static int64 ComputeKillGold(int64 BaseGold, int32 GlobalStageIndex, bool bIsBoss);
	static int32 GetMonsterLevelForStage(int32 GlobalStageIndex);
};
