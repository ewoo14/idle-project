#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneTypes.h"

struct IDLEPROJECT_API FRuneSetFormula
{
	static constexpr int32 Tier1Count = 2;
	static constexpr int32 Tier2Count = 4;
	static constexpr int32 Tier3Count = 6;
	static constexpr float Tier1Bonus = 0.05f;
	static constexpr float Tier2Bonus = 0.12f;
	static constexpr float Tier3Bonus = 0.25f;

	static float GetSetTierBonus(int32 EquippedCount);
	static void ComputeSetBonus(const TMap<ERuneSet, int32>& SetCounts, FRuneCoreMultipliers& OutCore, FRuneUtilValues& OutUtil);
	static ERuneSet RollRuneSet(EItemRarity Rarity, FRandomStream& Rng);
};
