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

void AccumulateSingleTrait(EUniqueTrait Trait, EItemRarity Rarity, FDerivedStats& OutBonus)
{
	const float Value = FUniqueTraitFormula::GetTraitValue(Trait, Rarity);
	if (Value <= 0.0f)
	{
		return;
	}

	switch (Trait)
	{
	case EUniqueTrait::AllStatSurge:
		OutBonus.PhysAtk += Value;
		OutBonus.MagicAtk += Value;
		OutBonus.PhysDef += Value;
		OutBonus.MagicDef += Value;
		break;
	case EUniqueTrait::CritDamageSurge:
		OutBonus.CritDmg += Value;
		break;
	case EUniqueTrait::CritRateSurge:
		OutBonus.CritRate += Value;
		break;
	case EUniqueTrait::LifeSurge:
		OutBonus.Hp += Value;
		break;
	case EUniqueTrait::SwiftSurge:
		OutBonus.AtkSpeed += Value;
		break;
	case EUniqueTrait::PhysMastery:
		OutBonus.PhysAtk += Value;
		break;
	case EUniqueTrait::MagicMastery:
		OutBonus.MagicAtk += Value;
		break;
	case EUniqueTrait::GuardMastery:
		OutBonus.PhysDef += Value;
		OutBonus.MagicDef += Value;
		break;
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
