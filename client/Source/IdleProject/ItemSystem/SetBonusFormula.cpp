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

}

int32 FSetBonusFormula::GetSetPieceThreshold(int32 TierIndex)
{
	return TierIndex <= 0 ? 2 : 4;
}

FDerivedStats FSetBonusFormula::GetTwoPieceBonus(EItemSet ItemSet)
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
	case EItemSet::Assassin:
		Bonus.CritRate = 0.03f;
		break;
	case EItemSet::Hunter:
		Bonus.AtkSpeed = 0.05f;
		break;
	case EItemSet::Holy:
		Bonus.Hp = 120.0f;
		break;
	case EItemSet::Berserker:
		Bonus.PhysAtk = 30.0f;
		break;
	case EItemSet::None:
	default:
		break;
	}
	return Bonus;
}

FDerivedStats FSetBonusFormula::GetFourPieceBonus(EItemSet ItemSet)
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
	case EItemSet::Assassin:
		Bonus.CritDmg = 0.15f;
		break;
	case EItemSet::Hunter:
		Bonus.PhysAtk = 35.0f;
		Bonus.AtkSpeed = 0.03f;
		break;
	case EItemSet::Holy:
		Bonus.PhysDef = 20.0f;
		Bonus.MagicDef = 30.0f;
		break;
	case EItemSet::Berserker:
		Bonus.CritRate = 0.04f;
		Bonus.CritDmg = 0.10f;
		break;
	case EItemSet::None:
	default:
		break;
	}
	return Bonus;
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
			AddBonus(TotalBonus, FSetBonusFormula::GetTwoPieceBonus(Pair.Key));
		}
		if (Pair.Value >= GetSetPieceThreshold(1))
		{
			AddBonus(TotalBonus, FSetBonusFormula::GetFourPieceBonus(Pair.Key));
		}
	}

	return TotalBonus;
}
