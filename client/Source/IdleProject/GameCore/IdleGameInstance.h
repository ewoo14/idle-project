#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "IdleGameInstance.generated.h"

class UApiClient;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int64, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, int64, CurrentExp, int64, NextExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);

/**
 * 게임 전역 서비스 컨테이너입니다.
 * PR #4에서는 API 클라이언트 생성과 기본 URL 보관만 담당합니다.
 */
UCLASS()
class IDLEPROJECT_API UIdleGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UApiClient* GetApiClient() const { return ApiClient; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UQuestService* GetQuestService() const { return QuestService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UPetService* GetPetService() const { return PetService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	USeasonService* GetSeasonService() const { return SeasonService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UStageService* GetStageService() const { return StageService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetApiBaseUrl() const { return ApiBaseUrl; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Settings")
	void SetLanguage(const FString& Language);

	UFUNCTION(BlueprintPure, Category = "Idle|Settings")
	const FString& GetLanguage() const { return Language; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void AddGold(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void AddExp(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void LevelUp();

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	bool CanRebirth() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Rebirth")
	bool Rebirth();

	UFUNCTION(BlueprintCallable, Category = "Idle|Rebirth")
	void MarkChapter1BossDefeated();

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	bool HasDefeatedChapter1Boss() const { return bChapter1BossDefeated; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Offline")
	FOfflineRewardResult ClaimOfflineRewards();

	FOfflineRewardResult ClaimOfflineRewardsAt(int64 NowUnixSec, int32 RebirthCountOverride = -1);

	UFUNCTION(BlueprintPure, Category = "Idle|Offline")
	FOfflineRewardResult PreviewOfflineRewards(int64 NowUnixSec, int32 RebirthCountOverride = -1) const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Offline")
	void SetLastSeenUnixSec(int64 UnixSec);

	UFUNCTION(BlueprintPure, Category = "Idle|Progression")
	int64 GetGold() const { return Gold; }

	UFUNCTION(BlueprintPure, Category = "Idle|Progression")
	int32 GetCharacterLevel() const { return CharacterLevel; }

	UFUNCTION(BlueprintPure, Category = "Idle|Progression")
	int64 GetCurrentExp() const { return CurrentExp; }

	UFUNCTION(BlueprintPure, Category = "Idle|Progression")
	int64 GetNextExp() const { return NextExp; }

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	int32 GetRebirthCount() const { return RebirthCount; }

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	int32 GetRebirthBonusPoints() const { return RebirthBonusPoints; }

	UFUNCTION(BlueprintPure, Category = "Idle|Offline")
	int64 GetLastSeenUnixSec() const { return LastSeenUnixSec; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordQuestProgress(EQuestObjective Objective, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordMonsterKilled();

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordGearEnhanced();

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	FQuestClaimResult ClaimQuest(const FString& QuestId);

	UFUNCTION(BlueprintPure, Category = "Idle|Quest")
	TArray<FQuestState> GetActiveQuestStates() const;

	bool GetQuestState(const FString& QuestId, FQuestState& OutState) const;

	void InitializeQuestServiceForTests(const FString& CurrentUtcDate);

	void InitializePetSeasonServicesForTests();

	void InitializeStageServiceForTests();

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool EquipPet(const FString& PetId);

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetGoldBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetDropBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	int64 ApplyEquippedPetGoldBonus(int64 BaseAmount) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float ApplyEquippedPetDropBonusChance(float BaseChance) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	int32 GetSeasonTokens() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Season")
	int32 GetReachedSeasonTier() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Season")
	FSeasonClaimResult ClaimSeasonReward(int32 Tier);

	UPROPERTY(BlueprintAssignable, Category = "Idle|Progression")
	FOnGoldChanged OnGoldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Progression")
	FOnExpChanged OnExpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Progression")
	FOnLevelUp OnLevelUp;

private:
	UPROPERTY(Transient)
	TObjectPtr<UApiClient> ApiClient;

	UPROPERTY(Transient)
	TObjectPtr<UQuestService> QuestService;

	UPROPERTY(Transient)
	TObjectPtr<UPetService> PetService;

	UPROPERTY(Transient)
	TObjectPtr<USeasonService> SeasonService;

	UPROPERTY(Transient)
	TObjectPtr<UStageService> StageService;

	/** 환경 변수 IDLE_API_BASE_URL이 없을 때 사용하는 로컬 기본 주소입니다. */
	UPROPERTY()
	FString ApiBaseUrl = TEXT("http://localhost:3000");

	UPROPERTY()
	FString Language = TEXT("ko");

	UPROPERTY()
	int64 Gold = 0;

	UPROPERTY()
	int32 CharacterLevel = 1;

	UPROPERTY()
	int64 CurrentExp = 0;

	UPROPERTY()
	int64 NextExp = 150;

	UPROPERTY()
	int32 RebirthCount = 0;

	UPROPERTY()
	int32 RebirthBonusPoints = 0;

	UPROPERTY()
	bool bChapter1BossDefeated = false;

	UPROPERTY()
	int64 LastSeenUnixSec = 0;

	static int64 GetCurrentUnixSeconds();
	void EnsureQuestService();
	void EnsurePetService();
	void EnsureSeasonService();
	void EnsureStageService();
	void LoadLanguage();
	void SaveLanguage() const;
	void LoadLastSeenUnixSec();
	void SaveLastSeenUnixSec() const;
};
