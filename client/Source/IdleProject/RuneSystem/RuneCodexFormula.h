#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneCodexTypes.h"

struct IDLEPROJECT_API FRuneCodexFormula
{
	static constexpr int32 TotalCells = 63;
	static constexpr int32 CoreCategoryCells = 35;
	static constexpr int32 UtilCategoryCells = 28;
	static constexpr float PerCellCoreBonus = 0.004f;
	static constexpr float CoreCategoryBonus = 0.05f;
	static constexpr float UtilCategoryCapExtension = 0.10f;

	static float GetRowCompletionBonus(EItemRarity Rarity);
	static FRuneCodexBonus ComputeBonus(const FRuneCodexCompletion& Completion);
};
