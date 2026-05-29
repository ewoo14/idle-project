#include "GameCore/BuffService.h"

#include "GameCore/ConsumableFormula.h"

void UBuffService::Initialize()
{
	Counts.Reset();
	BuffEndUnixSecByType.Reset();
}

int32 UBuffService::GetCount(EConsumableType Type) const
{
	return FMath::Max(0, Counts.FindRef(Type));
}

void UBuffService::AddConsumable(EConsumableType Type, int32 Amount)
{
	if (!FConsumableFormula::IsValidType(Type) || Amount <= 0)
	{
		return;
	}

	const int32 Current = GetCount(Type);
	Counts.Add(Type, Current > MAX_int32 - Amount ? MAX_int32 : Current + Amount);
}

bool UBuffService::UseConsumable(EConsumableType Type, int64 NowUnixSec)
{
	const int32 Current = GetCount(Type);
	const int64 Duration = FConsumableFormula::GetBuffDurationSec(Type);
	if (Current <= 0 || Duration <= 0)
	{
		return false;
	}

	Counts.Add(Type, Current - 1);
	const int64 SafeNow = FMath::Max<int64>(0, NowUnixSec);
	const int64 EndUnixSec = SafeNow > MAX_int64 - Duration ? MAX_int64 : SafeNow + Duration;
	BuffEndUnixSecByType.Add(Type, EndUnixSec);
	OnBuffActivated.Broadcast(Type, EndUnixSec);
	return true;
}

bool UBuffService::IsBuffActive(EConsumableType Type, int64 NowUnixSec) const
{
	return FConsumableFormula::IsValidType(Type) && FMath::Max<int64>(0, NowUnixSec) < BuffEndUnixSecByType.FindRef(Type);
}

int64 UBuffService::GetBuffRemainingSec(EConsumableType Type, int64 NowUnixSec) const
{
	return FMath::Max<int64>(0, BuffEndUnixSecByType.FindRef(Type) - FMath::Max<int64>(0, NowUnixSec));
}

float UBuffService::GetBuffStatMultiplier(EConsumableType Type, int64 NowUnixSec) const
{
	return IsBuffActive(Type, NowUnixSec) ? 1.0f + FConsumableFormula::GetBuffPercent(Type) : 1.0f;
}

float UBuffService::GetGoldBuffPct(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::GoldFeast, NowUnixSec) ? FConsumableFormula::GetBuffPercent(EConsumableType::GoldFeast) : 0.0f;
}

float UBuffService::GetExpBuffPct(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::WisdomBooster, NowUnixSec) ? FConsumableFormula::GetBuffPercent(EConsumableType::WisdomBooster) : 0.0f;
}

float UBuffService::GetDropBuffAdd(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::FortuneScroll, NowUnixSec) ? FConsumableFormula::GetBuffPercent(EConsumableType::FortuneScroll) : 0.0f;
}

TArray<FConsumableSaveEntry> UBuffService::ExportSave() const
{
	TArray<FConsumableSaveEntry> Entries;
	for (uint8 RawType = static_cast<uint8>(EConsumableType::AttackTonic); RawType <= static_cast<uint8>(EConsumableType::WisdomBooster); ++RawType)
	{
		const EConsumableType Type = static_cast<EConsumableType>(RawType);
		const int32 Count = GetCount(Type);
		const int64 EndUnixSec = BuffEndUnixSecByType.FindRef(Type);
		if (Count <= 0 && EndUnixSec <= 0)
		{
			continue;
		}

		FConsumableSaveEntry Entry;
		Entry.Type = RawType;
		Entry.Count = Count;
		Entry.BuffEndUnixSec = FMath::Max<int64>(0, EndUnixSec);
		Entries.Add(Entry);
	}
	return Entries;
}

void UBuffService::ImportSave(const TArray<FConsumableSaveEntry>& Entries)
{
	Initialize();
	for (const FConsumableSaveEntry& Entry : Entries)
	{
		const EConsumableType Type = static_cast<EConsumableType>(Entry.Type);
		if (!FConsumableFormula::IsValidType(Type))
		{
			continue;
		}

		const int32 Count = FMath::Max(0, Entry.Count);
		const int64 EndUnixSec = FMath::Max<int64>(0, Entry.BuffEndUnixSec);
		if (Count > 0)
		{
			Counts.Add(Type, Count);
		}
		if (EndUnixSec > 0)
		{
			BuffEndUnixSecByType.Add(Type, EndUnixSec);
		}
	}
}
