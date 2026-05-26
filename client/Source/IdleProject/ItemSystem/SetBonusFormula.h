#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FSetBonusFormula
{
	static int32 GetSetPieceThreshold(int32 TierIndex);
	static FDerivedStats GetTwoPieceBonus(EItemSet ItemSet);
	static FDerivedStats GetFourPieceBonus(EItemSet ItemSet);
	static FDerivedStats ComputeSetBonus(const TArray<FItemInstance>& EquippedItems);
};
