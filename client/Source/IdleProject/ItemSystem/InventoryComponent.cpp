#include "ItemSystem/InventoryComponent.h"

#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/SetBonusFormula.h"
#include "ItemSystem/UniqueTraitFormula.h"

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
	if (!Current || (!Current->bLocked && FItemPowerScore::Compute(*Current) < FItemPowerScore::Compute(NewItem)))
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
		float PhysAtkMultiplier = 0.0f;
		float MagicAtkMultiplier = 0.0f;
		float HpMultiplier = 0.0f;
		float PhysDefMultiplier = 0.0f;
		float MagicDefMultiplier = 0.0f;
		float PotentialCritRate = 0.0f;
		float PotentialAtkSpeed = 0.0f;
		float PotentialCritDmg = 0.0f;
		// 잠재 V2: Transcendent 4번째 줄 포함 순회. 전투 8종 경로 불변(신규 옵션 3종은 여기 미합산 —
		// AllStat/Gold/Drop 은 각 단일 집계 지점에서만 소비, 이중 적용 금지 #72).
		for (const FPotentialLine& Line : { Item.PotentialLine1, Item.PotentialLine2, Item.PotentialLine3, Item.PotentialLine4 })
		{
			switch (Line.Stat)
			{
			case EPotentialStat::PhysAtkPercent:
				PhysAtkMultiplier += Line.Value;
				break;
			case EPotentialStat::MagicAtkPercent:
				MagicAtkMultiplier += Line.Value;
				break;
			case EPotentialStat::HpPercent:
				HpMultiplier += Line.Value;
				break;
			case EPotentialStat::PhysDefPercent:
				PhysDefMultiplier += Line.Value;
				break;
			case EPotentialStat::MagicDefPercent:
				MagicDefMultiplier += Line.Value;
				break;
			case EPotentialStat::CritRatePercent:
				PotentialCritRate += Line.Value;
				break;
			case EPotentialStat::AtkSpeedPercent:
				PotentialAtkSpeed += Line.Value;
				break;
			case EPotentialStat::CritDmgPercent:
				PotentialCritDmg += Line.Value;
				break;
			// 잠재 V2 신규 옵션은 전투 스탯 보너스 경로에서 제외(경제/전역 단일 집계 지점에서만 적용).
			case EPotentialStat::AllStatPercent:
			case EPotentialStat::GoldFindPercent:
			case EPotentialStat::DropRatePercent:
			case EPotentialStat::None:
			default:
				break;
			}
		}
		Bonus.PhysAtk += Item.BonusAtk * EnhanceMultiplier * (1.0f + PhysAtkMultiplier);
		Bonus.PhysDef += (Item.BonusDef + Item.BonusPhysDef) * EnhanceMultiplier * (1.0f + PhysDefMultiplier);
		Bonus.Hp += (Item.BonusHp + Item.BonusAffixHp) * EnhanceMultiplier * (1.0f + HpMultiplier);
		Bonus.CritRate += Item.BonusCritRate * EnhanceMultiplier + PotentialCritRate;
		Bonus.AtkSpeed += Item.BonusAtkSpeed * EnhanceMultiplier + PotentialAtkSpeed;
		Bonus.MagicAtk += Item.BonusMagicAtk * EnhanceMultiplier * (1.0f + MagicAtkMultiplier);
		Bonus.MagicDef += Item.BonusMagicDef * EnhanceMultiplier * (1.0f + MagicDefMultiplier);
		Bonus.CritDmg += Item.BonusCritDmg * EnhanceMultiplier + PotentialCritDmg;
		FUniqueTraitFormula::AccumulateTraitBonus(Item, Bonus);
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

FEquippedPotentialEconomyBonus UInventoryComponent::ComputeEquippedPotentialEconomyBonus() const
{
	// 잠재 V2: 장착 장비 잠재 신규 옵션 3종 합산. 4줄(Transcendent) 모두 순회.
	// 반환 구조의 각 필드는 호출측 단일 지점에서만 소비(AllStat→RefreshDerivedStats, Gold→AddGold, Drop→펫 Drop 집계).
	FEquippedPotentialEconomyBonus Result;
	for (const TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (!Items.IsValidIndex(Pair.Value))
		{
			continue;
		}

		const FItemInstance& Item = Items[Pair.Value];
		for (const FPotentialLine& Line : { Item.PotentialLine1, Item.PotentialLine2, Item.PotentialLine3, Item.PotentialLine4 })
		{
			switch (Line.Stat)
			{
			case EPotentialStat::AllStatPercent:
				Result.AllStatPercent += Line.Value;
				break;
			case EPotentialStat::GoldFindPercent:
				Result.GoldFindPercent += Line.Value;
				break;
			case EPotentialStat::DropRatePercent:
				Result.DropRatePercent += Line.Value;
				break;
			default:
				break;
			}
		}
	}
	return Result;
}

FUniqueTraitCoreMultipliers UInventoryComponent::ComputeUniqueTraitMultipliers() const
{
	FUniqueTraitCoreMultipliers Multipliers;
	for (const TPair<EItemSlot, int32>& Pair : EquippedIndex)
	{
		if (!Items.IsValidIndex(Pair.Value))
		{
			continue;
		}

		FUniqueTraitFormula::AccumulateTraitMultipliers(Items[Pair.Value], Multipliers);
	}
	return Multipliers;
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

bool UInventoryComponent::ApplyEnhanceOutcome(EItemSlot Slot, const FEnhanceAttemptOutcome& Outcome)
{
	const int32* Index = EquippedIndex.Find(Slot);
	if (!Index || !Items.IsValidIndex(*Index) || !Outcome.bAttempted)
	{
		return false;
	}

	FItemInstance& Item = Items[*Index];
	Item.EnhanceLevel = FMath::Clamp(Outcome.NewLevel, 0, FEnhanceFormula::MaxEnhanceLevel);
	Item.EnhanceFailStreak = FMath::Max(0, Outcome.NewFailStreak);
	OnEquippedChanged.Broadcast(Slot);
	return true;
}

bool UInventoryComponent::SetItemLocked(EItemSlot Slot, bool bLocked)
{
	const int32* Index = EquippedIndex.Find(Slot);
	if (!Index || !Items.IsValidIndex(*Index))
	{
		return false;
	}

	Items[*Index].bLocked = bLocked;
	OnEquippedChanged.Broadcast(Slot);
	return true;
}

bool UInventoryComponent::SetEquippedPotential(EItemSlot Slot, EPotentialGrade Grade, const TArray<FPotentialLine>& Lines)
{
	const int32* Index = EquippedIndex.Find(Slot);
	if (!Index || !Items.IsValidIndex(*Index))
	{
		return false;
	}

	FItemInstance& Item = Items[*Index];
	Item.PotentialGrade = Grade;
	Item.PotentialLine1 = Lines.IsValidIndex(0) ? Lines[0] : FPotentialLine();
	Item.PotentialLine2 = Lines.IsValidIndex(1) ? Lines[1] : FPotentialLine();
	Item.PotentialLine3 = Lines.IsValidIndex(2) ? Lines[2] : FPotentialLine();
	// 잠재 V2: Transcendent 4번째 줄(하위 등급 롤은 인덱스 3 없음 → 기본값으로 초기화/소거).
	Item.PotentialLine4 = Lines.IsValidIndex(3) ? Lines[3] : FPotentialLine();
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
	TMap<int32, int32> RestoredIndexBySavedIndex;
	for (int32 SavedIndex = 0; SavedIndex < InItems.Num(); ++SavedIndex)
	{
		if (Items.Num() >= MaxItems)
		{
			break;
		}
		const FItemInstance& SourceItem = InItems[SavedIndex];
		if (SourceItem.Slot == EItemSlot::None || SourceItem.Rarity == EItemRarity::None)
		{
			continue;
		}

		FItemInstance RestoredItem = SourceItem;
		RestoredItem.EnhanceLevel = FMath::Clamp(RestoredItem.EnhanceLevel, 0, FEnhanceFormula::MaxEnhanceLevel);
		RestoredItem.EnhanceFailStreak = FMath::Max(0, RestoredItem.EnhanceFailStreak);
		const int32 RestoredIndex = Items.Add(RestoredItem);
		RestoredIndexBySavedIndex.Add(SavedIndex, RestoredIndex);
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
		const int32* RemappedIndex = SavedIndex ? RestoredIndexBySavedIndex.Find(*SavedIndex) : nullptr;
		const int32 CandidateIndex = RemappedIndex ? *RemappedIndex : INDEX_NONE;
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
