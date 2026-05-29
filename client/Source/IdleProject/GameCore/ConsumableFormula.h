#pragma once

#include "CoreMinimal.h"
#include "GameCore/ConsumableTypes.h"

struct IDLEPROJECT_API FConsumableFormula
{
	static constexpr int64 BuffDurationSec = 1800;

	static float GetBuffPercent(EConsumableType Type);
	static int64 GetBuffDurationSec(EConsumableType Type);
	static bool IsValidType(EConsumableType Type);
};
