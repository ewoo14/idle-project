#pragma once

#include "CoreMinimal.h"
#include "CombatSystem/StatusElementTypes.h"
#include "UObject/Object.h"
#include "StageService.generated.h"

USTRUCT(BlueprintType)
struct FStageInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	int32 Chapter = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	int32 Stage = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	int32 GlobalStageIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	int32 KillsThisStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	int32 KillsToAdvance = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	bool bBossStage = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	bool bEliteStage = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Stage")
	ESkillElement WeakElement = ESkillElement::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageChanged, FStageInfo, NewStageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapterBossDefeated, int32, ClearedChapter);

UCLASS(BlueprintType)
class IDLEPROJECT_API UStageService : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 StagesPerChapter = 10;
	static constexpr int32 TotalChapters = 7;

	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void InitializeDefaultStages();

	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void RecordKill(bool bWasBoss);

	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void AdvanceStage();

	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void MarkCurrentChapterBossDefeated();

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetCurrentChapter() const { return CurrentChapter; }

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetKillsThisStage() const { return KillsThisStage; }

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetKillsToAdvance() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetGlobalStageIndex() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	FStageInfo GetCurrentStageInfo() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	bool HasClearedCurrentChapterBoss() const { return HasClearedChapterBoss(CurrentChapter); }

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	bool HasClearedChapterBoss(int32 Chapter) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	int32 GetHighestClearedChapter() const { return HighestClearedChapter; }

	UFUNCTION(BlueprintPure, Category = "Idle|Stage")
	bool HasFinalChapterCleared() const { return bFinalChapterCleared; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Stage")
	void RestoreState(int32 Chapter, int32 Stage, int32 Kills, bool bFinalCleared, int32 HighestCleared);

	UPROPERTY(BlueprintAssignable, Category = "Idle|Stage")
	FOnStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Stage")
	FOnChapterBossDefeated OnChapterBossDefeated;

private:
	UPROPERTY()
	int32 CurrentChapter = 1;

	UPROPERTY()
	int32 CurrentStage = 1;

	UPROPERTY()
	int32 KillsThisStage = 0;

	UPROPERTY()
	bool bFinalChapterCleared = false;

	UPROPERTY()
	int32 HighestClearedChapter = 0;
};
