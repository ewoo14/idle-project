#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class UCombatComponent;
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

namespace IdleProject::UI
{
IDLEPROJECT_API TArray<FIdleHUDSkillSlotViewModel> BuildSkillSlotViewModels(const USkillComponent& SkillComponent, float Now);
IDLEPROJECT_API FIdleHUDUltimateViewModel BuildUltimateViewModel(const USkillComponent& SkillComponent);
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
	USkillComponent* ResolvePlayerSkills() const;
	void DrawSkillHud(const USkillComponent& SkillComponent, float Now);
	void DrawSkillSlot(const FIdleHUDSkillSlotViewModel& Slot, int32 SlotIndex, float X, float Y, float Width, float Height);
	void DrawUltimateGauge(const FIdleHUDUltimateViewModel& Ultimate, float X, float Y, float Width, float Height);

	UPROPERTY(Transient)
	TObjectPtr<UIdleGameInstance> IdleGameInstance;

	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> PlayerCombat;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> PlayerInventory;

	TSharedPtr<SIdleHUDWidget> RootWidget;
};
