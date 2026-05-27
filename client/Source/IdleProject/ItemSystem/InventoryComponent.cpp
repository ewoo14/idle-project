#include "ItemSystem/InventoryComponent.h"

#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/SetBonusFormula.h"

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

bool UInventoryComponent::CanAddItem(const FItemInstance& NewItem) const
{
	if (Items.Num() >= MaxItems || NewItem.Slot == EItemSlot::None || NewItem.Rarity == EItemRarity::None)
	{
		return false;
	}

	return true;
}

bool UInventoryComponent::AddItem(const FItemInstance& NewItem)
{
	if (!CanAddItem(NewItem))
	{
		return false;
	}

	const int32 NewIndex = Items.Add(NewItem);
	const FItemInstance* Current = GetEquippedItem(NewItem.Slot);
	if (!Current || FItemPowerScore::Compute(*Current) < FItemPowerScore::Compute(NewItem))
	{
		EquipItem(NewIndex);
	}
	return true;
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

	// PR #9 policy: equipping swaps the slot pointer only; the previously equipped item
	// remains in Items. Inventory pruning / weakest-item discard is delegated to PR #11.
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
	// server/src/core/formulas/equipment.ts 의 computeInventoryBonus 와 1:1 미러.
	// 강화 보정: (1 + EnhanceLevel × 0.1) 을 각 Bonus 에 곱한다 (PR #1 §2.3 강화 곡선).
	FDerivedStats Bonus;
	TArray<FItemInstance> EquippedItems;
	for (const TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (!Items.IsValidIndex(Pair.Value))
		{
			continue;
		}

		const FItemInstance& Item = Items[Pair.Value];
		EquippedItems.Add(Item);
		const float EnhanceMultiplier = 1.0f + static_cast<float>(Item.EnhanceLevel) * 0.1f;
		Bonus.PhysAtk += Item.BonusAtk * EnhanceMultiplier;
		Bonus.PhysDef += Item.BonusDef * EnhanceMultiplier;
		Bonus.Hp += Item.BonusHp * EnhanceMultiplier;
		Bonus.CritRate += Item.BonusCritRate * EnhanceMultiplier;
		Bonus.AtkSpeed += Item.BonusAtkSpeed * EnhanceMultiplier;
		Bonus.MagicAtk += Item.BonusMagicAtk * EnhanceMultiplier;
	}

	const FDerivedStats SetBonus = FSetBonusFormula::ComputeSetBonus(EquippedItems);
	Bonus.Hp += SetBonus.Hp;
	Bonus.PhysAtk += SetBonus.PhysAtk;
	Bonus.MagicAtk += SetBonus.MagicAtk;
	Bonus.PhysDef += SetBonus.PhysDef;
	Bonus.MagicDef += SetBonus.MagicDef;
	Bonus.AtkSpeed += SetBonus.AtkSpeed;
	Bonus.CritRate += SetBonus.CritRate;
	Bonus.CritDmg += SetBonus.CritDmg;

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

bool UInventoryComponent::EnhanceEquippedItem(EItemSlot Slot)
{
	const int32* Index = EquippedIndex.Find(Slot);
	if (!Index || !Items.IsValidIndex(*Index))
	{
		return false;
	}

	FItemInstance& Item = Items[*Index];
	if (Item.EnhanceLevel >= FEnhanceFormula::MaxEnhanceLevel)
	{
		return false;
	}

	Item.EnhanceLevel = FMath::Clamp(Item.EnhanceLevel + 1, 0, FEnhanceFormula::MaxEnhanceLevel);
	OnEquippedChanged.Broadcast(Slot);
	return true;
}

int32 UInventoryComponent::GetEquippedEnhanceLevel(EItemSlot Slot) const
{
	const FItemInstance* Item = GetEquippedItem(Slot);
	return Item ? Item->EnhanceLevel : INDEX_NONE;
}

void UInventoryComponent::CaptureState(TArray<FItemInstance>& OutItems, TMap<EItemSlot, int32>& OutEquipped) const
{
	OutItems = Items;
	OutEquipped = EquippedIndex;
}

void UInventoryComponent::RestoreState(const TArray<FItemInstance>& InItems, const TMap<EItemSlot, int32>& InEquipped)
{
	Items.Reset();
	for (const FItemInstance& SourceItem : InItems)
	{
		if (Items.Num() >= MaxItems)
		{
			break;
		}
		if (SourceItem.Slot == EItemSlot::None || SourceItem.Rarity == EItemRarity::None)
		{
			continue;
		}

		FItemInstance RestoredItem = SourceItem;
		RestoredItem.EnhanceLevel = FMath::Clamp(RestoredItem.EnhanceLevel, 0, FEnhanceFormula::MaxEnhanceLevel);
		Items.Add(RestoredItem);
	}

	EquippedIndex.Empty();
	const EItemSlot Slots[] = {
		EItemSlot::Weapon,
		EItemSlot::Helmet,
		EItemSlot::Top,
		EItemSlot::Bottom,
		EItemSlot::Shoes,
		EItemSlot::Gloves,
		EItemSlot::Cloak,
		EItemSlot::Accessory
	};

	for (const EItemSlot Slot : Slots)
	{
		const int32* SavedIndex = InEquipped.Find(Slot);
		const int32 CandidateIndex = SavedIndex ? *SavedIndex : INDEX_NONE;
		if (Items.IsValidIndex(CandidateIndex) && Items[CandidateIndex].Slot == Slot)
		{
			EquippedIndex.Add(Slot, CandidateIndex);
		}
		else
		{
			EquippedIndex.Add(Slot, INDEX_NONE);
		}
		OnEquippedChanged.Broadcast(Slot);
	}
}
