#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestService.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
	Main,
	Daily
};

UENUM(BlueprintType)
enum class EQuestObjective : uint8
{
	KillMonster,
	ClearMap,
	ClaimOffline,
	Enhance
};

USTRUCT(BlueprintType)
struct FQuestDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	FString QuestId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	EQuestType Type = EQuestType::Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	EQuestObjective Objective = EQuestObjective::KillMonster;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	int32 TargetCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardExp = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	FString PrerequisiteQuestId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Quest")
	FString ChapterMapId;
};

USTRUCT(BlueprintType)
struct FQuestState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FString QuestId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	EQuestType Type = EQuestType::Main;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FText Title;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	EQuestObjective Objective = EQuestObjective::KillMonster;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int32 TargetCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardGold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardExp = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FString PrerequisiteQuestId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FString ChapterMapId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int32 Progress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	bool bCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	bool bClaimed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FString DailyResetDate;
};

USTRUCT(BlueprintType)
struct FQuestSaveEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FString QuestId;

	UPROPERTY()
	EQuestType Type = EQuestType::Main;

	UPROPERTY()
	int32 Progress = 0;

	UPROPERTY()
	bool bCompleted = false;

	UPROPERTY()
	bool bClaimed = false;

	UPROPERTY()
	FString DailyResetDate;
};

USTRUCT(BlueprintType)
struct FQuestClaimResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	FQuestState Quest;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardGold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	int64 RewardExp = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Quest")
	TArray<FString> UnlockedQuestIds;
};

UCLASS(BlueprintType)
class IDLEPROJECT_API UQuestService : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void InitializeDefaultQuests(const FString& CurrentUtcDate);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void ResetDailyQuestsIfNeeded(const FString& CurrentUtcDate);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordProgress(EQuestObjective Objective, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	FQuestClaimResult ClaimQuest(const FString& QuestId);

	UFUNCTION(BlueprintPure, Category = "Idle|Quest")
	TArray<FQuestState> GetActiveQuestStates() const;

	const TArray<FQuestDefinition>& GetQuestDefinitions() const;

	bool GetQuestState(const FString& QuestId, FQuestState& OutState) const;

	void CaptureState(TArray<FQuestSaveEntry>& OutEntries, FString& OutDailyReset) const;
	void RestoreState(const TArray<FQuestSaveEntry>& InEntries, const FString& InDailyReset);

	static FString GetCurrentUtcDateString();

private:
	TArray<FQuestDefinition> Definitions;
	TMap<FString, FQuestDefinition> DefinitionById;
	TMap<FString, FQuestState> ActiveStates;
	FString DailyResetDate;

	void BuildDefaultDefinitions();
	void AddActiveQuest(const FQuestDefinition& Definition, const FString& CurrentDailyResetDate);
	bool IsQuestActive(const FString& QuestId) const;
};
