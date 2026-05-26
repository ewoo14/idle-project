#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/CombatComponent.h"
#include "GameFramework/HUD.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/SeasonService.h"
#include "GameCore/StageService.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class AIdleCharacter;
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
	FText ActionLabel;
	bool bEquipped = false;
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
	FText ProgressLabel;
	FText BossBadgeLabel;
	FText WeaknessLabel;
	FLinearColor BorderColor = FLinearColor::White;
	FLinearColor WeaknessColor = FLinearColor::White;
	float ProgressRatio = 0.0f;
	bool bBossStage = false;
};

namespace IdleProject::UI
{
IDLEPROJECT_API TArray<FIdleHUDSkillSlotViewModel> BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now);
IDLEPROJECT_API FIdleHUDUltimateViewModel BuildUltimateViewModel(const USkillComponent& SkillComponent);
IDLEPROJECT_API FIdleHUDStageViewModel BuildStageViewModel(const FStageInfo& StageInfo);
IDLEPROJECT_API FIdleHUDOfflineRewardViewModel BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward);
IDLEPROJECT_API FIdleHUDQuestLogViewModel BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates);
IDLEPROJECT_API FIdleHUDRebirthViewModel BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints);
IDLEPROJECT_API TArray<FIdleHUDClassSelectionOptionViewModel> BuildClassSelectionOptions(EClassId CurrentClassId);
IDLEPROJECT_API FIdleHUDPetPanelViewModel BuildPetPanelViewModel(const TArray<FPetDefinition>& PetDefinitions, const FString& EquippedPetId, float GoldBonusPercent, float DropBonusPercent);
IDLEPROJECT_API FIdleHUDSeasonPassViewModel BuildSeasonPassViewModel(const TArray<FSeasonTierDefinition>& Tiers, int32 SeasonTokens, int32 ReachedTier, TFunctionRef<bool(int32)> IsTierClaimed);
IDLEPROJECT_API FIdleHUDFloatingDamageViewModel BuildFloatingDamageViewModel(const FIdleHUDFloatingDamageEntry& Entry, float Now, FVector2D ProjectedScreenPosition, float HudScale);
IDLEPROJECT_API TArray<FIdleHUDStatusIndicatorViewModel> BuildStatusIndicatorViewModels(const TArray<FActiveSkillStatus>& Statuses, float Now, float HudScale);
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

private:
	void BindPlayerCombat();
	void UnbindPlayerCombat();
	void BindPlayerInventory();
	void RefreshEquipmentSummary();
	AIdleCharacter* ResolvePlayerCharacter() const;
	USkillComponent* ResolvePlayerSkills() const;
	void DrawClassSelectionPanel();
	void DrawClassSelectionOption(const FIdleHUDClassSelectionOptionViewModel& Option, float X, float Y, float Width, float Height);
	void SelectClassFromHitBox(FName BoxName);
	void DrawSkillHud(const USkillComponent& SkillComponent, float Now);
	void DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height);
	void DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height);
	void DrawStageIndicator();
	void RankUpSkillFromHitBox(FName BoxName);
	void PreviewOfflineRewardModal();
	void ClaimOfflineRewardModal();
	void DrawOfflineRewardModal();
	void DrawQuestLog();
	void ClaimQuestFromHitBox(FName BoxName);
	void DrawRebirthPanel();
	void TryRebirth();
	void DrawPetPanel();
	void DrawPetRow(const FIdleHUDPetRowViewModel& Row, float X, float Y, float Width, float Height);
	void EquipPetFromHitBox(FName BoxName);
	void DrawSeasonPassPanel();
	void DrawSeasonTierRow(const FIdleHUDSeasonTierRowViewModel& Row, float X, float Y, float Width, float Height);
	void ClaimSeasonTierFromHitBox(FName BoxName);
	void DrawFloatingDamageTexts(float Now);
	void DrawStatusIndicators(float Now);
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
	TArray<FIdleHUDFloatingDamageEntry> FloatingDamageEntries;
	FDelegateHandle AnyDamageReceivedHandle;
	bool bQuestLogVisible = false;
};
