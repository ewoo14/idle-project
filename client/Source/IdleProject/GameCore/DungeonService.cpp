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

FDungeonRunResult UDungeonService::TryRunDungeon(EDungeonType Type, int64 CombatPower, const FString& TodayUtc, int32 Tier, int32 BonusEntries)
{
	FDungeonRunResult Result;
	Result.Type = Type;
	Result.Tier = Tier;

	EnsureDailyReset(TodayUtc);
	if (GetRemainingEntries(Type, TodayUtc, BonusEntries) <= 0)
	{
		return Result;
	}

	Result = FDungeonFormula::GetRewardForCp(Type, CombatPower, Tier);
	if (!Result.bSuccess)
	{
		return Result;
	}

	// 심연 마스터리 2종(BonusEntries)으로 확장된 한도까지 입장 사용량을 클램프.
	const int32 EffectiveLimit = FDungeonFormula::GetDailyEntryLimit(Type) + FMath::Max(0, BonusEntries);
	const int32 Used = FMath::Clamp(EntriesUsed.FindRef(Type), 0, EffectiveLimit);
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

int32 UDungeonService::GetRemainingEntries(EDungeonType Type, const FString& TodayUtc, int32 BonusEntries) const
{
	const int32 BaseLimit = FDungeonFormula::GetDailyEntryLimit(Type);
	if (BaseLimit <= 0)
	{
		return 0;
	}

	// 심연 마스터리 2종: 기본 한도에 정수 보너스 입장(+N)을 더함.
	const int32 Limit = BaseLimit + FMath::Max(0, BonusEntries);
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
