#include "RuneSystem/RuneCodexFormula.h"

float FRuneCodexFormula::GetRowCompletionBonus(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return 0.01f;
	case EItemRarity::Uncommon:
		return 0.02f;
	case EItemRarity::Rare:
		return 0.03f;
	case EItemRarity::Epic:
		return 0.05f;
	case EItemRarity::Legendary:
		return 0.08f;
	case EItemRarity::Mythic:
		return 0.12f;
	default:
		return 0.0f;
	}
}

FRuneCodexBonus FRuneCodexFormula::ComputeBonus(const FRuneCodexCompletion& Completion)
{
	FRuneCodexBonus Bonus;
	double CoreStatAdd = static_cast<double>(FMath::Clamp(Completion.UnlockedCells, 0, TotalCells)) * PerCellCoreBonus;

	for (int32 RowIndex = 0; RowIndex < Completion.RowComplete.Num() && RowIndex < 6; ++RowIndex)
	{
		if (Completion.RowComplete[RowIndex])
		{
			CoreStatAdd += GetRowCompletionBonus(static_cast<EItemRarity>(RowIndex + 1));
		}
	}

	if (Completion.bCoreCategoryComplete)
	{
		CoreStatAdd += CoreCategoryBonus;
	}
	Bonus.CoreStatAdd = static_cast<float>(CoreStatAdd);
	if (Completion.bUtilCategoryComplete)
	{
		Bonus.UtilCapExtension = UtilCategoryCapExtension;
	}

	return Bonus;
}
