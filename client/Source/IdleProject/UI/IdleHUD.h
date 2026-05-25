#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleHUD.generated.h"

class UIdleGameInstance;
class UCombatComponent;
class UInventoryComponent;
class SIdleHUDWidget;

/** Slate HUD 구현을 붙이기 위한 최소 AHUD 베이스입니다. */
UCLASS()
class IDLEPROJECT_API AIdleHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(Transient)
	TObjectPtr<UIdleGameInstance> IdleGameInstance;

	UPROPERTY(Transient)
	TObjectPtr<UCombatComponent> PlayerCombat;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> PlayerInventory;

	TSharedPtr<SIdleHUDWidget> RootWidget;
};
