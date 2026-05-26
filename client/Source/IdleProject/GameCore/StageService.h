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
	ESkillElement WeakElement = ESkillElement::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageChanged, FStageInfo, NewStageInfo);

UCLASS(BlueprintType)
class IDLEPROJECT_API UStageService : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 StagesPerChapter = 5;

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
	bool HasClearedCurrentChapterBoss() const { return bCurrentChapterBossCleared; }

	UPROPERTY(BlueprintAssignable, Category = "Idle|Stage")
	FOnStageChanged OnStageChanged;

private:
	UPROPERTY()
	int32 CurrentChapter = 1;

	UPROPERTY()
	int32 CurrentStage = 1;

	UPROPERTY()
	int32 KillsThisStage = 0;

	UPROPERTY()
	bool bCurrentChapterBossCleared = false;
};
