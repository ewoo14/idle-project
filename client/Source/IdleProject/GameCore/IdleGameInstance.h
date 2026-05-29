#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CharacterSystem/StatFormulas.h"
#include "CharacterSystem/StatPointFormula.h"
#include "GameCore/AchievementService.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/DungeonService.h"
#include "GameCore/GuildTypes.h"
#include "GameCore/LeaderboardTypes.h"
#include "GameCore/MasteryService.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "GameCore/TowerService.h"
#include "GameCore/WeeklyBossTypes.h"
#include "ItemSystem/ItemTypes.h"
#include "RuneSystem/RuneTypes.h"
#include "TimerManager.h"
#include "IdleGameInstance.generated.h"

class UApiClient;
class UBuffService;
class UGuildService;
class UInventoryComponent;
class ULeaderboardService;
class AIdleCharacter;
class UIdleSaveGame;
class URuneService;
class UWeeklyBossService;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int64, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChanged, int64, CurrentExp, int64, NextExp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatPointsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTranscend);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProgressSaved);

UENUM(BlueprintType)
enum class ECloudSyncState : uint8
{
	Idle,
	Syncing,
	Synced,
	Offline,
	Conflict
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCloudSyncStateChanged, ECloudSyncState, NewState);

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FEnhanceAttemptResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	bool bAttempted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	bool bConsumedProtection = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	int64 GoldSpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	int32 NewLevel = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Enhance")
	int32 NewFailStreak = 0;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FPotentialRerollResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Potential")
	bool bRerolled = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Potential")
	EPotentialCubeType CubeType = EPotentialCubeType::Reset;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Potential")
	EPotentialGrade NewGrade = EPotentialGrade::None;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FShopPurchaseResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Shop")
	bool bPurchased = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Shop")
	int64 GoldSpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Shop")
	EItemRarity Rarity = EItemRarity::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Shop")
	EItemSlot Slot = EItemSlot::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Shop")
	FText ItemName;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FPetFeedResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Pet")
	bool bFed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Pet")
	int64 GoldSpent = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Pet")
	int32 NewLevel = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnhanceResult, const FEnhanceAttemptResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopPurchase, const FShopPurchaseResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPetFed, const FPetFeedResult&, Result);

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

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UTowerService* GetTowerService() const { return TowerService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UDungeonService* GetDungeonService() const { return DungeonService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	URuneService* GetRuneService() const { return RuneService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UAchievementService* GetAchievementService() const { return AchievementService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UMasteryService* GetMasteryService() const { return MasteryService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	ULeaderboardService* GetLeaderboardService() const { return LeaderboardService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UBuffService* GetBuffService() const { return BuffService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UWeeklyBossService* GetWeeklyBossService() const { return WeeklyBossService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UGuildService* GetGuildService() const { return GuildService; }

	/** 로그인/세이브 시 서버 `GET /v1/guilds/me` 스냅샷을 받아 GuildService 캐시에 반영. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Guild")
	void RefreshGuildSnapshot();

	/**
	 * 누적된 자동 기여 델타(던전 클리어·보스 도전)를 서버로 플러시(POST contribute).
	 * 소속 길드가 있고 pending>0 일 때만 호출되며, 성공 시 스냅샷을 재동기화한다.
	 * 세이브/재접속 시점에 호출(주간 상한은 GuildService/서버가 클램프).
	 */
	UFUNCTION(BlueprintCallable, Category = "Idle|Guild")
	void FlushPendingGuildContribution();

	// ── 길드 패널 UI 액션 배선(서버 권위 — 완료 시 스냅샷 재동기화) ───────────────
	/** 길드 목록 조회(무소속 화면). 파싱된 요약 배열을 콜백으로 전달(UI 캐시용). */
	void GuildPanelRefreshList(const FString& Query, TFunction<void(bool, const TArray<FGuildSummary>&)> OnComplete);
	/** 길드 생성 → 성공 시 스냅샷 갱신. */
	void GuildPanelCreate(const FString& Name, TFunction<void(bool)> OnComplete);
	/** 길드 가입(자유=즉시/승인=신청) → 성공 시 스냅샷 갱신. */
	void GuildPanelJoin(const FString& GuildId, TFunction<void(bool)> OnComplete);
	/** 길드 탈퇴(길드장은 서버 위임/해산 규칙) → 성공 시 스냅샷 갱신. */
	void GuildPanelLeave(const FString& GuildId, TFunction<void(bool)> OnComplete);
	/** 가입 신청 승인 → 성공 시 스냅샷 갱신. */
	void GuildPanelApprove(const FString& GuildId, const FString& TargetCharacterId, TFunction<void(bool)> OnComplete);
	/** 가입 신청 거절 → 성공 시 스냅샷 갱신. */
	void GuildPanelReject(const FString& GuildId, const FString& TargetCharacterId, TFunction<void(bool)> OnComplete);
	/** 멤버 계급 승강(길드장만) → 성공 시 스냅샷 갱신. */
	void GuildPanelSetRank(const FString& GuildId, const FString& TargetCharacterId, EGuildRank NewRank, TFunction<void(bool)> OnComplete);
	/** 길드 설정(가입 모드 토글 등, 길드장/부) → 성공 시 스냅샷 갱신. */
	void GuildPanelUpdateJoinMode(const FString& GuildId, EGuildJoinMode NewJoinMode, TFunction<void(bool)> OnComplete);
	/** 일일 출석(+기여) → 성공 시 스냅샷 갱신. */
	void GuildPanelAttendance(TFunction<void(bool)> OnComplete);
	/** 골드 헌납(floor(gold/1000) 기여, 일일 상한 서버 검증) → 성공 시 스냅샷 갱신. */
	void GuildPanelDonate(int64 DonateGold, TFunction<void(bool)> OnComplete);
	/** 길드 상점 카탈로그 조회(별도 fetch). 파싱된 아이템 배열을 콜백으로 전달(UI 캐시용). */
	void GuildPanelFetchShop(TFunction<void(bool, const TArray<FGuildShopItemInfo>&)> OnComplete);
	/** 길드 상점 구매(포인트 차감, 서버 검증) → 성공 시 스냅샷 갱신. */
	void GuildPanelBuyShopItem(const FString& ItemId, TFunction<void(bool)> OnComplete);
	/**
	 * 공유 HP 풀 길드 보스 도전(PR-G3). 현재 캐릭터 CP(GetCombatPower)를 서버로 전달 →
	 * 서버가 누적·격파 루프·데미지→기여(applyContribution)를 권위 처리 → 스냅샷 재동기화.
	 * 데미지→기여 적립은 서버만 수행한다(클라 이중 적립 금지).
	 */
	void GuildPanelChallengeBoss(TFunction<void(bool)> OnComplete);
	/**
	 * 보스 격파 보상 수령(PR-G3). 응답 rewards 배열(격파 N건 누적)을 각각
	 * ApplyGuildShopReward(type, amount)(G2 재사용)로 지급 → 세이브 → 스냅샷 갱신.
	 */
	void GuildPanelClaimBossReward(TFunction<void(bool)> OnComplete);
	/** 주간 길드 랭킹 조회(상위 N + 내 길드 순위). 파싱된 랭킹/내 순위를 콜백으로 전달. */
	void GuildPanelFetchRankings(int32 Limit, TFunction<void(bool, const TArray<FGuildRankingEntry>&, const FGuildRankingEntry&)> OnComplete);

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetApiBaseUrl() const { return ApiBaseUrl; }

	UFUNCTION(BlueprintPure, Category = "Idle|Time")
	static int64 GetCurrentUnixSeconds();

	UFUNCTION(BlueprintCallable, Category = "Idle|Settings")
	void SetLanguage(const FString& Language);

	UFUNCTION(BlueprintPure, Category = "Idle|Settings")
	const FString& GetLanguage() const { return Language; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void AddGold(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Save")
	void SaveProgress();

	UFUNCTION(BlueprintCallable, Category = "Idle|Save")
	void LoadProgress();

	UFUNCTION(BlueprintCallable, Category = "Idle|Cloud")
	void SyncFromCloud();

	UFUNCTION(BlueprintCallable, Category = "Idle|Cloud")
	void UploadToCloud();

	UFUNCTION(BlueprintCallable, Category = "Idle|Leaderboard")
	void RefreshLeaderboard(ELeaderboardKind Kind);

	UFUNCTION(BlueprintPure, Category = "Idle|Cloud")
	ECloudSyncState GetCloudSyncState() const { return CloudSyncState; }

	bool CaptureToSave(UIdleSaveGame* SaveGame);
	bool ApplyFromSave(const UIdleSaveGame* SaveGame);
	void ApplyPendingCharacterSaveToCharacter(AIdleCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Idle|Tower")
	int64 ClimbTower();

	UFUNCTION(BlueprintCallable, Category = "Idle|Dungeon")
	FDungeonRunResult TryRunDungeon(EDungeonType Type, int32 Tier = 1);

	UFUNCTION(BlueprintCallable, Category = "Idle|WeeklyBoss")
	FWeeklyBossChallengeResult TryChallengeWeeklyBoss();

	UFUNCTION(BlueprintCallable, Category = "Idle|WeeklyBoss")
	bool ClaimWeeklyBossMilestone(int32 Milestone);

	UFUNCTION(BlueprintCallable, Category = "Idle|Enhance")
	FEnhanceAttemptResult TryEnhanceEquipped(EItemSlot Slot, bool bUseProtection = false);

	FEnhanceAttemptResult TryEnhanceEquipped(EItemSlot Slot, UInventoryComponent* Inventory);
	FEnhanceAttemptResult TryEnhanceEquipped(EItemSlot Slot, bool bUseProtection, UInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Idle|Potential")
	FPotentialRerollResult TryRerollPotential(EItemSlot Slot, EPotentialCubeType CubeType);

	FPotentialRerollResult TryRerollPotential(EItemSlot Slot, EPotentialCubeType CubeType, UInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Idle|Inventory")
	bool SetItemLocked(EItemSlot Slot, bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Idle|Shop")
	FShopPurchaseResult TryBuyGearRoll();

	FShopPurchaseResult TryBuyGearRoll(UInventoryComponent* Inventory);

	UFUNCTION(BlueprintCallable, Category = "Idle|Shop")
	bool TryBuyProtectionScroll();

	UFUNCTION(BlueprintCallable, Category = "Idle|Shop")
	bool TryBuyResetCube();

	UFUNCTION(BlueprintCallable, Category = "Idle|Shop")
	bool TryBuyRankCube();

	UFUNCTION(BlueprintCallable, Category = "Idle|Consumable")
	void AddConsumable(EConsumableType Type, EConsumableGrade Grade = EConsumableGrade::Standard, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Idle|Consumable")
	bool TryUseConsumable(EConsumableType Type, EConsumableGrade Grade = EConsumableGrade::Standard);

	UFUNCTION(BlueprintCallable, Category = "Idle|Shop")
	bool TryBuyConsumable(EConsumableType Type, EConsumableGrade Grade = EConsumableGrade::Standard);

	/** 등급별 소비 상점 가격: Lesser=기어롤×0.6, Standard=×1.0, Greater=×2.5 (#38 곡선 기반). */
	UFUNCTION(BlueprintPure, Category = "Idle|Shop")
	int64 GetConsumableShopCost(EConsumableGrade Grade) const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	void AddRune(const FRuneInstance& Rune);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryEnhanceRune(int32 OwnedIndex);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryDisenchantRune(int32 OwnedIndex);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryBuyRuneRoll();

	/** 보유 룬 세트 리롤(룬 정수 비용). 성공 시 세트 균등 랜덤 재설정. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryRerollRuneSet(int32 OwnedIndex);

	/** 보유 룬 등급 상승 시도(룬 정수+골드 비용, 확률). 성공 여부는 bOutSucceeded. 반환=시도 성립(자원 차감). */
	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryUpgradeRuneRarity(int32 OwnedIndex, bool& bOutSucceeded);

	/** 강화 레벨 전송(룬 정수 비용, source 레벨 비례). Dst=max(Dst,Src), Src 삭제. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TransferRuneEnhancement(int32 SrcIndex, int32 DstIndex);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryCraftClassRune(EItemRarity Rarity = EItemRarity::Common);

	bool TryRollClassRuneDrop(int32 MonsterLevel, bool bIsBoss);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool TryEquipRune(int32 SlotIndex, int32 OwnedIndex);

	UFUNCTION(BlueprintCallable, Category = "Idle|Rune")
	bool UnequipRune(int32 SlotIndex);

	void SetEnhanceRandomSeed(int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void AddExp(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Idle|Progression")
	void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Idle|Stats")
	void GrantStatPoints(int32 Points);

	UFUNCTION(BlueprintCallable, Category = "Idle|Stats")
	bool AllocateStatPoint(EPrimaryStat Stat);

	UFUNCTION(BlueprintCallable, Category = "Idle|Stats")
	void ResetStatPoints();

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	FPrimaryStats GetAllocatedPrimaryStats() const { return AllocatedStats; }

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	int32 GetAvailableStatPoints() const { return AvailableStatPoints; }

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	bool CanRebirth() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Rebirth")
	bool Rebirth();

	UFUNCTION(BlueprintPure, Category = "Idle|Rebirth")
	int32 PreviewRebirthReward() const;

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

	UFUNCTION(BlueprintPure, Category = "Idle|Rune")
	int64 GetRuneEssence() const { return RuneEssence; }

	UFUNCTION(BlueprintPure, Category = "Idle|Potential")
	int64 GetProtectionScrolls() const { return ProtectionScrolls; }

	UFUNCTION(BlueprintPure, Category = "Idle|Potential")
	int64 GetResetCubes() const { return ResetCubes; }

	UFUNCTION(BlueprintPure, Category = "Idle|Potential")
	int64 GetRankCubes() const { return RankCubes; }

	/**
	 * 길드 상점 구매 보상(서버 `shopBuy` 응답 reward)을 캐릭터에 실제 지급한다.
	 * RewardType 은 서버 카탈로그 type 문자열(gold/expPotion/essence/protectionScroll/resetCube/rankCube).
	 * 알 수 없는 타입은 무시(로그)하며, Amount<=0 은 지급하지 않는다.
	 */
	void ApplyGuildShopReward(const FString& RewardType, int64 Amount);

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

	UFUNCTION(BlueprintPure, Category = "Idle|Transcend")
	bool CanTranscend() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Transcend")
	bool Transcend();

	UFUNCTION(BlueprintPure, Category = "Idle|Transcend")
	int32 GetTranscendCount() const { return TranscendCount; }

	UFUNCTION(BlueprintPure, Category = "Idle|Transcend")
	float GetTranscendStatMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Tower")
	float GetTowerMilestoneMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	float GetAchievementStatMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
	float GetMasteryCoreStatMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	int32 GetAchievementTotalPoints() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Achievement")
	int64 GetAchievementMetricValue(EAchievementMetric Metric) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Rune")
	float GetRuneGoldFindBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Rune")
	float GetRuneExpBoostBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Rune")
	float GetRuneOfflineEffBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Rune")
	float GetRuneCritDamageBonus() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Transcend")
	float PreviewTranscendMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Offline")
	int64 GetLastSeenUnixSec() const { return LastSeenUnixSec; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordQuestProgress(EQuestObjective Objective, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordMonsterKilled();

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	void RecordGearEnhanced();

	UFUNCTION(BlueprintCallable, Category = "Idle|Achievement")
	void RecordAchievementMetric(EAchievementMetric Metric, int64 AmountOrValue);

	void RecordAchievementItemCollected(const FItemInstance& Item);

	UFUNCTION(BlueprintCallable, Category = "Idle|Quest")
	FQuestClaimResult ClaimQuest(const FString& QuestId);

	UFUNCTION(BlueprintPure, Category = "Idle|Quest")
	TArray<FQuestState> GetActiveQuestStates() const;

	bool GetQuestState(const FString& QuestId, FQuestState& OutState) const;

	void InitializeQuestServiceForTests(const FString& CurrentUtcDate);

	void InitializePetSeasonServicesForTests();

	void InitializeStageServiceForTests();

	void InitializeTowerServiceForTests();

	void InitializeDungeonServiceForTests(const FString& CurrentUtcDate);

	void InitializeWeeklyBossServiceForTests(const FString& CurrentWeek);

	void InitializeGuildServiceForTests();

	void InitializeRuneServiceForTests();

#if WITH_DEV_AUTOMATION_TESTS
	void AddRuneForTests(const FRuneInstance& Rune);
	void AddRuneEssenceForTests(int64 Amount);
	void SetRuneOwnerClassForTests(EClassId ClassId);
#endif

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool EquipPet(const FString& PetId);

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool TryUnlockPet(const FString& PetId);

	// 보유 펫 진화 진입점 — 현재 별 기준 골드 비용 검증·단일 차감 후 별 1 증가.
	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	bool EvolvePet(const FString& PetId);

	UFUNCTION(BlueprintCallable, Category = "Idle|Pet")
	FPetFeedResult TryFeedPet(const FString& PetId);

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetGoldBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetDropBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	float GetEquippedPetExpBonusPercent() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Pet")
	FPetStatBonus GetEquippedPetStatBonus() const;

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

	UPROPERTY(BlueprintAssignable, Category = "Idle|Stats")
	FOnStatPointsChanged OnStatPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Transcend")
	FOnTranscend OnTranscend;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Enhance")
	FOnEnhanceResult OnEnhanceResult;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Shop")
	FOnShopPurchase OnShopPurchase;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Pet")
	FOnPetFed OnPetFed;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Save")
	FOnProgressSaved OnProgressSaved;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Cloud")
	FOnCloudSyncStateChanged OnCloudSyncStateChanged;

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

	UPROPERTY(Transient)
	TObjectPtr<UTowerService> TowerService;

	UPROPERTY(Transient)
	TObjectPtr<UDungeonService> DungeonService;

	UPROPERTY(Transient)
	TObjectPtr<URuneService> RuneService;

	UPROPERTY(Transient)
	TObjectPtr<UAchievementService> AchievementService;

	UPROPERTY(Transient)
	TObjectPtr<UMasteryService> MasteryService;

	UPROPERTY(Transient)
	TObjectPtr<ULeaderboardService> LeaderboardService;

	UPROPERTY(Transient)
	TObjectPtr<UBuffService> BuffService;

	UPROPERTY(Transient)
	TObjectPtr<UWeeklyBossService> WeeklyBossService;

	UPROPERTY(Transient)
	TObjectPtr<UGuildService> GuildService;

	/** 환경 변수 IDLE_API_BASE_URL이 없을 때 사용하는 로컬 기본 주소입니다. */
	UPROPERTY()
	FString ApiBaseUrl = TEXT("http://localhost:3000");

	UPROPERTY()
	FString Language = TEXT("ko");

	UPROPERTY()
	int64 Gold = 0;

	UPROPERTY()
	int64 RuneEssence = 0;

	UPROPERTY()
	int64 ProtectionScrolls = 0;

	UPROPERTY()
	int64 ResetCubes = 0;

	UPROPERTY()
	int64 RankCubes = 0;

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
	int32 TranscendCount = 0;

	UPROPERTY()
	int32 AvailableStatPoints = 0;

	UPROPERTY()
	FPrimaryStats AllocatedStats;

	UPROPERTY()
	bool bChapter1BossDefeated = false;

	UPROPERTY()
	int64 LastSeenUnixSec = 0;

	bool bAutosaveSuppressed = false;
	bool bAutosavePending = false;
	bool bCloudUploadSuppressed = false;
	bool bHasPendingCharacterSaveV2 = false;
	TArray<FItemInstance> PendingInventoryItems;
	TMap<EItemSlot, int32> PendingEquippedSlotIndex;
	TMap<FName, int32> PendingSkillRanks;
	int32 PendingSkillPoints = 0;
	FTimerHandle AutosaveTimerHandle;

	FRandomStream EnhanceRandomStream;
	FRandomStream PotentialRandomStream;
	FRandomStream RuneRandomStream;
	ECloudSyncState CloudSyncState = ECloudSyncState::Idle;

	static const TCHAR* SaveSlotName;
	static constexpr float AutosaveDebounceSeconds = 1.0f;
	static constexpr int32 CloudSaveApiVersion = 4;
	// 길드 자동 기여 누적량(소량, 주간 상한은 GuildService/서버가 클램프).
	static constexpr int64 GuildAutoContributionPerDungeon = 5;
	static constexpr int64 GuildAutoContributionPerBoss = 10;
	UInventoryComponent* FindPlayerInventory() const;
	AIdleCharacter* FindPlayerCharacter() const;
	EClassId GetCurrentClassIdForRunes() const;
	bool ApplyCharacterSaveState(
		AIdleCharacter* Character,
		const TArray<FItemInstance>& InventoryItems,
		const TMap<EItemSlot, int32>& EquippedSlotIndex,
		const TMap<FName, int32>& SkillRanks,
		int32 SkillPoints);
	void RequestAutosave();
	void FlushPendingAutosave();
	void SetCloudSyncState(ECloudSyncState NewState);
	void EnsureQuestService();
	void EnsurePetService();
	void EnsureSeasonService();
	void EnsureStageService();
	void EnsureTowerService();
	void EnsureDungeonService();
	void EnsureRuneService();
	void EnsureAchievementService();
	void EnsureMasteryService();
	void EnsureLeaderboardService();
	void EnsureBuffService();
	void EnsureWeeklyBossService();
	void EnsureGuildService();
	void RefreshPlayerCharacterStats();
	bool TryBuyShopResource(int64 Cost, int64& ResourceCount);
	int64 ApplyEquipmentCubeCostReduction(int64 BaseCost);
	UFUNCTION()
	void HandleChapterBossDefeated(int32 ClearedChapter);
	void LoadLanguage();
	void SaveLanguage() const;
	void LoadLastSeenUnixSec();
	void SaveLastSeenUnixSec() const;
};
