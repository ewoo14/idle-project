#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "Components/ActorComponent.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/ItemTypes.h"
#include "ItemSystem/UniqueTraitFormula.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedChanged, EItemSlot, Slot);

UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, Category = "Idle|Inventory")
	FOnEquippedChanged OnEquippedChanged;

	bool AddItem(const FItemInstance& NewItem);
	bool CanAddItem(const FItemInstance& NewItem) const;
	int32 GetItemCount() const { return Items.Num(); }
	void EquipItem(int32 ItemIndex);
	void UnequipSlot(EItemSlot Slot);
	FDerivedStats ComputeEquipmentBonus() const;
	FUniqueTraitCoreMultipliers ComputeUniqueTraitMultipliers() const;
	const FItemInstance* GetEquippedItem(EItemSlot Slot) const;
	bool EnhanceEquippedItem(EItemSlot Slot);
	bool ApplyEnhanceOutcome(EItemSlot Slot, const FEnhanceAttemptOutcome& Outcome);
	bool SetItemLocked(EItemSlot Slot, bool bLocked);
	bool SetEquippedPotential(EItemSlot Slot, EPotentialGrade Grade, const TArray<FPotentialLine>& Lines);
	int32 GetEquippedEnhanceLevel(EItemSlot Slot) const;
	void CaptureState(TArray<FItemInstance>& OutItems, TMap<EItemSlot, int32>& OutEquipped) const;
	void RestoreState(const TArray<FItemInstance>& InItems, const TMap<EItemSlot, int32>& InEquipped);

private:
	static constexpr int32 MaxItems = 100;

	UPROPERTY()
	TArray<FItemInstance> Items;

	UPROPERTY()
	TMap<EItemSlot, int32> EquippedIndex;
};
