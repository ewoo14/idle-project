#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "GameFramework/HUD.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/QuestService.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class UCombatComponent;
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
	bool bReady = true;
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

namespace IdleProject::UI
{
IDLEPROJECT_API TArray<FIdleHUDSkillSlotViewModel> BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now);
IDLEPROJECT_API FIdleHUDUltimateViewModel BuildUltimateViewModel(const USkillComponent& SkillComponent);
IDLEPROJECT_API FIdleHUDOfflineRewardViewModel BuildOfflineRewardViewModel(const FOfflineRewardResult& Reward);
IDLEPROJECT_API FIdleHUDQuestLogViewModel BuildQuestLogViewModel(const TArray<FQuestState>& QuestStates);
IDLEPROJECT_API FIdleHUDRebirthViewModel BuildRebirthViewModel(bool bCanRebirth, bool bBossDefeated, int32 CharacterLevel, int32 RebirthCount, int32 RebirthBonusPoints);
IDLEPROJECT_API TArray<FIdleHUDClassSelectionOptionViewModel> BuildClassSelectionOptions(EClassId CurrentClassId);
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

	UFUNCTION()
	void HandleEquippedChanged(EItemSlot Slot);

private:
	void BindPlayerCombat();
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
	void PreviewOfflineRewardModal();
	void ClaimOfflineRewardModal();
	void DrawOfflineRewardModal();
	void DrawQuestLog();
	void ClaimQuestFromHitBox(FName BoxName);
	void DrawRebirthPanel();
	void TryRebirth();
	void RefreshMouseInteraction();

	UPROPERTY(Transient)
	TObjectPtr<UIdleGameInstance> IdleGameInstance;

	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> PlayerCombat;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> PlayerInventory;

	TSharedPtr<SIdleHUDWidget> RootWidget;
	FIdleHUDOfflineRewardViewModel OfflineRewardModal;
	bool bQuestLogVisible = false;
};
