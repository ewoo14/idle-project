#pragma once

#include "CoreMinimal.h"

/** Boss HP phase and enrage constants shared by combat and HUD code. */
struct IDLEPROJECT_API FBossPhaseFormula
{
	static constexpr float SpecialAttackIntervalSeconds = 6.0f;

	static int32 GetBossPhase(float HpRatio);
	static float GetPhaseAtkMultiplier(int32 Phase);
	static float GetPhaseAtkSpeedMultiplier(int32 Phase);
	static float GetSpecialAttackDamageMultiplier();
};
