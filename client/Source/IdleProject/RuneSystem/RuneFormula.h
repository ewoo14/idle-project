#pragma once

#include "CoreMinimal.h"
#include "RuneSystem/RuneTypes.h"

struct IDLEPROJECT_API FRuneFormula
{
	static constexpr int32 RuneSlotCount = 7;

	static bool IsCoreType(ERuneType Type);
	static bool IsUtilType(ERuneType Type);
	static float GetCoreRuneMultiplier(EItemRarity Rarity, int32 EnhanceLevel);
	static float GetUtilRuneValue(ERuneType Type, EItemRarity Rarity, int32 EnhanceLevel);
	static float GetUtilCap(ERuneType Type);
	static int64 GetEnhanceEssenceCost(int32 CurrentLevel);
	static int64 GetEnhanceGoldCost(int32 CurrentLevel);
	static int64 GetDisenchantEssence(EItemRarity Rarity, int32 EnhanceLevel);
	static int64 GetRerollSetEssenceCost(EItemRarity Rarity);
	static int64 GetRarityUpgradeEssenceCost(EItemRarity Rarity);
	static int64 GetRarityUpgradeGoldCost(EItemRarity Rarity);
	static float GetRarityUpgradeChance(EItemRarity Rarity);
	static int64 GetTransferEssenceCost(int32 SourceLevel);
	static bool RollRuneDrop(int32 MonsterLevel, bool bIsBoss, FRandomStream& Rng, FRuneInstance& OutRune);
	static int64 GetShopRuneRollCost(int32 ProgressIndex);
	static FRuneInstance RollShopRune(int32 ProgressIndex, FRandomStream& Rng);
};
