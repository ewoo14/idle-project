#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"

struct IDLEPROJECT_API FCombatPowerFormula
{
	static int64 ComputeCombatPower(const FDerivedStats& Stats);
};
