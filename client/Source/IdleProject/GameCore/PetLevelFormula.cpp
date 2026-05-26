#include "GameCore/PetLevelFormula.h"

int64 FPetLevelFormula::GetFeedCost(int32 CurrentLevel)
{
	if (CurrentLevel >= MaxPetLevel)
	{
		return 0;
	}

	const int32 EffectiveLevel = FMath::Max(0, CurrentLevel);
	const int64 NextLevel = static_cast<int64>(EffectiveLevel + 1);
	return 500 * NextLevel * NextLevel;
}

float FPetLevelFormula::GetBonusMultiplier(int32 Level)
{
	const int32 EffectiveLevel = FMath::Clamp(Level, 0, MaxPetLevel);
	return 1.0f + static_cast<float>(EffectiveLevel) * 0.1f;
}
