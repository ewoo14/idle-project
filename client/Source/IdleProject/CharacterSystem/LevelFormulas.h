#pragma once

#include "CoreMinimal.h"

/** 서버 level.ts 계열 밸런스 수식을 미러링하는 정적 유틸리티입니다. */
struct IDLEPROJECT_API FLevelFormulas
{
	static constexpr int32 LEVEL_CAP = 200;

	static int64 ExpToNext(int32 Level);
	static int64 CumulativeExp(int32 Level);
	static float EnhanceSuccessRate(int32 CurrentStage);
};
