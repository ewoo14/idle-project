#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FSetBonusFormula
{
	static int32 GetSetPieceThreshold(int32 TierIndex);
	static FDerivedStats ComputeSetBonus(const TArray<FItemInstance>& EquippedItems);
};
