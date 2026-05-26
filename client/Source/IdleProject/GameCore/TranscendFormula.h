#pragma once

#include "CoreMinimal.h"

/** Second prestige progression formulas for transcendence. */
struct IDLEPROJECT_API FTranscendFormula
{
	static constexpr int32 TranscendRebirthThreshold = 5;

	static float GetTranscendStatMultiplier(int32 TranscendCount);
	static bool CanTranscend(int32 RebirthCount);
};
