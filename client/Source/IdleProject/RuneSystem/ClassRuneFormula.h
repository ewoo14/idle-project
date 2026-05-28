#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneTypes.h"

struct IDLEPROJECT_API FClassRuneFormula
{
	static constexpr int32 ClassRuneSlotIndex = 6;

	static FRuneCoreMultipliers GetClassMasteryMultipliers(EClassId ClassId, EItemRarity Rarity, int32 EnhanceLevel);
	static int64 GetClassRuneCraftCost(EItemRarity Rarity);
	static FRuneInstance MakeClassRune(EClassId ClassId, EItemRarity Rarity, int32 ProgressIndex);
	static bool RollClassRuneDrop(int32 MonsterLevel, bool bIsBoss, EClassId ClassId, FRandomStream& Rng, FRuneInstance& OutRune);
};
