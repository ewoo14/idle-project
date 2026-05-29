#include "GameCore/MasteryService.h"

void UMasteryService::Initialize()
{
	for (int32 Index = 0; Index < FMasteryFormula::TrackCount; ++Index)
	{
		TrackXp[Index] = 0;
		CachedLevel[Index] = 0;
	}
}

void UMasteryService::AddXp(EMasteryTrack Track, int64 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	const int32 Index = ToIndex(Track);
	if (Index == INDEX_NONE)
	{
		return;
	}

	const int32 PreviousLevel = CachedLevel[Index];
	TrackXp[Index] = TrackXp[Index] > MAX_int64 - Amount ? MAX_int64 : TrackXp[Index] + Amount;
	RefreshCachedLevel(Index);
	const int32 NewLevel = CachedLevel[Index];
	if (NewLevel > PreviousLevel)
	{
		OnTrackLevelUp.Broadcast(Track, NewLevel);
	}
}

int32 UMasteryService::GetTrackLevel(EMasteryTrack Track) const
{
	return LevelOf(Track);
}

int64 UMasteryService::GetTrackTotalXp(EMasteryTrack Track) const
{
	const int32 Index = ToIndex(Track);
	return Index == INDEX_NONE ? 0 : TrackXp[Index];
}

FMasteryLevelInfo UMasteryService::GetTrackLevelInfo(EMasteryTrack Track) const
{
	FMasteryLevelInfo Info;
	const int32 Index = ToIndex(Track);
	if (Index == INDEX_NONE)
	{
		Info.XpToNext = FMasteryFormula::XpToNext(0);
		return Info;
	}

	Info.TotalXp = TrackXp[Index];
	FMasteryFormula::LevelFromTotalXp(Info.TotalXp, Info.Level, Info.XpIntoLevel, Info.XpToNext);
	return Info;
}

int64 UMasteryService::GetWorldPower() const
{
	int64 Total = 0;
	for (int32 Index = 0; Index < FMasteryFormula::TrackCount; ++Index)
	{
		Total += LevelOf(static_cast<EMasteryTrack>(Index));
	}
	return Total;
}

FMasteryGlobalBonus UMasteryService::GetGlobalBonus() const
{
	FMasteryGlobalBonus Bonus;
	const int32 CombatLevel = LevelOf(EMasteryTrack::Combat);
	const int32 EquipmentLevel = LevelOf(EMasteryTrack::Equipment);
	const int32 AbyssLevel = LevelOf(EMasteryTrack::Abyss);
	const int32 RuneLevel = LevelOf(EMasteryTrack::Rune);
	const int32 BeastLevel = LevelOf(EMasteryTrack::Beast);
	const int32 ExploreLevel = LevelOf(EMasteryTrack::Explore);

	Bonus.CoreStatMultiplier = FMasteryFormula::CoreStatMultiplier(CombatLevel, EquipmentLevel, ExploreLevel);
	Bonus.CritRateAdd = FMasteryFormula::CritRateAdd(RuneLevel);
	Bonus.DropRateAdd = FMasteryFormula::DropRateAdd(AbyssLevel);
	Bonus.GoldFindPct = FMasteryFormula::GoldFindPct(BeastLevel);
	Bonus.ExpBoostPct = FMasteryFormula::ExpBoostPct(BeastLevel);
	Bonus.WorldPower = GetWorldPower();
	return Bonus;
}

float UMasteryService::GetLocalBonus(EMasteryTrack Track) const
{
	return FMasteryFormula::GetLocalBonus(Track, GetTrackLevel(Track));
}

float UMasteryService::GetLocalBonus2(EMasteryTrack Track) const
{
	return FMasteryFormula::GetLocalBonus2(Track, GetTrackLevel(Track));
}

int32 UMasteryService::GetAbyssBonusEntries() const
{
	return FMasteryFormula::GetAbyssBonusEntries(GetTrackLevel(EMasteryTrack::Abyss));
}

TArray<FMasterySaveEntry> UMasteryService::ExportSave() const
{
	TArray<FMasterySaveEntry> Entries;
	Entries.Reserve(FMasteryFormula::TrackCount);
	for (int32 Index = 0; Index < FMasteryFormula::TrackCount; ++Index)
	{
		FMasterySaveEntry Entry;
		Entry.Track = static_cast<uint8>(Index);
		Entry.TotalXp = TrackXp[Index];
		Entries.Add(Entry);
	}
	return Entries;
}

void UMasteryService::ImportSave(const TArray<FMasterySaveEntry>& Entries)
{
	Initialize();
	for (const FMasterySaveEntry& Entry : Entries)
	{
		if (Entry.Track < FMasteryFormula::TrackCount)
		{
			TrackXp[Entry.Track] = FMath::Max<int64>(0, Entry.TotalXp);
			RefreshCachedLevel(Entry.Track);
		}
	}
}

int32 UMasteryService::ToIndex(EMasteryTrack Track)
{
	const int32 Index = static_cast<int32>(Track);
	return Index >= 0 && Index < FMasteryFormula::TrackCount ? Index : INDEX_NONE;
}

void UMasteryService::RefreshCachedLevel(int32 Index)
{
	if (Index < 0 || Index >= FMasteryFormula::TrackCount)
	{
		return;
	}

	int32 Level = 0;
	int64 Into = 0;
	int64 Need = 0;
	FMasteryFormula::LevelFromTotalXp(TrackXp[Index], Level, Into, Need);
	CachedLevel[Index] = Level;
}

int32 UMasteryService::LevelOf(EMasteryTrack Track) const
{
	const int32 Index = ToIndex(Track);
	return Index == INDEX_NONE ? 0 : CachedLevel[Index];
}
