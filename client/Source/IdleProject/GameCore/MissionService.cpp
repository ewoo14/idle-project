#include "GameCore/MissionService.h"

#include "GameCore/QuestService.h"

void UMissionService::InitializeDefaultMissions()
{
	BuildDefaultDefinitions();
}

void UMissionService::BuildDefaultDefinitions()
{
	if (!Definitions.IsEmpty())
	{
		return;
	}

	auto AddDefinition = [this](const TCHAR* Id, EMissionPeriod Period, EMissionMetric Metric, int64 Target, EMissionReward RewardType, int64 RewardValue)
	{
		FMissionDefinition Definition;
		Definition.Id = Id;
		Definition.Period = Period;
		Definition.Metric = Metric;
		Definition.Target = Target;
		Definition.RewardType = RewardType;
		Definition.RewardValue = RewardValue;
		Definitions.Add(Definition);
		DefinitionById.Add(Definition.Id, Definition);
	};

	// 서버 mission.ts MISSION_CATALOG 10종(일일 6 + 주간 4) 1:1 이식.
	// 일일(Daily) — target 소
	AddDefinition(TEXT("daily_kill_300"), EMissionPeriod::Daily, EMissionMetric::MonstersKilled, 300, EMissionReward::Gold, 50000);
	AddDefinition(TEXT("daily_stage_20"), EMissionPeriod::Daily, EMissionMetric::StagesCleared, 20, EMissionReward::Essence, 5);
	AddDefinition(TEXT("daily_dungeon_3"), EMissionPeriod::Daily, EMissionMetric::DungeonRuns, 3, EMissionReward::Consumable, 1);
	AddDefinition(TEXT("daily_enhance_10"), EMissionPeriod::Daily, EMissionMetric::GearEnhanced, 10, EMissionReward::Gold, 80000);
	AddDefinition(TEXT("daily_boss_5"), EMissionPeriod::Daily, EMissionMetric::BossesKilled, 5, EMissionReward::Essence, 3);
	AddDefinition(TEXT("daily_gold_1m"), EMissionPeriod::Daily, EMissionMetric::GoldEarned, 1000000, EMissionReward::Consumable, 1);
	// 주간(Weekly) — target 대
	AddDefinition(TEXT("weekly_kill_5000"), EMissionPeriod::Weekly, EMissionMetric::MonstersKilled, 5000, EMissionReward::Gold, 500000);
	AddDefinition(TEXT("weekly_boss_50"), EMissionPeriod::Weekly, EMissionMetric::BossesKilled, 50, EMissionReward::Essence, 30);
	AddDefinition(TEXT("weekly_stage_150"), EMissionPeriod::Weekly, EMissionMetric::StagesCleared, 150, EMissionReward::Consumable, 3);
	AddDefinition(TEXT("weekly_dungeon_15"), EMissionPeriod::Weekly, EMissionMetric::DungeonRuns, 15, EMissionReward::Gold, 800000);
}

void UMissionService::RecordProgress(EMissionMetric Metric, int64 Delta)
{
	if (Delta <= 0)
	{
		return;
	}

	BuildDefaultDefinitions();
	for (const FMissionDefinition& Definition : Definitions)
	{
		if (Definition.Metric != Metric)
		{
			continue;
		}

		int64& Value = Progress.FindOrAdd(Definition.Id);
		// 오버플로 가드(누적형). target 을 넘는 진행도 그대로 보존(UI 클램프는 표시 측).
		Value = Value > MAX_int64 - Delta ? MAX_int64 : Value + Delta;
	}
}

bool UMissionService::IsComplete(const FString& Id) const
{
	const FMissionDefinition* Definition = DefinitionById.Find(Id);
	if (!Definition)
	{
		return false;
	}
	return Progress.FindRef(Id) >= Definition->Target;
}

bool UMissionService::ClaimMission(const FString& Id)
{
	if (!DefinitionById.Contains(Id) || Claimed.Contains(Id) || !IsComplete(Id))
	{
		return false;
	}

	Claimed.Add(Id);
	return true;
}

void UMissionService::EnsurePeriodFresh(const FString& Date, const FString& Week)
{
	BuildDefaultDefinitions();

	const FString ResetDate = Date.IsEmpty() ? UQuestService::GetCurrentUtcDateString() : Date;
	const FString ResetWeek = Week.IsEmpty() ? UQuestService::GetCurrentUtcWeekString() : Week;

	// 최초 호출(마커 미설정)이면 리셋 없이 마커만 초기화한다.
	if (DailyResetDate.IsEmpty())
	{
		DailyResetDate = ResetDate;
	}
	else if (DailyResetDate != ResetDate)
	{
		ResetPeriod(EMissionPeriod::Daily);
		DailyResetDate = ResetDate;
	}

	if (WeeklyResetWeek.IsEmpty())
	{
		WeeklyResetWeek = ResetWeek;
	}
	else if (WeeklyResetWeek != ResetWeek)
	{
		ResetPeriod(EMissionPeriod::Weekly);
		WeeklyResetWeek = ResetWeek;
	}
}

void UMissionService::ResetPeriod(EMissionPeriod Period)
{
	for (const FMissionDefinition& Definition : Definitions)
	{
		if (Definition.Period == Period)
		{
			Progress.Remove(Definition.Id);
			Claimed.Remove(Definition.Id);
		}
	}
}

TArray<FMissionProgressView> UMissionService::GetProgressViews() const
{
	TArray<FMissionProgressView> Views;
	Views.Reserve(Definitions.Num());
	for (const FMissionDefinition& Definition : Definitions)
	{
		FMissionProgressView View;
		View.Id = Definition.Id;
		View.Period = Definition.Period;
		View.Metric = Definition.Metric;
		View.Target = Definition.Target;
		View.Progress = Progress.FindRef(Definition.Id);
		View.RewardType = Definition.RewardType;
		View.RewardValue = Definition.RewardValue;
		View.bCompleted = View.Progress >= Definition.Target;
		View.bClaimed = Claimed.Contains(Definition.Id);
		Views.Add(View);
	}
	return Views;
}

bool UMissionService::GetDefinition(const FString& Id, FMissionDefinition& OutDefinition) const
{
	const FMissionDefinition* Definition = DefinitionById.Find(Id);
	if (!Definition)
	{
		return false;
	}
	OutDefinition = *Definition;
	return true;
}

void UMissionService::RestoreState(const TMap<FString, int64>& InProgress, const TSet<FString>& InClaimed, const FString& InDate, const FString& InWeek)
{
	BuildDefaultDefinitions();

	Progress.Reset();
	for (const TPair<FString, int64>& Pair : InProgress)
	{
		// 정의에 없는(구버전/잘못된) id 는 무시.
		if (DefinitionById.Contains(Pair.Key))
		{
			Progress.Add(Pair.Key, FMath::Max<int64>(0, Pair.Value));
		}
	}

	Claimed.Reset();
	for (const FString& Id : InClaimed)
	{
		if (DefinitionById.Contains(Id))
		{
			Claimed.Add(Id);
		}
	}

	DailyResetDate = InDate;
	WeeklyResetWeek = InWeek;
}
