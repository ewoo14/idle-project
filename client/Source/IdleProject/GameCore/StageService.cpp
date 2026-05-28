#include "GameCore/StageService.h"

#include "GameCore/StageFormula.h"

void UStageService::InitializeDefaultStages()
{
	CurrentChapter = 1;
	CurrentStage = 1;
	KillsThisStage = 0;
	bFinalChapterCleared = false;
	HighestClearedChapter = 0;
}

void UStageService::RecordKill(bool bWasBoss)
{
	if (bFinalChapterCleared)
	{
		return;
	}

	const bool bBossStage = FStageFormula::IsBossStage(CurrentChapter, CurrentStage, StagesPerChapter);
	if (bBossStage && !bWasBoss)
	{
		return;
	}

	KillsThisStage = FMath::Min(KillsThisStage + 1, GetKillsToAdvance());
	if (KillsThisStage >= GetKillsToAdvance())
	{
		AdvanceStage();
	}
}

void UStageService::AdvanceStage()
{
	if (CurrentStage < StagesPerChapter)
	{
		++CurrentStage;
		KillsThisStage = 0;
		OnStageChanged.Broadcast(GetCurrentStageInfo());
		return;
	}

	MarkCurrentChapterBossDefeated();
}

void UStageService::MarkCurrentChapterBossDefeated()
{
	if (!FStageFormula::IsBossStage(CurrentChapter, CurrentStage, StagesPerChapter)
		|| HasClearedChapterBoss(CurrentChapter))
	{
		return;
	}

	const int32 ClearedChapter = CurrentChapter;
	HighestClearedChapter = FMath::Max(HighestClearedChapter, ClearedChapter);
	OnChapterBossDefeated.Broadcast(ClearedChapter);

	if (ClearedChapter < TotalChapters)
	{
		CurrentChapter = ClearedChapter + 1;
		CurrentStage = 1;
		KillsThisStage = 0;
	}
	else
	{
		bFinalChapterCleared = true;
		KillsThisStage = GetKillsToAdvance();
	}

	OnStageChanged.Broadcast(GetCurrentStageInfo());
}

bool UStageService::HasClearedChapterBoss(int32 Chapter) const
{
	return Chapter > 0 && Chapter <= HighestClearedChapter;
}

int32 UStageService::GetKillsToAdvance() const
{
	switch (CurrentStage)
	{
	case 1:
		return 5;
	case 2:
		return 8;
	case 3:
		return 12;
	case 4:
		return 16;
	case 5:
		return 1;
	case 6:
		return 20;
	case 7:
		return 24;
	case 8:
		return 28;
	case 9:
		return 32;
	case 10:
		return 1;
	default:
		return 5;
	}
}

int32 UStageService::GetGlobalStageIndex() const
{
	return (FMath::Max(1, CurrentChapter) - 1) * StagesPerChapter + FMath::Max(1, CurrentStage);
}

FStageInfo UStageService::GetCurrentStageInfo() const
{
	FStageInfo Info;
	Info.Chapter = CurrentChapter;
	Info.Stage = CurrentStage;
	Info.GlobalStageIndex = GetGlobalStageIndex();
	Info.KillsThisStage = KillsThisStage;
	Info.KillsToAdvance = GetKillsToAdvance();
	Info.bBossStage = FStageFormula::IsBossStage(CurrentChapter, CurrentStage, StagesPerChapter);
	Info.bEliteStage = FStageFormula::IsEliteStage(CurrentStage);
	Info.WeakElement = FStageFormula::GetStageWeakElement(Info.GlobalStageIndex);
	return Info;
}

void UStageService::RestoreState(int32 Chapter, int32 Stage, int32 Kills, bool bFinalCleared, int32 HighestCleared)
{
	bFinalChapterCleared = bFinalCleared;
	if (bFinalChapterCleared)
	{
		CurrentChapter = TotalChapters;
		CurrentStage = StagesPerChapter;
		HighestClearedChapter = TotalChapters;
		KillsThisStage = GetKillsToAdvance();
		OnStageChanged.Broadcast(GetCurrentStageInfo());
		return;
	}

	CurrentChapter = FMath::Clamp(Chapter, 1, TotalChapters);
	CurrentStage = FMath::Clamp(Stage, 1, StagesPerChapter);
	HighestClearedChapter = FMath::Clamp(HighestCleared, 0, TotalChapters);
	KillsThisStage = FMath::Clamp(Kills, 0, GetKillsToAdvance());
	OnStageChanged.Broadcast(GetCurrentStageInfo());
}
