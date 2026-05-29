#include "GameCore/DungeonService.h"

#include "GameCore/DungeonFormula.h"
#include "GameCore/QuestService.h"

namespace
{
FString NormalizeDungeonDate(const FString& TodayUtc)
{
	return TodayUtc.IsEmpty() ? UQuestService::GetCurrentUtcDateString() : TodayUtc;
}
}

FDungeonRunResult UDungeonService::TryRunDungeon(EDungeonType Type, int64 CombatPower, const FString& TodayUtc, int32 Tier)
{
	FDungeonRunResult Result;
	Result.Type = Type;
	Result.Tier = Tier;

	EnsureDailyReset(TodayUtc);
	if (GetRemainingEntries(Type, TodayUtc) <= 0)
	{
		return Result;
	}

	Result = FDungeonFormula::GetRewardForCp(Type, CombatPower, Tier);
	if (!Result.bSuccess)
	{
		return Result;
	}

	const int32 Used = FMath::Clamp(EntriesUsed.FindRef(Type), 0, FDungeonFormula::GetDailyEntryLimit(Type));
	EntriesUsed.Add(Type, Used + 1);
	return Result;
}

int64 UDungeonService::GetTierCpRequirement(EDungeonType Type, int32 Tier) const
{
	return FDungeonFormula::GetTierCpRequirement(Type, Tier);
}

int32 UDungeonService::GetMaxAccessibleTier(EDungeonType Type, int64 CombatPower) const
{
	return FDungeonFormula::GetMaxAccessibleTier(Type, CombatPower);
}

int32 UDungeonService::GetRemainingEntries(EDungeonType Type, const FString& TodayUtc) const
{
	const int32 Limit = FDungeonFormula::GetDailyEntryLimit(Type);
	if (Limit <= 0)
	{
		return 0;
	}

	const FString ResetDate = NormalizeDungeonDate(TodayUtc);
	if (DailyResetDate != ResetDate)
	{
		return Limit;
	}

	return FMath::Clamp(Limit - EntriesUsed.FindRef(Type), 0, Limit);
}

void UDungeonService::EnsureDailyReset(const FString& TodayUtc)
{
	const FString ResetDate = NormalizeDungeonDate(TodayUtc);
	if (DailyResetDate == ResetDate)
	{
		return;
	}

	DailyResetDate = ResetDate;
	EntriesUsed.Empty();
}

void UDungeonService::CaptureState(TMap<EDungeonType, int32>& OutEntriesUsed, FString& OutDailyReset) const
{
	OutEntriesUsed.Empty();
	for (const EDungeonType Type : { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence })
	{
		const int32 Limit = FDungeonFormula::GetDailyEntryLimit(Type);
		OutEntriesUsed.Add(Type, FMath::Clamp(EntriesUsed.FindRef(Type), 0, Limit));
	}
	OutDailyReset = DailyResetDate;
}

void UDungeonService::RestoreState(const TMap<EDungeonType, int32>& InEntriesUsed, const FString& InDailyReset)
{
	DailyResetDate = InDailyReset.IsEmpty() ? UQuestService::GetCurrentUtcDateString() : InDailyReset;
	EntriesUsed.Empty();
	for (const EDungeonType Type : { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence })
	{
		const int32 Limit = FDungeonFormula::GetDailyEntryLimit(Type);
		EntriesUsed.Add(Type, FMath::Clamp(InEntriesUsed.FindRef(Type), 0, Limit));
	}
}
