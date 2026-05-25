#include "GameCore/QuestService.h"

void UQuestService::InitializeDefaultQuests(const FString& CurrentUtcDate)
{
	BuildDefaultDefinitions();
	ActiveStates.Empty();
	DailyResetDate = CurrentUtcDate.IsEmpty() ? GetCurrentUtcDateString() : CurrentUtcDate;

	for (const FQuestDefinition& Definition : Definitions)
	{
		if (Definition.Type == EQuestType::Daily || Definition.PrerequisiteQuestId.IsEmpty())
		{
			AddActiveQuest(Definition, DailyResetDate);
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
			AddActiveQuest(Definition, DailyResetDate);
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

		State.Progress = FMath::Clamp(State.Progress + Amount, 0, State.TargetCount);
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
			AddActiveQuest(Definition, DailyResetDate);
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

FString UQuestService::GetCurrentUtcDateString()
{
	return FDateTime::UtcNow().ToString(TEXT("%Y-%m-%d"));
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
		Definition.Title = FText::FromString(Title);
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
	AddDefinition(TEXT("daily_kill_monsters"), EQuestType::Daily, TEXT("Daily Hunt"), EQuestObjective::KillMonster, 30, 500, 240);
	AddDefinition(TEXT("daily_claim_offline"), EQuestType::Daily, TEXT("Claim Rest Rewards"), EQuestObjective::ClaimOffline, 1, 300, 180);
	AddDefinition(TEXT("daily_enhance_gear"), EQuestType::Daily, TEXT("Temper Equipment"), EQuestObjective::Enhance, 3, 650, 320);
}

void UQuestService::AddActiveQuest(const FQuestDefinition& Definition, const FString& CurrentDailyResetDate)
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
	ActiveStates.Add(Definition.QuestId, State);
}

bool UQuestService::IsQuestActive(const FString& QuestId) const
{
	return ActiveStates.Contains(QuestId);
}
