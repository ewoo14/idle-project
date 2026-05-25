#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "Components/ActorComponent.h"
#include "ItemSystem/ItemTypes.h"
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

	void AddItem(const FItemInstance& NewItem);
	void EquipItem(int32 ItemIndex);
	void UnequipSlot(EItemSlot Slot);
	FDerivedStats ComputeEquipmentBonus() const;
	const FItemInstance* GetEquippedItem(EItemSlot Slot) const;

private:
	static constexpr int32 MaxItems = 100;

	UPROPERTY()
	TArray<FItemInstance> Items;

	UPROPERTY()
	TMap<EItemSlot, int32> EquippedIndex;
};
