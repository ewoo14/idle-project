#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CharacterSystem/StatFormulas.h"
#include "CharacterSystem/StatPointFormula.h"
#include "GameCore/AchievementService.h"
#include "GameCore/AttendanceService.h"
#include "GameCore/AutomationPolicyService.h"
#include "GameCore/ConsumableTypes.h"
#include "GameCore/DungeonService.h"
#include "GameCore/GuildTypes.h"
#include "GameCore/LeaderboardTypes.h"
#include "GameCore/MasteryService.h"
#include "GameCore/MissionService.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/RebirthPerkService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "GameCore/TitleService.h"
#include "GameCore/TowerService.h"
#include "GameCore/TreasureBoxService.h"
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
	UTitleService* GetTitleService() const { return TitleService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UMissionService* GetMissionService() const
	{
		// 지연 초기화: Init() 미경유 컨텍스트(테스트 등)에서도 null 을 반환하지 않도록 보장.
		const_cast<UIdleGameInstance*>(this)->EnsureMissionService();
		return MissionService;
	}

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	ULeaderboardService* GetLeaderboardService() const { return LeaderboardService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UBuffService* GetBuffService() const { return BuffService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UWeeklyBossService* GetWeeklyBossService() const { return WeeklyBossService; }

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UAttendanceService* GetAttendanceService() const
	{
		// 지연 초기화: Init() 미경유 컨텍스트(테스트 등)에서도 null 을 반환하지 않도록 보장.
		const_cast<UIdleGameInstance*>(this)->EnsureAttendanceService();
		return AttendanceService;
	}

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UTreasureBoxService* GetTreasureBoxService() const
	{
		// 지연 초기화: Init() 미경유 컨텍스트(테스트 등)에서도 null 을 반환하지 않도록 보장(#91).
		const_cast<UIdleGameInstance*>(this)->EnsureTreasureBoxService();
		return TreasureBoxService;
	}

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	URebirthPerkService* GetRebirthPerkService() const
	{
		// 지연 초기화: Init() 미경유 컨텍스트(테스트 등)에서도 null 을 반환하지 않도록 보장(#91).
		const_cast<UIdleGameInstance*>(this)->EnsureRebirthPerkService();
		return RebirthPerkService;
	}

	// 자동화 정책 서비스 접근자. 없으면 생성(lazy-ensure).
	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	UAutomationPolicyService* GetAutomationPolicyService() const
	{
		// 지연 초기화: Init() 미경유 컨텍스트(테스트 등)에서도 null 을 반환하지 않도록 보장.
		const_cast<UIdleGameInstance*>(this)->EnsureAutomationPolicyService();
		return AutomationPolicyService;
	}

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

	// 출석 마일스톤 보상 수령 단일 진입점. N<=GetReached && !Claimed 검증 → AttendanceService 마킹 후
	// 보상(골드/정수/소비) 지급 + 자동저장. 성공 시 true. 미달/중복/무효 N 은 false(보상 미지급).
	UFUNCTION(BlueprintCallable, Category = "Idle|Attendance")
	bool ClaimAttendanceMilestone(int32 N);

	// 주요 진행/로그인/세이브 시 호출. 오늘(UTC date) 첫 출석이면 누적++ 후 자동저장. 이미 출석이면 무동작.
	UFUNCTION(BlueprintCallable, Category = "Idle|Attendance")
	bool CheckInAttendance();

	// 보물 상자(일일 뽑기) 단일 진입점. 오늘(UTC date) CanDrawToday 검증 → DrawTreasure(RNG 멤버) →
	// 보상 단일 지급(Gold→AddGold / Essence→RuneEssence / Consumable→AddConsumable / ProtectionScroll/
	// ResetCube/RankCube→해당 재화 누적) + 자동저장. 이미 오늘 뽑았으면 빈 보상(Reward=None, 미지급).
	UFUNCTION(BlueprintCallable, Category = "Idle|TreasureBox")
	FTreasureReward DrawTreasureBox();

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

	// 환생 특성 분배(+1). RebirthPerkService 위임 후 성공 시 스탯 재계산·오토세이브.
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	bool AllocateRebirthPerk(ERebirthPerk Perk);

	// 환생 특성 분배 해제(-1). 성공 시 스탯 재계산·오토세이브.
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	bool DeallocateRebirthPerk(ERebirthPerk Perk);

	// 환생 특성 전체 초기화(가용 회수). 스탯 재계산·오토세이브.
	UFUNCTION(BlueprintCallable, Category = "Idle|RebirthPerk")
	void ResetRebirthPerks();

	// 환생 특성 perk 보너스 비율(없으면 0). RefreshDerivedStats/AddGold 등 단일 소비.
	float GetRebirthPerkBonus(ERebirthPerk Perk) const;

	// ── 자동화 진행 탭용 정책 변경 래퍼(BP 바인딩). 데이터 구동 HUD 진입점(신규 C++ 위젯 없음) ──
	// 변경 후 자동 저장 트리거는 기존 세이브 흐름 따름.
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationProgressionMode(EProgressionMode InMode);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationFarmLockStage(int32 InGlobalStage);

	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void SetAutomationAutoBossChallenge(bool bValue);

	UFUNCTION(BlueprintPure, Category = "Idle|Automation")
	bool IsAutomationFeatureUnlocked(EAutomationFeature Feature) const;

	// 스테이지 클리어로 자동 전진(+1)이 일어난 직후, 진행 정책에 따라 위치를 보정한다.
	// Advance: 무동작(이미 전진 완료). Hold(FarmLock/보스게이트): 목표로 점프. Retreat: 클리어 경로엔 없음.
	// ClearedGlobalStage = 전진 직전(클리어한) 글로벌 스테이지, bNextWasBoss = 전진해 들어간 스테이지가 보스였는지.
	UFUNCTION(BlueprintCallable, Category = "Idle|Automation")
	void ApplyProgressionPolicyAfterAdvance(int32 ClearedGlobalStage, bool bNextWasBoss);

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

	// 장착 칭호 AllStatPct 보너스를 곱 배수(1.0 + value)로 반환. 미장착/타 타입=1.0. 캐릭터 스탯 단일 적용 지점에서만 사용.
	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	float GetTitleAllStatMultiplier() const;

	// 장착 칭호 CritDmgPct 보너스를 가산값(비율)으로 반환. 미장착/타 타입=0.0. CritDmg 단일 가산 지점에서만 사용.
	UFUNCTION(BlueprintPure, Category = "Idle|Title")
	float GetTitleCritDamageBonus() const;

	// 환생 특성 AllStatPct 를 곱 배수(1.0 + 보너스)로 반환. RefreshDerivedStats 전역 배수 단일 지점에서만 소비(이중 적용 금지 #72).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	float GetRebirthPerkAllStatMultiplier() const;

	// 환생 특성 CritDmgPct 를 가산값(비율)으로 반환. CritDmg 단일 가산 지점에서만 소비(이중 적용 금지 #72).
	UFUNCTION(BlueprintPure, Category = "Idle|RebirthPerk")
	float GetRebirthPerkCritDamageBonus() const;

	// 잠재 V2: 장착 장비 잠재 AllStatPercent 합을 곱 배수(1.0 + 합)로 반환. RefreshDerivedStats 전역 배수 단일 지점에서만 소비(이중 적용 금지 #72).
	UFUNCTION(BlueprintPure, Category = "Idle|Item")
	float GetEquippedPotentialAllStatMultiplier() const;

	// 잠재 V2: 장착 장비 잠재 GoldFindPercent 합(비율). AddGold 골드 배수 단일 지점에서만 소비.
	UFUNCTION(BlueprintPure, Category = "Idle|Item")
	float GetEquippedPotentialGoldFindPercent() const;

	// 잠재 V2: 장착 장비 잠재 DropRatePercent 합(비율). 펫 Drop 보너스 집계(ApplyEquippedPetDropBonusChance) 단일 지점에서만 소비.
	UFUNCTION(BlueprintPure, Category = "Idle|Item")
	float GetEquippedPotentialDropRatePercent() const;

	// 칭호 장착/해제 진입점. 성공 시 캐릭터 스탯 갱신 + 자동 저장 요청.
	UFUNCTION(BlueprintCallable, Category = "Idle|Title")
	bool EquipTitle(const FString& TitleId);

	UFUNCTION(BlueprintCallable, Category = "Idle|Title")
	void UnequipTitle();

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

	// 미션 보상 수령 단일 진입점. 완료 && 미수령이면 MissionService 마킹 후 보상(골드/정수/소비) 지급 + 자동저장. 성공 시 true.
	UFUNCTION(BlueprintCallable, Category = "Idle|Mission")
	bool ClaimMission(const FString& Id);

	UFUNCTION(BlueprintPure, Category = "Idle|Quest")
	TArray<FQuestState> GetActiveQuestStates() const;

	bool GetQuestState(const FString& QuestId, FQuestState& OutState) const;

	void InitializeQuestServiceForTests(const FString& CurrentUtcDate);

	void InitializePetSeasonServicesForTests();

	void InitializeStageServiceForTests();

	void InitializeTowerServiceForTests();

	void InitializeDungeonServiceForTests(const FString& CurrentUtcDate);

	void InitializeWeeklyBossServiceForTests(const FString& CurrentWeek);

	// 출석 누적일을 직접 세팅(고임계 마일스톤 검증용). LastAttendanceDate/수령 집합은 보존하지 않고 초기화.
	void SeedAttendanceForTests(int64 Total);

	// 보물 상자 RNG 시드를 고정하여 결정적 뽑기를 검증한다(테스트 전용). 서비스도 초기화.
	void SeedTreasureBoxRngForTests(int32 Seed);

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
	TObjectPtr<UTitleService> TitleService;

	UPROPERTY(Transient)
	TObjectPtr<UMissionService> MissionService;

	UPROPERTY(Transient)
	TObjectPtr<ULeaderboardService> LeaderboardService;

	UPROPERTY(Transient)
	TObjectPtr<UBuffService> BuffService;

	UPROPERTY(Transient)
	TObjectPtr<UWeeklyBossService> WeeklyBossService;

	UPROPERTY(Transient)
	TObjectPtr<UAttendanceService> AttendanceService;

	UPROPERTY(Transient)
	TObjectPtr<UTreasureBoxService> TreasureBoxService;

	UPROPERTY(Transient)
	TObjectPtr<URebirthPerkService> RebirthPerkService;

	UPROPERTY(Transient)
	TObjectPtr<UAutomationPolicyService> AutomationPolicyService = nullptr;

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
	FRandomStream TreasureRandomStream;
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
	void EnsureTitleService();
	void EnsureMissionService();
	// 업적 메트릭 갱신/로그인/세이브 시 호출 — 달성한 칭호를 영구 해금한다(AchievementService 기준).
	void RecomputeUnlockedTitles();
	void EnsureLeaderboardService();
	void EnsureBuffService();
	void EnsureWeeklyBossService();
	void EnsureAttendanceService();
	void EnsureTreasureBoxService();
	void EnsureRebirthPerkService();
	void EnsureAutomationPolicyService();
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
