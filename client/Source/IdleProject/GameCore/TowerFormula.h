#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FTowerFormula
{
	static int64 GetFloorRequiredPower(int32 Floor);
	static bool CanClearFloor(int64 CombatPower, int32 Floor);
	static int64 GetFloorReward(int32 Floor);
};
