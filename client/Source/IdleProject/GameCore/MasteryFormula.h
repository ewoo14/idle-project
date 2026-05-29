#pragma once

#include "CoreMinimal.h"
#include "GameCore/MasteryTypes.h"

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
	static float GetLocalBonus(EMasteryTrack Track, int32 Level);

	// V2 로컬 보너스(트랙당 2종째). 1종과 동일 ln 곡선·계수(0.01)이며 Equipment/Beast 비용 절감은 0.5 상한 클램프.
	static float GetLocalBonus2(EMasteryTrack Track, int32 Level);

	// 심연 2종: 던전 일일 입장 추가 정수 보너스. floor(level/50), 상한 +3.
	static int32 GetAbyssBonusEntries(int32 Level);
};
