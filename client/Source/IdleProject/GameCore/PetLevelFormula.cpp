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

int64 FPetLevelFormula::GetPetEvolveCost(int32 Star)
{
	const int32 EffectiveStar = FMath::Max(0, Star);
	// 서버 floor(BASE * GROWTH^star): double 거듭제곱 후 floor → int64.
	const double Cost = static_cast<double>(PetEvolveBase) * FMath::Pow(PetEvolveGrowth, static_cast<double>(EffectiveStar));
	return FMath::FloorToInt64(Cost);
}

float FPetLevelFormula::GetPetStarMultiplier(int32 Star)
{
	// 서버 1 + STEP * max(0,star): float 연산 순서 동일.
	return 1.0f + PetStarStep * static_cast<float>(FMath::Max(0, Star));
}
