#include "GameCore/QuestService.h"

#include "Internationalization/IdleLocalization.h"

namespace
{
FString GetUtcWeekString(const FDateTime& Date)
{
	const int32 DayOfYear = Date.GetDayOfYear();
	int32 IsoWeekday = 1;
	switch (Date.GetDayOfWeek())
	{
	case EDayOfWeek::Monday:
		IsoWeekday = 1;
		break;
	case EDayOfWeek::Tuesday:
		IsoWeekday = 2;
		break;
	case EDayOfWeek::Wednesday:
		IsoWeekday = 3;
		break;
	case EDayOfWeek::Thursday:
		IsoWeekday = 4;
		break;
	case EDayOfWeek::Friday:
		IsoWeekday = 5;
		break;
	case EDayOfWeek::Saturday:
		IsoWeekday = 6;
		break;
	case EDayOfWeek::Sunday:
		IsoWeekday = 7;
		break;
	default:
		break;
	}
	const int32 Week = FMath::Clamp(FMath::FloorToInt(static_cast<float>(DayOfYear - IsoWeekday + 10) / 7.0f), 1, 53);
	return FString::Printf(TEXT("%04d-W%02d"), Date.GetYear(), Week);
}

FString GetUtcWeekStringFromDateString(const FString& DateString)
{
	TArray<FString> Parts;
	if (DateString.ParseIntoArray(Parts, TEXT("-")) == 3)
	{
		int32 Year = 0;
		int32 Month = 0;
		int32 Day = 0;
		if (LexTryParseString(Year, *Parts[0])
			&& LexTryParseString(Month, *Parts[1])
			&& LexTryParseString(Day, *Parts[2])
			&& Year > 0
			&& Month >= 1
			&& Month <= 12
			&& Day >= 1
			&& Day <= 31)
		{
			return GetUtcWeekString(FDateTime(Year, Month, Day));
		}
	}

	return UQuestService::GetCurrentUtcWeekString();
}
}

void UQuestService::InitializeDefaultQuests(const FString& CurrentUtcDate)
{
	BuildDefaultDefinitions();
	ActiveStates.Empty();
	DailyResetDate = CurrentUtcDate.IsEmpty() ? GetCurrentUtcDateString() : CurrentUtcDate;
	WeeklyResetId = CurrentUtcDate.IsEmpty() ? GetCurrentUtcWeekString() : GetUtcWeekStringFromDateString(DailyResetDate);

	for (const FQuestDefinition& Definition : Definitions)
	{
		if (Definition.Type == EQuestType::Daily || Definition.Type == EQuestType::Weekly || Definition.PrerequisiteQuestId.IsEmpty())
		{
			AddActiveQuest(Definition, DailyResetDate, WeeklyResetId);
		}
	}
}

void UQuestService::ResetDailyQuestsIfNeeded(const FString& CurrentUtcDate)
{
	const FString ResetDate = CurrentUtcDate.IsEmpty() ? GetCurrentUtcDateString() : CurrentUtcDate;
	if (DailyResetDate == ResetDate)
	{
		return;
	}

	DailyResetDate = ResetDate;
	for (const FQuestDefinition& Definition : Definitions)
	{
		if (Definition.Type == EQuestType::Daily)
		{
			AddActiveQuest(Definition, DailyResetDate, WeeklyResetId);
		}
	}
}

void UQuestService::ResetWeeklyQuestsIfNeeded(const FString& CurrentUtcWeek)
{
	const FString ResetWeek = CurrentUtcWeek.IsEmpty() ? GetCurrentUtcWeekString() : CurrentUtcWeek;
	if (WeeklyResetId == ResetWeek)
	{
		return;
	}

	WeeklyResetId = ResetWeek;
	for (const FQuestDefinition& Definition : Definitions)
	{
		if (Definition.Type == EQuestType::Weekly)
		{
			AddActiveQuest(Definition, DailyResetDate, WeeklyResetId);
		}
	}
}

void UQuestService::RecordProgress(EQuestObjective Objective, int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	for (TPair<FString, FQuestState>& Pair : ActiveStates)
	{
		FQuestState& State = Pair.Value;
		if (State.Objective != Objective || State.bClaimed)
		{
			continue;
		}

		if (Objective == EQuestObjective::ReachLevel)
		{
			State.Progress = FMath::Clamp(FMath::Max(State.Progress, Amount), 0, State.TargetCount);
		}
		else
		{
			State.Progress = FMath::Clamp(State.Progress + Amount, 0, State.TargetCount);
		}
		State.bCompleted = State.Progress >= State.TargetCount;
	}
}

FQuestClaimResult UQuestService::ClaimQuest(const FString& QuestId)
{
	FQuestClaimResult Result;
	FQuestState* State = ActiveStates.Find(QuestId);
	if (!State)
	{
		Result.Message = TEXT("quest_not_active");
		return Result;
	}
	if (!State->bCompleted)
	{
		Result.Message = TEXT("quest_not_completed");
		Result.Quest = *State;
		return Result;
	}
	if (State->bClaimed)
	{
		Result.Message = TEXT("quest_already_claimed");
		Result.Quest = *State;
		return Result;
	}

	State->bClaimed = true;
	Result.bSuccess = true;
	Result.Message = TEXT("claimed");
	Result.RewardGold = State->RewardGold;
	Result.RewardExp = State->RewardExp;
	Result.Quest = *State;

	for (const FQuestDefinition& Definition : Definitions)
	{
		if (Definition.Type == EQuestType::Main
			&& Definition.PrerequisiteQuestId == QuestId
			&& !IsQuestActive(Definition.QuestId))
		{
			AddActiveQuest(Definition, DailyResetDate, WeeklyResetId);
			Result.UnlockedQuestIds.Add(Definition.QuestId);
		}
	}

	return Result;
}

TArray<FQuestState> UQuestService::GetActiveQuestStates() const
{
	TArray<FQuestState> States;
	ActiveStates.GenerateValueArray(States);
	States.Sort([](const FQuestState& Left, const FQuestState& Right)
	{
		if (Left.Type != Right.Type)
		{
			return Left.Type == EQuestType::Main;
		}
		return Left.QuestId < Right.QuestId;
	});
	return States;
}

const TArray<FQuestDefinition>& UQuestService::GetQuestDefinitions() const
{
	return Definitions;
}

bool UQuestService::GetQuestState(const FString& QuestId, FQuestState& OutState) const
{
	if (const FQuestState* State = ActiveStates.Find(QuestId))
	{
		OutState = *State;
		return true;
	}
	return false;
}

void UQuestService::CaptureState(TArray<FQuestSaveEntry>& OutEntries, FString& OutDailyReset) const
{
	FString IgnoredWeeklyReset;
	CaptureState(OutEntries, OutDailyReset, IgnoredWeeklyReset);
}

void UQuestService::CaptureState(TArray<FQuestSaveEntry>& OutEntries, FString& OutDailyReset, FString& OutWeeklyReset) const
{
	OutEntries.Reset();
	OutDailyReset = DailyResetDate;
	OutWeeklyReset = WeeklyResetId;

	TArray<FQuestState> States = GetActiveQuestStates();
	for (const FQuestState& State : States)
	{
		FQuestSaveEntry Entry;
		Entry.QuestId = State.QuestId;
		Entry.Type = State.Type;
		Entry.Progress = State.Progress;
		Entry.bCompleted = State.bCompleted;
		Entry.bClaimed = State.bClaimed;
		Entry.DailyResetDate = State.DailyResetDate;
		Entry.WeeklyResetId = State.WeeklyResetId;
		OutEntries.Add(Entry);
	}
}

void UQuestService::RestoreState(const TArray<FQuestSaveEntry>& InEntries, const FString& InDailyReset)
{
	RestoreState(InEntries, InDailyReset, FString());
}

void UQuestService::RestoreState(const TArray<FQuestSaveEntry>& InEntries, const FString& InDailyReset, const FString& InWeeklyReset)
{
	const FString RestoreDate = InDailyReset.IsEmpty() ? GetCurrentUtcDateString() : InDailyReset;
	InitializeDefaultQuests(RestoreDate);
	WeeklyResetId = InWeeklyReset.IsEmpty() ? GetUtcWeekStringFromDateString(RestoreDate) : InWeeklyReset;

	for (const FQuestSaveEntry& Entry : InEntries)
	{
		const FQuestDefinition* Definition = DefinitionById.Find(Entry.QuestId);
		if (!Definition)
		{
			continue;
		}
		if (!IsQuestActive(Entry.QuestId))
		{
			AddActiveQuest(*Definition, RestoreDate, WeeklyResetId);
		}

		FQuestState& State = ActiveStates.FindChecked(Entry.QuestId);
		State.Progress = FMath::Clamp(Entry.Progress, 0, State.TargetCount);
		State.bCompleted = Entry.bCompleted || State.Progress >= State.TargetCount;
		State.bClaimed = Entry.bClaimed && State.bCompleted;
		State.DailyResetDate = State.Type == EQuestType::Daily
			? (Entry.DailyResetDate.IsEmpty() ? RestoreDate : Entry.DailyResetDate)
			: FString();
		State.WeeklyResetId = State.Type == EQuestType::Weekly
			? (Entry.WeeklyResetId.IsEmpty() ? WeeklyResetId : Entry.WeeklyResetId)
			: FString();
	}
}

FString UQuestService::GetCurrentUtcDateString()
{
	return FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d"));
}

FString UQuestService::GetCurrentUtcWeekString()
{
	return GetUtcWeekString(FDateTime::UtcNow());
}

void UQuestService::BuildDefaultDefinitions()
{
	if (!Definitions.IsEmpty())
	{
		return;
	}

	auto AddDefinition = [this](
		const TCHAR* QuestId,
		EQuestType Type,
		const TCHAR* Title,
		EQuestObjective Objective,
		int32 TargetCount,
		int64 RewardGold,
		int64 RewardExp,
		const TCHAR* PrerequisiteQuestId = TEXT(""),
		const TCHAR* ChapterMapId = TEXT(""))
	{
		FQuestDefinition Definition;
		Definition.QuestId = QuestId;
		Definition.Type = Type;
		Definition.Title = IdleProject::Localization::Text(TEXT("Quest"), QuestId);
		Definition.Objective = Objective;
		Definition.TargetCount = TargetCount;
		Definition.RewardGold = RewardGold;
		Definition.RewardExp = RewardExp;
		Definition.PrerequisiteQuestId = PrerequisiteQuestId;
		Definition.ChapterMapId = ChapterMapId;
		Definitions.Add(Definition);
		DefinitionById.Add(Definition.QuestId, Definition);
	};

	AddDefinition(TEXT("main_ch1_001"), EQuestType::Main, TEXT("Find the Broken Gate"), EQuestObjective::KillMonster, 5, 150, 80, TEXT(""), TEXT("1-1"));
	AddDefinition(TEXT("main_ch1_002"), EQuestType::Main, TEXT("Secure the Village Field"), EQuestObjective::ClearMap, 1, 220, 140, TEXT("main_ch1_001"), TEXT("1-1"));
	AddDefinition(TEXT("main_ch1_003"), EQuestType::Main, TEXT("Repeating Echoes"), EQuestObjective::KillMonster, 12, 420, 300, TEXT("main_ch1_002"), TEXT("1-2"));
	AddDefinition(TEXT("main_ch1_004"), EQuestType::Main, TEXT("Trace of the Guardian"), EQuestObjective::ClearMap, 1, 700, 520, TEXT("main_ch1_003"), TEXT("1-3"));
	AddDefinition(TEXT("main_ch1_005"), EQuestType::Main, TEXT("Gate of the Winged Legion"), EQuestObjective::KillMonster, 20, 1200, 900, TEXT("main_ch1_004"), TEXT("1-5"));
	AddDefinition(TEXT("main_ch1_006"), EQuestType::Main, TEXT("Temper the Gate Key"), EQuestObjective::Enhance, 2, 1600, 1200, TEXT("main_ch1_005"), TEXT("1-5"));
	AddDefinition(TEXT("main_ch1_007"), EQuestType::Main, TEXT("Break the First Seal"), EQuestObjective::DefeatBoss, 1, 2200, 1600, TEXT("main_ch1_006"), TEXT("1-5"));
	AddDefinition(TEXT("main_ch2_001"), EQuestType::Main, TEXT("Enter the Ashen Road"), EQuestObjective::KillMonster, 25, 2600, 1900, TEXT("main_ch1_007"), TEXT("2-1"));
	AddDefinition(TEXT("main_ch2_002"), EQuestType::Main, TEXT("Map the Ember Ruins"), EQuestObjective::ClearMap, 1, 3200, 2300, TEXT("main_ch2_001"), TEXT("2-2"));
	AddDefinition(TEXT("main_ch2_003"), EQuestType::Main, TEXT("Reach the Watch Spire"), EQuestObjective::ReachLevel, 10, 3900, 2800, TEXT("main_ch2_002"), TEXT("2-3"));
	AddDefinition(TEXT("main_ch2_004"), EQuestType::Main, TEXT("Reforge Through Rebirth"), EQuestObjective::Rebirth, 1, 4800, 3400, TEXT("main_ch2_003"), TEXT("2-4"));
	AddDefinition(TEXT("main_ch2_005"), EQuestType::Main, TEXT("Defeat the Ember Warden"), EQuestObjective::DefeatBoss, 1, 6200, 4500, TEXT("main_ch2_004"), TEXT("2-5"));
	AddDefinition(TEXT("main_ch3_001"), EQuestType::Main, TEXT("Enter the Rift Frontier"), EQuestObjective::KillMonster, 35, 7600, 5400, TEXT("main_ch2_005"), TEXT("3-1"));
	AddDefinition(TEXT("main_ch3_002"), EQuestType::Main, TEXT("Chart the Shadow Fault"), EQuestObjective::ClearMap, 1, 8800, 6200, TEXT("main_ch3_001"), TEXT("3-2"));
	AddDefinition(TEXT("main_ch3_003"), EQuestType::Main, TEXT("Strength for the Abyss Gate"), EQuestObjective::ReachLevel, 25, 10200, 7300, TEXT("main_ch3_002"), TEXT("3-4"));
	AddDefinition(TEXT("main_ch3_004"), EQuestType::Main, TEXT("Light the Rift Beacon"), EQuestObjective::ClimbTower, 15, 11800, 8400, TEXT("main_ch3_003"), TEXT("3-5"));
	AddDefinition(TEXT("main_ch3_005"), EQuestType::Main, TEXT("Silence the Umbral Legion"), EQuestObjective::KillMonster, 50, 13600, 9800, TEXT("main_ch3_004"), TEXT("3-8"));
	AddDefinition(TEXT("main_ch3_006"), EQuestType::Main, TEXT("Defeat the Dimension Lord"), EQuestObjective::DefeatBoss, 1, 16000, 12000, TEXT("main_ch3_005"), TEXT("3-10"));
	AddDefinition(TEXT("daily_kill_monsters"), EQuestType::Daily, TEXT("Daily Hunt"), EQuestObjective::KillMonster, 30, 500, 240);
	AddDefinition(TEXT("daily_claim_offline"), EQuestType::Daily, TEXT("Claim Rest Rewards"), EQuestObjective::ClaimOffline, 1, 300, 180);
	AddDefinition(TEXT("daily_enhance_gear"), EQuestType::Daily, TEXT("Temper Equipment"), EQuestObjective::Enhance, 3, 650, 320);
	AddDefinition(TEXT("daily_reach_level"), EQuestType::Daily, TEXT("Push Your Training"), EQuestObjective::ReachLevel, 10, 700, 360);
	AddDefinition(TEXT("daily_spend_gold"), EQuestType::Daily, TEXT("Keep the Market Moving"), EQuestObjective::SpendGold, 1000, 750, 380);
	AddDefinition(TEXT("daily_roll_gear_shop"), EQuestType::Daily, TEXT("Try the Gear Shop"), EQuestObjective::RollGearShop, 1, 850, 420);
	AddDefinition(TEXT("daily_feed_pet"), EQuestType::Daily, TEXT("Feed a Companion"), EQuestObjective::FeedPet, 1, 900, 450);
	AddDefinition(TEXT("weekly_defeat_bosses"), EQuestType::Weekly, TEXT("Weekly Boss Breaker"), EQuestObjective::DefeatBoss, 3, 5000, 2500);
	AddDefinition(TEXT("weekly_rebirth"), EQuestType::Weekly, TEXT("Weekly Rebirth"), EQuestObjective::Rebirth, 1, 8000, 4000);
	AddDefinition(TEXT("weekly_climb_tower"), EQuestType::Weekly, TEXT("Weekly Tower Push"), EQuestObjective::ClimbTower, 10, 7000, 3600);
	AddDefinition(TEXT("weekly_spend_gold"), EQuestType::Weekly, TEXT("Weekly Gold Sink"), EQuestObjective::SpendGold, 10000, 6500, 3200);
}

void UQuestService::AddActiveQuest(const FQuestDefinition& Definition, const FString& CurrentDailyResetDate)
{
	AddActiveQuest(Definition, CurrentDailyResetDate, WeeklyResetId);
}

void UQuestService::AddActiveQuest(const FQuestDefinition& Definition, const FString& CurrentDailyResetDate, const FString& CurrentWeeklyResetId)
{
	FQuestState State;
	State.QuestId = Definition.QuestId;
	State.Type = Definition.Type;
	State.Title = Definition.Title;
	State.Objective = Definition.Objective;
	State.TargetCount = Definition.TargetCount;
	State.RewardGold = Definition.RewardGold;
	State.RewardExp = Definition.RewardExp;
	State.PrerequisiteQuestId = Definition.PrerequisiteQuestId;
	State.ChapterMapId = Definition.ChapterMapId;
	State.DailyResetDate = Definition.Type == EQuestType::Daily ? CurrentDailyResetDate : FString();
	State.WeeklyResetId = Definition.Type == EQuestType::Weekly ? CurrentWeeklyResetId : FString();
	ActiveStates.Add(Definition.QuestId, State);
}

bool UQuestService::IsQuestActive(const FString& QuestId) const
{
	return ActiveStates.Contains(QuestId);
}
