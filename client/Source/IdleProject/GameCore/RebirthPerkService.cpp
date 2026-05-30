#include "GameCore/RebirthPerkService.h"

float URebirthPerkService::GetPerkStep(ERebirthPerk Perk)
{
	// 서버 rebirthPerk.ts PERK_STEP 와 1:1(f32). None 은 0.
	switch (Perk)
	{
	case ERebirthPerk::GoldPct:
		return 0.02f;
	case ERebirthPerk::DropPct:
		return 0.02f;
	case ERebirthPerk::CritDmgPct:
		return 0.03f;
	case ERebirthPerk::AllStatPct:
		return 0.01f;
	case ERebirthPerk::ExpPct:
		return 0.02f;
	case ERebirthPerk::OfflineEffPct:
		return 0.03f;
	default:
		return 0.0f;
	}
}

int32 URebirthPerkService::GetTotalRebirthPerkPoints(int32 RebirthCount)
{
	// 서버 getTotalRebirthPerkPoints: max(0, trunc(rebirthCount)). int32 입력은 이미 정수.
	return FMath::Max(0, RebirthCount);
}

float URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk Perk, int32 Level)
{
	// 서버 getPerkBonus: fround(PerkStep * max(0, level)). f32 곱셈으로 parity.
	const int32 SafeLevel = FMath::Max(0, Level);
	const float Step = GetPerkStep(Perk);
	return Step * static_cast<float>(SafeLevel);
}

float URebirthPerkService::GetPerkBonus(ERebirthPerk Perk) const
{
	return GetPerkBonusForLevel(Perk, GetPerkLevel(Perk));
}

int32 URebirthPerkService::GetPerkLevel(ERebirthPerk Perk) const
{
	const int32* Level = Allocations.Find(Perk);
	return Level ? FMath::Max(0, *Level) : 0;
}

int32 URebirthPerkService::GetSpent() const
{
	int32 Spent = 0;
	for (const TPair<ERebirthPerk, int32>& Pair : Allocations)
	{
		Spent += FMath::Max(0, Pair.Value);
	}
	return Spent;
}

int32 URebirthPerkService::GetAvailable() const
{
	return FMath::Max(0, TotalPoints - GetSpent());
}

bool URebirthPerkService::AllocatePerk(ERebirthPerk Perk)
{
	if (Perk == ERebirthPerk::None || GetAvailable() <= 0)
	{
		return false;
	}

	int32& Level = Allocations.FindOrAdd(Perk);
	Level = FMath::Max(0, Level) + 1;
	return true;
}

bool URebirthPerkService::DeallocatePerk(ERebirthPerk Perk)
{
	if (Perk == ERebirthPerk::None)
	{
		return false;
	}

	int32* Level = Allocations.Find(Perk);
	if (!Level || *Level <= 0)
	{
		return false;
	}

	--(*Level);
	if (*Level <= 0)
	{
		Allocations.Remove(Perk);
	}
	return true;
}

void URebirthPerkService::ResetPerks()
{
	Allocations.Empty();
}

void URebirthPerkService::SetTotalPoints(int32 InTotalPoints)
{
	TotalPoints = FMath::Max(0, InTotalPoints);
}

void URebirthPerkService::RestoreState(const TMap<ERebirthPerk, int32>& InAllocations)
{
	Allocations.Empty();
	for (const TPair<ERebirthPerk, int32>& Pair : InAllocations)
	{
		if (Pair.Key == ERebirthPerk::None || Pair.Value <= 0)
		{
			continue;
		}
		Allocations.Add(Pair.Key, Pair.Value);
	}
}
