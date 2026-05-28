#include "ItemSystem/UniqueTraitFormula.h"

namespace
{
float GetUniqueBaseValue(EUniqueTrait Trait)
{
	switch (Trait)
	{
	case EUniqueTrait::AllStatSurge:
		return 0.08f;
	case EUniqueTrait::CritDamageSurge:
		return 0.15f;
	case EUniqueTrait::CritRateSurge:
		return 0.05f;
	case EUniqueTrait::LifeSurge:
		return 0.10f;
	case EUniqueTrait::SwiftSurge:
		return 0.08f;
	case EUniqueTrait::PhysMastery:
	case EUniqueTrait::MagicMastery:
		return 0.12f;
	case EUniqueTrait::GuardMastery:
		return 0.10f;
	case EUniqueTrait::None:
	default:
		return 0.0f;
	}
}

EUniqueTrait RollTrait(FRandomStream& Rng)
{
	return static_cast<EUniqueTrait>(Rng.RandRange(
		static_cast<int32>(EUniqueTrait::AllStatSurge),
		static_cast<int32>(EUniqueTrait::GuardMastery)));
}

void AddMultiplier(float Value, float& OutMultiplier)
{
	OutMultiplier = (1.0f + OutMultiplier) * (1.0f + Value) - 1.0f;
}

void AccumulateSingleTrait(EUniqueTrait Trait, EItemRarity Rarity, FDerivedStats& OutBonus)
{
	const float Value = FUniqueTraitFormula::GetTraitValue(Trait, Rarity);
	if (Value <= 0.0f)
	{
		return;
	}

	switch (Trait)
	{
	case EUniqueTrait::CritDamageSurge:
		OutBonus.CritDmg += Value;
		break;
	case EUniqueTrait::CritRateSurge:
		OutBonus.CritRate += Value;
		break;
	case EUniqueTrait::SwiftSurge:
		OutBonus.AtkSpeed += Value;
		break;
	case EUniqueTrait::AllStatSurge:
	case EUniqueTrait::LifeSurge:
	case EUniqueTrait::PhysMastery:
	case EUniqueTrait::MagicMastery:
	case EUniqueTrait::GuardMastery:
		break;
	case EUniqueTrait::None:
	default:
		break;
	}
}

void AccumulateSingleTraitMultiplier(EUniqueTrait Trait, EItemRarity Rarity, FUniqueTraitCoreMultipliers& OutMultipliers)
{
	const float Value = FUniqueTraitFormula::GetTraitValue(Trait, Rarity);
	if (Value <= 0.0f)
	{
		return;
	}

	switch (Trait)
	{
	case EUniqueTrait::AllStatSurge:
		AddMultiplier(Value, OutMultipliers.PhysAtk);
		AddMultiplier(Value, OutMultipliers.MagicAtk);
		AddMultiplier(Value, OutMultipliers.PhysDef);
		AddMultiplier(Value, OutMultipliers.MagicDef);
		break;
	case EUniqueTrait::LifeSurge:
		AddMultiplier(Value, OutMultipliers.Hp);
		break;
	case EUniqueTrait::PhysMastery:
		AddMultiplier(Value, OutMultipliers.PhysAtk);
		break;
	case EUniqueTrait::MagicMastery:
		AddMultiplier(Value, OutMultipliers.MagicAtk);
		break;
	case EUniqueTrait::GuardMastery:
		AddMultiplier(Value, OutMultipliers.PhysDef);
		AddMultiplier(Value, OutMultipliers.MagicDef);
		break;
	case EUniqueTrait::CritDamageSurge:
	case EUniqueTrait::CritRateSurge:
	case EUniqueTrait::SwiftSurge:
	case EUniqueTrait::None:
	default:
		break;
	}
}
}

float FUniqueTraitFormula::GetTraitValue(EUniqueTrait Trait, EItemRarity Rarity)
{
	const float BaseValue = GetUniqueBaseValue(Trait);
	if (BaseValue <= 0.0f)
	{
		return 0.0f;
	}
	if (Rarity == EItemRarity::Unique)
	{
		return BaseValue;
	}
	if (Rarity == EItemRarity::Transcendent)
	{
		return BaseValue * 1.5f;
	}
	return 0.0f;
}

bool FUniqueTraitFormula::RarityGrantsUnique(EItemRarity Rarity)
{
	return Rarity == EItemRarity::Unique || Rarity == EItemRarity::Transcendent;
}

bool FUniqueTraitFormula::RarityGrantsTwoTraits(EItemRarity Rarity)
{
	return Rarity == EItemRarity::Transcendent;
}

void FUniqueTraitFormula::RollUniqueTraits(EItemRarity Rarity, FRandomStream& Rng, EUniqueTrait& OutTrait1, EUniqueTrait& OutTrait2)
{
	OutTrait1 = EUniqueTrait::None;
	OutTrait2 = EUniqueTrait::None;

	if (!RarityGrantsUnique(Rarity))
	{
		return;
	}

	OutTrait1 = RollTrait(Rng);
	if (!RarityGrantsTwoTraits(Rarity))
	{
		return;
	}

	do
	{
		OutTrait2 = RollTrait(Rng);
	}
	while (OutTrait2 == OutTrait1);
}

void FUniqueTraitFormula::AccumulateTraitBonus(const FItemInstance& Item, FDerivedStats& OutBonus)
{
	AccumulateSingleTrait(Item.UniqueTrait1, Item.Rarity, OutBonus);
	AccumulateSingleTrait(Item.UniqueTrait2, Item.Rarity, OutBonus);
}

void FUniqueTraitFormula::AccumulateTraitMultipliers(const FItemInstance& Item, FUniqueTraitCoreMultipliers& OutMultipliers)
{
	AccumulateSingleTraitMultiplier(Item.UniqueTrait1, Item.Rarity, OutMultipliers);
	AccumulateSingleTraitMultiplier(Item.UniqueTrait2, Item.Rarity, OutMultipliers);
}

void FUniqueTraitFormula::ApplyCoreMultipliers(FDerivedStats& InOutStats, const FUniqueTraitCoreMultipliers& Multipliers)
{
	InOutStats.Hp *= 1.0f + Multipliers.Hp;
	InOutStats.PhysAtk *= 1.0f + Multipliers.PhysAtk;
	InOutStats.MagicAtk *= 1.0f + Multipliers.MagicAtk;
	InOutStats.PhysDef *= 1.0f + Multipliers.PhysDef;
	InOutStats.MagicDef *= 1.0f + Multipliers.MagicDef;
}
