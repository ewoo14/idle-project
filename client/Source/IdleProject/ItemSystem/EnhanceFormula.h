#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FEnhanceFormula
{
	static constexpr int32 MaxEnhanceLevel = 5;

	static int64 GetEnhanceCost(int32 CurrentLevel);
	static int64 GetEnhanceCost(int32 CurrentLevel, EItemRarity Rarity);
	static int64 GetRarityCostMultiplier(EItemRarity Rarity);
	static float GetEnhanceSuccessRate(int32 CurrentLevel);
	static bool RollEnhanceSuccess(float SuccessRate, FRandomStream& Stream);
};
