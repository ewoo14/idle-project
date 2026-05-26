#include "CombatSystem/BossPhaseFormula.h"

int32 FBossPhaseFormula::GetBossPhase(float HpRatio)
{
	const float ClampedRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
	if (ClampedRatio > 0.66f)
	{
		return 1;
	}

	if (ClampedRatio > 0.33f)
	{
		return 2;
	}

	return 3;
}

float FBossPhaseFormula::GetPhaseAtkMultiplier(int32 Phase)
{
	switch (Phase)
	{
	case 1:
		return 1.0f;
	case 2:
		return 1.25f;
	case 3:
		return 1.6f;
	default:
		return 1.0f;
	}
}

float FBossPhaseFormula::GetPhaseAtkSpeedMultiplier(int32 Phase)
{
	switch (Phase)
	{
	case 1:
		return 1.0f;
	case 2:
		return 1.15f;
	case 3:
		return 1.3f;
	default:
		return 1.0f;
	}
}

float FBossPhaseFormula::GetSpecialAttackDamageMultiplier()
{
	return 2.5f;
}
