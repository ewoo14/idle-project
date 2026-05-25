#include "ItemSystem/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	EquippedIndex.Add(EItemSlot::Weapon, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Helmet, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Top, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Bottom, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Shoes, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Gloves, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Cloak, INDEX_NONE);
	EquippedIndex.Add(EItemSlot::Accessory, INDEX_NONE);
}

void UInventoryComponent::AddItem(const FItemInstance& NewItem)
{
	if (Items.Num() >= MaxItems || NewItem.Slot == EItemSlot::None || NewItem.Rarity == EItemRarity::None)
	{
		return;
	}

	const int32 NewIndex = Items.Add(NewItem);
	const FItemInstance* Current = GetEquippedItem(NewItem.Slot);
	if (!Current || FItemPowerScore::Compute(*Current) < FItemPowerScore::Compute(NewItem))
	{
		EquipItem(NewIndex);
	}
}

void UInventoryComponent::EquipItem(int32 ItemIndex)
{
	if (!Items.IsValidIndex(ItemIndex))
	{
		return;
	}

	const EItemSlot Slot = Items[ItemIndex].Slot;
	if (Slot == EItemSlot::None)
	{
		return;
	}

	EquippedIndex.FindOrAdd(Slot) = ItemIndex;
	OnEquippedChanged.Broadcast(Slot);
}

void UInventoryComponent::UnequipSlot(EItemSlot Slot)
{
	if (Slot == EItemSlot::None)
	{
		return;
	}

	int32& Index = EquippedIndex.FindOrAdd(Slot);
	if (Index != INDEX_NONE)
	{
		Index = INDEX_NONE;
		OnEquippedChanged.Broadcast(Slot);
	}
}

FDerivedStats UInventoryComponent::ComputeEquipmentBonus() const
{
	FDerivedStats Bonus;
	for (const TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (!Items.IsValidIndex(Pair.Value))
		{
			continue;
		}

		const FItemInstance& Item = Items[Pair.Value];
		Bonus.PhysAtk += Item.BonusAtk;
		Bonus.PhysDef += Item.BonusDef;
		Bonus.Hp += Item.BonusHp;
	}
	return Bonus;
}

const FItemInstance* UInventoryComponent::GetEquippedItem(EItemSlot Slot) const
{
	const int32* Index = EquippedIndex.Find(Slot);
	if (!Index || !Items.IsValidIndex(*Index))
	{
		return nullptr;
	}
	return &Items[*Index];
}
