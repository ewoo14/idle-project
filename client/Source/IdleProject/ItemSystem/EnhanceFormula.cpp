#include "ItemSystem/EnhanceFormula.h"

namespace
{
constexpr int64 BaseEnhanceCost = 100;
constexpr float EnhanceSuccessRates[FEnhanceFormula::MaxEnhanceLevel] = {
	0.95f,
	0.85f,
	0.70f,
	0.55f,
	0.40f
};
}

int64 FEnhanceFormula::GetEnhanceCost(int32 CurrentLevel)
{
	return GetEnhanceCost(CurrentLevel, EItemRarity::Common);
}

int64 FEnhanceFormula::GetEnhanceCost(int32 CurrentLevel, EItemRarity Rarity)
{
	if (CurrentLevel >= MaxEnhanceLevel)
	{
		return 0;
	}

	const int32 ClampedLevel = FMath::Max(CurrentLevel, 0);
	const int64 NextLevel = static_cast<int64>(ClampedLevel) + 1;
	return BaseEnhanceCost * NextLevel * NextLevel * GetRarityCostMultiplier(Rarity);
}

int64 FEnhanceFormula::GetRarityCostMultiplier(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return 1;
	case EItemRarity::Uncommon:
		return 2;
	case EItemRarity::Rare:
		return 4;
	case EItemRarity::Epic:
		return 8;
	case EItemRarity::Legendary:
		return 16;
	case EItemRarity::None:
	default:
		return 0;
	}
}

float FEnhanceFormula::GetEnhanceSuccessRate(int32 CurrentLevel)
{
	if (CurrentLevel >= MaxEnhanceLevel)
	{
		return 0.0f;
	}

	const int32 ClampedLevel = FMath::Clamp(CurrentLevel, 0, MaxEnhanceLevel - 1);
	return EnhanceSuccessRates[ClampedLevel];
}

bool FEnhanceFormula::RollEnhanceSuccess(float SuccessRate, FRandomStream& Stream)
{
	const float ClampedRate = FMath::Clamp(SuccessRate, 0.0f, 1.0f);
	return Stream.GetFraction() < ClampedRate;
}
