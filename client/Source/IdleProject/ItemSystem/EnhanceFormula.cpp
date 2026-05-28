#include "ItemSystem/EnhanceFormula.h"

namespace
{
constexpr int64 BaseEnhanceCost = 100;
constexpr float MaxEnhanceSuccessRate = 0.95f;
constexpr float MinEnhanceSuccessRate = 0.05f;
constexpr float EnhanceSuccessRateDecayPerLevel = 0.018f;
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
	case EItemRarity::Rare:
		return 2;
	case EItemRarity::Epic:
		return 4;
	case EItemRarity::Unique:
		return 8;
	case EItemRarity::Legendary:
		return 16;
	case EItemRarity::Transcendent:
		return 32;
	case EItemRarity::Mythic:
		return 64;
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
	return FMath::Clamp(
		MaxEnhanceSuccessRate - static_cast<float>(ClampedLevel) * EnhanceSuccessRateDecayPerLevel,
		MinEnhanceSuccessRate,
		MaxEnhanceSuccessRate);
}

bool FEnhanceFormula::RollEnhanceSuccess(float SuccessRate, FRandomStream& Stream)
{
	const float ClampedRate = FMath::Clamp(SuccessRate, 0.0f, 1.0f);
	return Stream.GetFraction() < ClampedRate;
}
