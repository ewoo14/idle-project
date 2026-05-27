#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/CombatComponent.h"
#include "GameFramework/HUD.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class AIdleCharacter;
class AIdleMonster;
class UBattleAIComponent;
class UInventoryComponent;
class USkillComponent;
class SIdleHUDWidget;

struct IDLEPROJECT_API FIdleHUDSkillSlotViewModel
{
	FName SkillId;
	FText DisplayName;
	float CooldownRatio = 0.0f;
	float CooldownRemaining = 0.0f;
	int32 AvailableSkillPoints = 0;
	int32 Rank = 0;
	int32 MaxRank = 0;
	bool bReady = true;
	bool bCanRankUp = false;
};

struct IDLEPROJECT_API FIdleHUDUltimateViewModel
{
	float GaugeRatio = 0.0f;
	float GaugePercent = 0.0f;
	bool bReady = false;
};

struct IDLEPROJECT_API FIdleHUDOfflineRewardViewModel
{
	bool bVisible = false;
	FText Title;
	FText ElapsedLabel;
	FText GoldLabel;
	FText ExpLabel;
	FText ClaimLabel;
};

struct IDLEPROJECT_API FIdleHUDQuestLogRowViewModel
{
	FString QuestId;
	FText TypeLabel;
	FText Title;
	FText ProgressLabel;
	FText RewardLabel;
	FText ActionLabel;
	float ProgressRatio = 0.0f;
	bool bCanClaim = false;
	bool bClaimed = false;
};

struct IDLEPROJECT_API FIdleHUDQuestLogViewModel
{
	FText Title;
	FText ShortcutLabel;
	FText EmptyLabel;
	TArray<FIdleHUDQuestLogRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDRebirthViewModel
{
	FText Title;
	FText StatusLabel;
	FText BossLabel;
	FText LevelLabel;
	FText CountLabel;
	FText BonusLabel;
	FText NextBonusLabel;
	FText ButtonLabel;
	bool bCanRebirth = false;
	bool bBossDefeated = false;
	bool bLevelReady = false;
};

struct IDLEPROJECT_API FIdleHUDTranscendViewModel
{
	FText Title;
	FText StatusLabel;
	FText RequirementLabel;
	FText CurrentMultiplierLabel;
	FText PreviewMultiplierLabel;
	FText CountLabel;
	FText ButtonLabel;
	bool bCanTranscend = false;
	bool bThresholdReady = false;
};

struct IDLEPROJECT_API FIdleHUDClassSelectionOptionViewModel
{
	EClassId ClassId = EClassId::Warrior;
	FText DisplayName;
	FText RoleLabel;
	FText StatSummary;
	bool bSelected = false;
};

struct IDLEPROJECT_API FIdleHUDPetRowViewModel
{
	FString PetId;
	FText Name;
	FText BonusLabel;
	FText LevelLabel;
	FText FeedCostLabel;
	FText ActionLabel;
	FText FeedActionLabel;
	FText StatusLabel;
	bool bEquipped = false;
	bool bCanFeed = false;
	bool bFeedDisabled = true;
	bool bMaxLevel = false;
};

struct IDLEPROJECT_API FIdleHUDPetPanelViewModel
{
	FText Title;
	FText EquippedLabel;
	FText GoldBonusLabel;
	FText DropBonusLabel;
	TArray<FIdleHUDPetRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDSeasonTierRowViewModel
{
	int32 Tier = 0;
	FText TierLabel;
	FText RequirementLabel;
	FText RewardLabel;
	FText ActionLabel;
	float ProgressRatio = 0.0f;
	bool bReached = false;
	bool bClaimed = false;
	bool bCanClaim = false;
};

struct IDLEPROJECT_API FIdleHUDSeasonPassViewModel
{
	FText Title;
	FText TokenLabel;
	FText ProgressLabel;
	float ProgressRatio = 0.0f;
	TArray<FIdleHUDSeasonTierRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDFloatingDamageEntry
{
	FVector WorldLocation = FVector::ZeroVector;
	float Amount = 0.0f;
	bool bWasCrit = false;
	EDamageKind Kind = EDamageKind::Physical;
	float StartTime = 0.0f;
};

struct IDLEPROJECT_API FIdleHUDFloatingDamageViewModel
{
	bool bVisible = false;
	FString Label;
	FVector2D ScreenPosition = FVector2D::ZeroVector;
	FLinearColor Color = FLinearColor::White;
	float TextScale = 1.0f;
};

struct IDLEPROJECT_API FIdleHUDStatusIndicatorViewModel
{
	ESkillStatusEffect Type = ESkillStatusEffect::None;
	FString Label;
	FLinearColor Color = FLinearColor::White;
	float RemainingSeconds = 0.0f;
	float Size = 0.0f;
};

struct IDLEPROJECT_API FIdleHUDStageViewModel
{
	FText TitleLabel;
	FText ChapterLabel;
	FText ProgressLabel;
	FText BossBadgeLabel;
	FText WeaknessLabel;
	FLinearColor BorderColor = FLinearColor::White;
	FLinearColor WeaknessColor = FLinearColor::White;
	float ProgressRatio = 0.0f;
	bool bBossStage = false;
};

struct IDLEPROJECT_API FIdleHUDBossViewModel
{
	bool bVisible = false;
	FText TitleLabel;
	FText HpLabel;
	FText PhaseLabel;
	FLinearColor PhaseColor = FLinearColor::White;
	float HpRatio = 0.0f;
	float HpPercent = 0.0f;
	int32 Phase = 1;
};

struct IDLEPROJECT_API FIdleHUDEnhanceSlotViewModel
{
	EItemSlot Slot = EItemSlot::None;
	FText SlotLabel;
	FText ItemName;
	FText RarityLabel;
	FLinearColor RarityColor = FLinearColor::White;
	FText LevelLabel;
	FText CostLabel;
	FText SuccessRateLabel;
	FText StatusLabel;
	FText ButtonLabel;
	int32 EnhanceLevel = INDEX_NONE;
	int64 Cost = 0;
	float SuccessRate = 0.0f;
	bool bEquipped = false;
	bool bCanEnhance = false;
	bool bMaxLevel = false;
	bool bGoldEnough = false;
};

struct IDLEPROJECT_API FIdleHUDEnhancePanelViewModel
{
	FText Title;
	FText GoldLabel;
	FText FeedbackLabel;
	bool bFeedbackSuccess = false;
	TArray<FIdleHUDEnhanceSlotViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDShopPanelViewModel
{
	FText Title;
	FText GoldLabel;
	FText CostLabel;
	FText ButtonLabel;
	FText StatusLabel;
	FText LastResultLabel;
	FName GearRollHitBoxName;
	EItemRarity LastResultRarity = EItemRarity::None;
	int64 GearRollCost = 0;
	int64 Gold = 0;
	bool bCanBuyGearRoll = false;
	bool bHasLastResult = false;
	bool bLastResultError = false;
};

struct IDLEPROJECT_API FIdleHUDStatRowViewModel
{
	EPrimaryStat Stat = EPrimaryStat::Str;
	FText StatLabel;
	FText ValueLabel;
	FName AllocationHitBoxName;
	int32 BaseValue = 0;
	int32 AllocatedValue = 0;
	int32 TotalValue = 0;
	bool bCanAllocate = false;
};

struct IDLEPROJECT_API FIdleHUDStatPanelViewModel
{
	FText Title;
	FText AvailableLabel;
	FText ResetLabel;
	FName ResetHitBoxName;
	int32 AvailablePoints = 0;
	bool bCanReset = false;
	TArray<FIdleHUDStatRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDStatInfoRowViewModel
{
	FText StatLabel;
	FText ValueLabel;
};

struct IDLEPROJECT_API FIdleHUDStatInfoViewModel
{
	FText Title;
	FText HeaderLabel;
	FText CombatPowerLabel;
	FText ToggleLabel;
	FName ToggleHitBoxName;
	TArray<FIdleHUDStatInfoRowViewModel> PrimaryRows;
	TArray<FIdleHUDStatInfoRowViewModel> DerivedRows;
};

struct IDLEPROJECT_API FIdleHUDSetSummaryRowViewModel
{
	EItemSet ItemSet = EItemSet::None;
	FText SetLabel;
	FText TierLabel;
	FText SummaryLabel;
	FText BonusLabel;
	FText NextTierLabel;
	int32 PieceCount = 0;
	bool bTwoPieceActive = false;
	bool bFourPieceActive = false;
};

struct IDLEPROJECT_API FIdleHUDSetSummaryViewModel
{
	TArray<FIdleHUDSetSummaryRowViewModel> Rows;
};

struct IDLEPROJECT_API FIdleHUDTowerViewModel
{
	FText Title;
	FText HighestFloorLabel;
	FText NextRequiredPowerLabel;
	FText CombatPowerLabel;
	FText MilestoneMultiplierLabel;
	FText NextMilestoneLabel;
	FText StatusLabel;
	FText ButtonLabel;
	FName ClimbHitBoxName;
	int32 HighestFloor = 0;
	int32 NextMilestoneFloor = 0;
	int64 NextRequiredPower = 0;
	int64 CombatPower = 0;
	float MilestoneMultiplier = 1.0f;
	bool bCanClimb = false;
};

namespace IdleProject::UI
{
IDLEPROJECT_API FText RarityToLabel(EItemRarity Rarity);
IDLEPROJECT_API FLinearColor RarityToColor(EItemRarity Rarity);
IDLEPROJECT_API FText BuildAffixSummary(const FItemInstance& Item);
IDLEPROJECT_API FIdleHUDSetSummaryViewModel BuildSetSummaryViewModel(const TArray<FItemInstance>& EquippedItems);
IDLEPROJECT_API TArray<FIdleHUDSkillSlotViewModel> BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now);
IDLEPROJECT_API FIdleHUDUltimateViewModel BuildUltimateViewModel(const USkillComponent& SkillComponent);
IDLEPROJECT_API FIdleHUDStageViewModel BuildStageViewModel(const FStageInfo& StageInfo);
IDLEPROJECT_API FText BuildChapterEntryFeedbackLabel(int32 Chapter);
IDLEPROJECT_API FText BuildChapterClearFeedbackLabel(int32 Chapter);
IDLEPROJECT_API FIdleHUDOfflineRewardViewModel BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward);
IDLEPROJECT_API FIdleHUDQuestLogViewModel BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates);
IDLEPROJECT_API FIdleHUDRebirthViewModel BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints, int32 PreviewRebirthReward);
IDLEPROJECT_API FIdleHUDTranscendViewModel BuildTranscendViewModel(bool bCanTranscend, int32 RebirthCount, int32 Threshold, int32 TranscendCount, float CurrentMultiplier, float PreviewMultiplier);
IDLEPROJECT_API TArray<FIdleHUDClassSelectionOptionViewModel> BuildClassSelectionOptions(EClassId CurrentClassId);
IDLEPROJECT_API FIdleHUDPetPanelViewModel BuildPetPanelViewModel(const TArray<FPetDefinition>& PetDefinitions, const FString& EquippedPetId, float GoldBonusPercent, float DropBonusPercent, int64 Gold, TFunctionRef<int32(const FString&)> GetPetLevel);
IDLEPROJECT_API FIdleHUDSeasonPassViewModel BuildSeasonPassViewModel(const TArray<FSeasonTierDefinition>& Tiers, int32 SeasonTokens, int32 ReachedTier, TFunctionRef<bool(int32)> IsTierClaimed);
IDLEPROJECT_API FIdleHUDBossViewModel BuildBossViewModel(float CurrentHp, float MaxHp);
IDLEPROJECT_API FIdleHUDFloatingDamageViewModel BuildFloatingDamageViewModel(const FIdleHUDFloatingDamageEntry& Entry, float Now, FVector2D ProjectedScreenPosition, float HudScale);
IDLEPROJECT_API TArray<FIdleHUDStatusIndicatorViewModel> BuildStatusIndicatorViewModels(const TArray<FActiveSkillStatus>& Statuses, float Now, float HudScale);
IDLEPROJECT_API FIdleHUDEnhancePanelViewModel BuildEnhancePanelViewModel(const UInventoryComponent& Inventory, int64 Gold, FText FeedbackLabel, bool bFeedbackSuccess);
IDLEPROJECT_API FIdleHUDShopPanelViewModel BuildShopPanelViewModel(int64 GearRollCost, int64 Gold, const FShopPurchaseResult& LastResult);
IDLEPROJECT_API FIdleHUDStatPanelViewModel BuildStatPanelViewModel(const FPrimaryStats& BaseStats, const FPrimaryStats& AllocatedStats, int32 AvailablePoints);
IDLEPROJECT_API FIdleHUDStatInfoViewModel BuildStatInfoViewModel(const FPrimaryStats& PrimaryStats, const FDerivedStats& DerivedStats, int32 Level, EClassId ClassId, int32 RebirthCount, int64 CombatPower);
IDLEPROJECT_API FIdleHUDTowerViewModel BuildTowerViewModel(int32 HighestFloor, int64 NextRequiredPower, int64 CombatPower, float MilestoneMultiplier = -1.0f);
IDLEPROJECT_API FText BuildTowerClimbFeedbackLabel(int32 NewHighestFloor, int64 TotalReward);
IDLEPROJECT_API FText BuildProgressSavedFeedbackLabel();
}

/** Slate HUD 구현을 붙이기 위한 최소 AHUD 베이스입니다. */
UCLASS()
class IDLEPROJECT_API AIdleHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override;
	virtual void NotifyHitBoxClick(FName BoxName) override;

	void ToggleQuestLog();

protected:
	UFUNCTION()
	void HandleGoldChanged(int64 NewGold);

	UFUNCTION()
	void HandleExpChanged(int64 CurrentExp, int64 NextExp);

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

	UFUNCTION()
	void HandleHpChanged(float NewHp);

	void HandleDamageReceived(AActor* DamagedActor, float Amount, bool bWasCrit, EDamageKind Kind);

	UFUNCTION()
	void HandleEquippedChanged(EItemSlot Slot);

	UFUNCTION()
	void HandleEnhanceResult(const FEnhanceAttemptResult& Result);

	UFUNCTION()
	void HandleShopPurchase(const FShopPurchaseResult& Result);

	UFUNCTION()
	void HandlePetFed(const FPetFeedResult& Result);

	UFUNCTION()
	void HandleStatPointsChanged();

	UFUNCTION()
	void HandleTranscend();

	UFUNCTION()
	void HandleBossSpecialAttack(AActor* Boss);

	UFUNCTION()
	void HandleStageChanged(FStageInfo NewStageInfo);

	UFUNCTION()
	void HandleChapterBossDefeated(int32 ClearedChapter);

	UFUNCTION()
	void HandleTowerClimbed(int32 NewHighestFloor, int64 TotalReward);

	UFUNCTION()
	void HandleProgressSaved();

private:
	void BindStageService();
	void UnbindStageService();
	void BindTowerService();
	void UnbindTowerService();
	void BindPlayerCombat();
	void UnbindPlayerCombat();
	void BindPlayerInventory();
	void BindBossSpecialAttack(AIdleMonster* Boss);
	void UnbindBossSpecialAttack();
	void RefreshEquipmentSummary();
	AIdleCharacter* ResolvePlayerCharacter() const;
	AIdleMonster* ResolveVisibleBoss() const;
	USkillComponent* ResolvePlayerSkills() const;
	void DrawClassSelectionPanel();
	void DrawClassSelectionOption(const FIdleHUDClassSelectionOptionViewModel& Option, float X, float Y, float Width, float Height);
	void SelectClassFromHitBox(FName BoxName);
	void DrawSkillHud(const USkillComponent& SkillComponent, float Now);
	void DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height);
	void DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height);
	void DrawStageIndicator();
	void DrawBossBar();
	void DrawBossSpecialWarning(float Now);
	void DrawShopPanel();
	void TryBuyGearRoll();
	void DrawEnhancePanel();
	void DrawEnhanceSlotRow(const FIdleHUDEnhanceSlotViewModel& Row, float X, float Y, float Width, float Height);
	void TryEnhanceFromHitBox(FName BoxName);
	void DrawStatAllocationPanel();
	void DrawStatAllocationRow(const FIdleHUDStatRowViewModel& Row, float X, float Y, float Width, float Height);
	void AllocateStatFromHitBox(FName BoxName);
	void ResetStatAllocation();
	void DrawStatInfoPanel();
	void DrawStatInfoToggle(const FIdleHUDStatInfoViewModel& ViewModel, float X, float Y, float Scale);
	void DrawStatInfoRow(const FIdleHUDStatInfoRowViewModel& Row, float X, float Y, float Width, float Height, const FLinearColor& AccentColor);
	void ToggleStatInfoPanel();
	void RankUpSkillFromHitBox(FName BoxName);
	void PreviewOfflineRewardModal();
	void ClaimOfflineRewardModal();
	void DrawOfflineRewardModal();
	void DrawQuestLog();
	void ClaimQuestFromHitBox(FName BoxName);
	void DrawRebirthPanel();
	void TryRebirth();
	void DrawTranscendPanel();
	void TryTranscend();
	void DrawTowerPanel();
	void TryClimbTower();
	void DrawPetPanel();
	void DrawPetRow(const FIdleHUDPetRowViewModel& Row, float X, float Y, float Width, float Height);
	void EquipPetFromHitBox(FName BoxName);
	void TryFeedPetFromHitBox(FName BoxName);
	void DrawSeasonPassPanel();
	void DrawSeasonTierRow(const FIdleHUDSeasonTierRowViewModel& Row, float X, float Y, float Width, float Height);
	void ClaimSeasonTierFromHitBox(FName BoxName);
	void DrawFloatingDamageTexts(float Now);
	void DrawStatusIndicators(float Now);
	void DrawProgressSavedIndicator(float Now);
	void RefreshMouseInteraction();

	UPROPERTY(Transient)
	TObjectPtr<UIdleGameInstance> IdleGameInstance;

	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> PlayerCombat;

	UPROPERTY(Transient)
	TObjectPtr<APawn> PlayerCombatPawn;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> PlayerInventory;

	TSharedPtr<SIdleHUDWidget> RootWidget;
	FIdleHUDOfflineRewardViewModel OfflineRewardModal;
	FText EnhanceFeedbackLabel;
	FText PetFeedbackLabel;
	FText TranscendFeedbackLabel;
	bool bEnhanceFeedbackSuccess = false;
	bool bPetFeedbackSuccess = false;
	float PetFeedbackStartTime = -1000.0f;
	float TranscendFeedbackStartTime = -1000.0f;
	FShopPurchaseResult LastShopPurchaseResult;
	TArray<FIdleHUDFloatingDamageEntry> FloatingDamageEntries;
	FDelegateHandle AnyDamageReceivedHandle;
	TWeakObjectPtr<UBattleAIComponent> BoundBossBattleAI;
	TWeakObjectPtr<AActor> BossSpecialAttackActor;
	TWeakObjectPtr<UStageService> BoundStageService;
	TWeakObjectPtr<UTowerService> BoundTowerService;
	FText StageFeedbackLabel;
	FText TowerFeedbackLabel;
	FText ProgressSavedFeedbackLabel;
	float BossSpecialAttackStartTime = -1000.0f;
	float StageFeedbackStartTime = -1000.0f;
	float TowerFeedbackStartTime = -1000.0f;
	float ProgressSavedFeedbackStartTime = -1000.0f;
	bool bQuestLogVisible = false;
	bool bStatInfoVisible = false;
};
