#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FDropFormula
{
	static float GetRarityStatMultiplier(EItemRarity Rarity);
	static EItemRarity RollRarityForLevel(int32 Level, FRandomStream& Rng);
	static EItemSet RollItemSet(EItemRarity Rarity, FRandomStream& Rng);
	static FItemInstance ComputeItemBonus(EItemSlot Slot, int32 Level, EItemRarity Rarity, float Variance);
	static void RollAffixes(EItemRarity Rarity, int32 Level, FRandomStream& Rng, FItemInstance& OutItem);
};
