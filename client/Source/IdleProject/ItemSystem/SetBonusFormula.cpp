#include "ItemSystem/SetBonusFormula.h"

namespace
{
void AddBonus(FDerivedStats& Target, const FDerivedStats& Source)
{
	Target.Hp += Source.Hp;
	Target.PhysAtk += Source.PhysAtk;
	Target.MagicAtk += Source.MagicAtk;
	Target.PhysDef += Source.PhysDef;
	Target.MagicDef += Source.MagicDef;
	Target.AtkSpeed += Source.AtkSpeed;
	Target.CritRate += Source.CritRate;
	Target.CritDmg += Source.CritDmg;
}

FDerivedStats GetTwoPieceBonus(EItemSet ItemSet)
{
	FDerivedStats Bonus;
	switch (ItemSet)
	{
	case EItemSet::Warrior:
		Bonus.PhysAtk = 20.0f;
		break;
	case EItemSet::Guardian:
		Bonus.PhysDef = 15.0f;
		Bonus.Hp = 100.0f;
		break;
	case EItemSet::Arcane:
		Bonus.MagicAtk = 20.0f;
		break;
	case EItemSet::None:
	default:
		break;
	}
	return Bonus;
}

FDerivedStats GetFourPieceBonus(EItemSet ItemSet)
{
	FDerivedStats Bonus;
	switch (ItemSet)
	{
	case EItemSet::Warrior:
		Bonus.PhysAtk = 50.0f;
		Bonus.CritRate = 0.05f;
		break;
	case EItemSet::Guardian:
		Bonus.PhysDef = 35.0f;
		Bonus.Hp = 250.0f;
		break;
	case EItemSet::Arcane:
		Bonus.MagicAtk = 50.0f;
		Bonus.CritDmg = 0.10f;
		break;
	case EItemSet::None:
	default:
		break;
	}
	return Bonus;
}
}

int32 FSetBonusFormula::GetSetPieceThreshold(int32 TierIndex)
{
	return TierIndex <= 0 ? 2 : 4;
}

FDerivedStats FSetBonusFormula::ComputeSetBonus(const TArray<FItemInstance>& EquippedItems)
{
	TMap<EItemSet, int32> PieceCounts;
	for (const FItemInstance& Item : EquippedItems)
	{
		if (Item.ItemSet == EItemSet::None || Item.Slot == EItemSlot::None || Item.Rarity == EItemRarity::None)
		{
			continue;
		}
		PieceCounts.FindOrAdd(Item.ItemSet) += 1;
	}

	FDerivedStats TotalBonus;
	for (const TPair<EItemSet, int32>& Pair : PieceCounts)
	{
		if (Pair.Value >= GetSetPieceThreshold(0))
		{
			AddBonus(TotalBonus, GetTwoPieceBonus(Pair.Key));
		}
		if (Pair.Value >= GetSetPieceThreshold(1))
		{
			AddBonus(TotalBonus, GetFourPieceBonus(Pair.Key));
		}
	}

	return TotalBonus;
}
