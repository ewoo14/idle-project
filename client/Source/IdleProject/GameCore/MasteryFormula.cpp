#include "GameCore/MasteryFormula.h"

int64 FMasteryFormula::XpToNext(int32 Level)
{
	const int32 Safe = FMath::Max(0, Level);
	return static_cast<int64>(FMath::FloorToDouble(static_cast<double>(XpBase) * FMath::Pow(static_cast<double>(XpGrowth), static_cast<double>(Safe))));
}

void FMasteryFormula::LevelFromTotalXp(int64 TotalXp, int32& OutLevel, int64& OutXpIntoLevel, int64& OutXpToNext)
{
	int64 Remaining = FMath::Max<int64>(0, TotalXp);
	int32 Level = 0;
	int64 Need = XpToNext(0);
	while (Remaining >= Need)
	{
		Remaining -= Need;
		++Level;
		Need = XpToNext(Level);
	}

	OutLevel = Level;
	OutXpIntoLevel = Remaining;
	OutXpToNext = Need;
}

float FMasteryFormula::CoreStatMultiplier(int32 CombatLv, int32 EquipLv, int32 ExploreLv)
{
	const int32 Sum = FMath::Max(0, CombatLv) + FMath::Max(0, EquipLv) + FMath::Max(0, ExploreLv);
	return 1.0f + 0.02f * FMath::Loge(1.0f + static_cast<float>(Sum));
}

float FMasteryFormula::CritRateAdd(int32 RuneLv)
{
	return 0.01f * FMath::Loge(1.0f + static_cast<float>(FMath::Max(0, RuneLv)));
}

float FMasteryFormula::DropRateAdd(int32 AbyssLv)
{
	return 0.01f * FMath::Loge(1.0f + static_cast<float>(FMath::Max(0, AbyssLv)));
}

float FMasteryFormula::GoldFindPct(int32 BeastLv)
{
	return 0.02f * FMath::Loge(1.0f + static_cast<float>(FMath::Max(0, BeastLv)));
}

float FMasteryFormula::ExpBoostPct(int32 BeastLv)
{
	return 0.02f * FMath::Loge(1.0f + static_cast<float>(FMath::Max(0, BeastLv)));
}

float FMasteryFormula::GetLocalBonus(EMasteryTrack Track, int32 Level)
{
	const int32 SafeLevel = FMath::Max(0, Level);
	if (SafeLevel <= 0)
	{
		return 0.0f;
	}

	const float RawBonus = 0.01f * FMath::Loge(1.0f + static_cast<float>(SafeLevel));
	return Track == EMasteryTrack::Equipment ? FMath::Min(0.5f, RawBonus) : RawBonus;
}

float FMasteryFormula::GetLocalBonus2(EMasteryTrack Track, int32 Level)
{
	const int32 SafeLevel = FMath::Max(0, Level);
	if (SafeLevel <= 0)
	{
		return 0.0f;
	}

	const float RawBonus = 0.01f * FMath::Loge(1.0f + static_cast<float>(SafeLevel));
	// Equipment(큐브 가격 절감)·Beast(펫 먹이 비용 절감)는 비용 절감 비율이므로 0.5 상한 클램프.
	const bool bCostReduction = Track == EMasteryTrack::Equipment || Track == EMasteryTrack::Beast;
	return bCostReduction ? FMath::Min(0.5f, RawBonus) : RawBonus;
}

int32 FMasteryFormula::GetAbyssBonusEntries(int32 Level)
{
	const int32 SafeLevel = FMath::Max(0, Level);
	return FMath::Clamp(SafeLevel / 50, 0, 3);
}
