#include "GameCore/BuffService.h"

#include "GameCore/ConsumableFormula.h"

uint32 UBuffService::MakeCountKey(EConsumableType Type, EConsumableGrade Grade)
{
	return (static_cast<uint32>(Type) << 8) | static_cast<uint32>(Grade);
}

void UBuffService::Initialize()
{
	Counts.Reset();
	BuffStateByType.Reset();
}

int32 UBuffService::GetCount(EConsumableType Type, EConsumableGrade Grade) const
{
	if (!FConsumableFormula::IsValidGrade(Grade))
	{
		return 0;
	}
	return FMath::Max(0, Counts.FindRef(MakeCountKey(Type, Grade)));
}

int32 UBuffService::GetTotalCount(EConsumableType Type) const
{
	int64 Total = 0;
	const EConsumableGrade Grades[] = { EConsumableGrade::Lesser, EConsumableGrade::Standard, EConsumableGrade::Greater };
	for (const EConsumableGrade Grade : Grades)
	{
		Total += GetCount(Type, Grade);
	}
	return static_cast<int32>(FMath::Min<int64>(MAX_int32, Total));
}

int32 UBuffService::GetCount(EConsumableType Type) const
{
	return GetTotalCount(Type);
}

void UBuffService::AddConsumable(EConsumableType Type, EConsumableGrade Grade, int32 Amount)
{
	if (!FConsumableFormula::IsValidType(Type) || !FConsumableFormula::IsValidGrade(Grade) || Amount <= 0)
	{
		return;
	}

	const int32 Current = GetCount(Type, Grade);
	Counts.Add(MakeCountKey(Type, Grade), Current > MAX_int32 - Amount ? MAX_int32 : Current + Amount);
}

void UBuffService::AddConsumable(EConsumableType Type, int32 Amount)
{
	AddConsumable(Type, EConsumableGrade::Standard, Amount);
}

bool UBuffService::UseConsumable(EConsumableType Type, EConsumableGrade Grade, int64 NowUnixSec)
{
	if (!FConsumableFormula::IsValidGrade(Grade))
	{
		return false;
	}

	const int32 Current = GetCount(Type, Grade);
	const int64 Duration = FConsumableFormula::GetBuffDurationSec(Type, Grade);
	if (Current <= 0 || Duration <= 0)
	{
		return false;
	}

	Counts.Add(MakeCountKey(Type, Grade), Current - 1);
	const int64 SafeNow = FMath::Max<int64>(0, NowUnixSec);
	const int64 EndUnixSec = SafeNow > MAX_int64 - Duration ? MAX_int64 : SafeNow + Duration;

	FBuffState& State = BuffStateByType.FindOrAdd(Type);
	State.EndUnixSec = EndUnixSec;
	State.ActiveGrade = Grade;

	OnBuffActivated.Broadcast(Type, EndUnixSec);
	return true;
}

bool UBuffService::UseConsumable(EConsumableType Type, int64 NowUnixSec)
{
	// Standard 우선, 없으면 Lesser, Greater 순으로 보유 등급을 사용합니다.
	const EConsumableGrade Priority[] = { EConsumableGrade::Standard, EConsumableGrade::Lesser, EConsumableGrade::Greater };
	for (const EConsumableGrade Grade : Priority)
	{
		if (GetCount(Type, Grade) > 0)
		{
			return UseConsumable(Type, Grade, NowUnixSec);
		}
	}
	return false;
}

bool UBuffService::IsBuffActive(EConsumableType Type, int64 NowUnixSec) const
{
	if (!FConsumableFormula::IsValidType(Type))
	{
		return false;
	}
	const FBuffState* State = BuffStateByType.Find(Type);
	return State != nullptr && FMath::Max<int64>(0, NowUnixSec) < State->EndUnixSec;
}

EConsumableGrade UBuffService::GetActiveGrade(EConsumableType Type, int64 NowUnixSec) const
{
	const FBuffState* State = BuffStateByType.Find(Type);
	if (State != nullptr && IsBuffActive(Type, NowUnixSec))
	{
		return State->ActiveGrade;
	}
	return EConsumableGrade::Standard;
}

int64 UBuffService::GetBuffRemainingSec(EConsumableType Type, int64 NowUnixSec) const
{
	const FBuffState* State = BuffStateByType.Find(Type);
	const int64 EndUnixSec = State != nullptr ? State->EndUnixSec : 0;
	return FMath::Max<int64>(0, EndUnixSec - FMath::Max<int64>(0, NowUnixSec));
}

float UBuffService::GetBuffStatMultiplier(EConsumableType Type, int64 NowUnixSec) const
{
	return IsBuffActive(Type, NowUnixSec) ? 1.0f + FConsumableFormula::GetBuffPercent(Type, GetActiveGrade(Type, NowUnixSec)) : 1.0f;
}

float UBuffService::GetGoldBuffPct(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::GoldFeast, NowUnixSec)
		? FConsumableFormula::GetBuffPercent(EConsumableType::GoldFeast, GetActiveGrade(EConsumableType::GoldFeast, NowUnixSec))
		: 0.0f;
}

float UBuffService::GetExpBuffPct(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::WisdomBooster, NowUnixSec)
		? FConsumableFormula::GetBuffPercent(EConsumableType::WisdomBooster, GetActiveGrade(EConsumableType::WisdomBooster, NowUnixSec))
		: 0.0f;
}

float UBuffService::GetDropBuffAdd(int64 NowUnixSec) const
{
	return IsBuffActive(EConsumableType::FortuneScroll, NowUnixSec)
		? FConsumableFormula::GetBuffPercent(EConsumableType::FortuneScroll, GetActiveGrade(EConsumableType::FortuneScroll, NowUnixSec))
		: 0.0f;
}

TArray<FConsumableSaveEntry> UBuffService::ExportSave() const
{
	TArray<FConsumableSaveEntry> Entries;
	const EConsumableGrade Grades[] = { EConsumableGrade::Lesser, EConsumableGrade::Standard, EConsumableGrade::Greater };
	for (uint8 RawType = static_cast<uint8>(EConsumableType::AttackTonic); RawType <= static_cast<uint8>(EConsumableType::WisdomBooster); ++RawType)
	{
		const EConsumableType Type = static_cast<EConsumableType>(RawType);
		const FBuffState* State = BuffStateByType.Find(Type);
		const int64 EndUnixSec = State != nullptr ? FMath::Max<int64>(0, State->EndUnixSec) : 0;
		const EConsumableGrade ActiveGrade = State != nullptr ? State->ActiveGrade : EConsumableGrade::Standard;

		for (const EConsumableGrade Grade : Grades)
		{
			const int32 Count = GetCount(Type, Grade);
			// 활성 버프 상태(종료시각+활성등급)는 활성 등급 엔트리에만 1회 기록합니다.
			const bool bWriteBuff = EndUnixSec > 0 && Grade == ActiveGrade;
			if (Count <= 0 && !bWriteBuff)
			{
				continue;
			}

			FConsumableSaveEntry Entry;
			Entry.Type = RawType;
			Entry.Grade = static_cast<uint8>(Grade);
			Entry.Count = Count;
			Entry.BuffEndUnixSec = bWriteBuff ? EndUnixSec : 0;
			Entries.Add(Entry);
		}
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

		// v15 이하 엔트리는 Grade 필드가 없어 기본값 Standard(1) 로 마이그레이션됩니다.
		EConsumableGrade Grade = static_cast<EConsumableGrade>(Entry.Grade);
		if (!FConsumableFormula::IsValidGrade(Grade))
		{
			Grade = EConsumableGrade::Standard;
		}

		const int32 Count = FMath::Max(0, Entry.Count);
		const int64 EndUnixSec = FMath::Max<int64>(0, Entry.BuffEndUnixSec);
		if (Count > 0)
		{
			Counts.Add(MakeCountKey(Type, Grade), Count);
		}
		if (EndUnixSec > 0)
		{
			FBuffState& State = BuffStateByType.FindOrAdd(Type);
			// 같은 타입에 여러 엔트리가 있으면 더 늦은 종료시각을 활성으로 유지합니다.
			if (EndUnixSec >= State.EndUnixSec)
			{
				State.EndUnixSec = EndUnixSec;
				State.ActiveGrade = Grade;
			}
		}
	}
}
