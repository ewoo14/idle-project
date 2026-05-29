#pragma once

#include "CoreMinimal.h"

struct IDLEPROJECT_API FMasteryFormula
{
	static constexpr int32 TrackCount = 6;
	static constexpr int64 XpBase = 100;
	static constexpr float XpGrowth = 1.15f;

	static int64 XpToNext(int32 Level);
	static void LevelFromTotalXp(int64 TotalXp, int32& OutLevel, int64& OutXpIntoLevel, int64& OutXpToNext);
	static float CoreStatMultiplier(int32 CombatLv, int32 EquipLv, int32 ExploreLv);
	static float CritRateAdd(int32 RuneLv);
	static float DropRateAdd(int32 AbyssLv);
	static float GoldFindPct(int32 BeastLv);
	static float ExpBoostPct(int32 BeastLv);
};
