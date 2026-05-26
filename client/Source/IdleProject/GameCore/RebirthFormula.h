#pragma once

#include "CoreMinimal.h"

/** 환생 보상 포인트 계산 공식입니다. */
struct IDLEPROJECT_API FRebirthFormula
{
	static int32 GetRebirthPointsReward(int32 RebirthCount, int32 LevelAtRebirth);
};
