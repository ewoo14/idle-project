#include "ItemSystem/DropFormula.h"

namespace
{
enum class EAffixKind : uint8
{
	CritRate,
	AtkSpeed,
	MagicAtk
};

int32 GetAffixCount(EItemRarity Rarity, FRandomStream& Rng)
{
	switch (Rarity)
	{
	case EItemRarity::Uncommon:
	case EItemRarity::Rare:
		return 1;
	case EItemRarity::Epic:
		return 2;
	case EItemRarity::Legendary:
		return Rng.GetFraction() < 0.5f ? 2 : 3;
	case EItemRarity::None:
	case EItemRarity::Common:
	default:
		return 0;
	}
}

void ShuffleAffixKinds(TArray<EAffixKind>& Kinds, FRandomStream& Rng)
{
	for (int32 Index = Kinds.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Rng.RandRange(0, Index);
		Kinds.Swap(Index, SwapIndex);
	}
}
}

float FDropFormula::GetRarityStatMultiplier(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return 1.0f;
	case EItemRarity::Uncommon:
		return 1.3f;
	case EItemRarity::Rare:
		return 1.7f;
	case EItemRarity::Epic:
		return 2.3f;
	case EItemRarity::Legendary:
		return 3.2f;
	case EItemRarity::None:
	default:
		return 0.0f;
	}
}

EItemRarity FDropFormula::RollRarityForLevel(int32 Level, FRandomStream& Rng)
{
	const int32 SafeLevel = FMath::Max(Level, 1);
	const float LevelScale = FMath::Clamp((static_cast<float>(SafeLevel) - 1.0f) / 99.0f, 0.0f, 1.0f);

	const float NoneChance = 0.02f;
	const float UncommonChance = 0.20f;
	const float RareChance = 0.08f + 0.12f * LevelScale;
	const float EpicChance = 0.06f * LevelScale;
	const float LegendaryChance = 0.02f * LevelScale;
	const float CommonChance = FMath::Max(0.0f, 1.0f - NoneChance - UncommonChance - RareChance - EpicChance - LegendaryChance);

	const float Roll = Rng.GetFraction();
	if (Roll < NoneChance)
	{
		return EItemRarity::None;
	}
	if (Roll < NoneChance + CommonChance)
	{
		return EItemRarity::Common;
	}
	if (Roll < NoneChance + CommonChance + UncommonChance)
	{
		return EItemRarity::Uncommon;
	}
	if (Roll < NoneChance + CommonChance + UncommonChance + RareChance)
	{
		return EItemRarity::Rare;
	}
	if (Roll < NoneChance + CommonChance + UncommonChance + RareChance + EpicChance)
	{
		return EItemRarity::Epic;
	}
	return EItemRarity::Legendary;
}

EItemSet FDropFormula::RollItemSet(EItemRarity Rarity, FRandomStream& Rng)
{
	if (Rarity == EItemRarity::None || Rarity == EItemRarity::Common)
	{
		return EItemSet::None;
	}

	const int32 Roll = Rng.RandRange(0, 2);
	switch (Roll)
	{
	case 0:
		return EItemSet::Warrior;
	case 1:
		return EItemSet::Guardian;
	case 2:
	default:
		return EItemSet::Arcane;
	}
}

FItemInstance FDropFormula::ComputeItemBonus(EItemSlot Slot, int32 Level, EItemRarity Rarity, float Variance)
{
	FItemInstance Item;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.ItemSet = EItemSet::None;
	Item.EnhanceLevel = 0;

	const int32 SafeLevel = FMath::Max(Level, 1);
	const float SafeVariance = FMath::Max(Variance, 0.0f);
	const float BaseBonus = static_cast<float>(SafeLevel) * SafeVariance * GetRarityStatMultiplier(Rarity);

	switch (Slot)
	{
	case EItemSlot::Weapon:
		Item.BonusAtk = BaseBonus;
		break;
	case EItemSlot::Accessory:
		Item.BonusAtk = BaseBonus * 0.5f;
		Item.BonusDef = BaseBonus * 0.3f;
		Item.BonusHp = BaseBonus * 2.0f;
		break;
	case EItemSlot::Helmet:
	case EItemSlot::Top:
	case EItemSlot::Bottom:
	case EItemSlot::Shoes:
	case EItemSlot::Gloves:
	case EItemSlot::Cloak:
		Item.BonusDef = BaseBonus * 0.7f;
		Item.BonusHp = BaseBonus * 3.0f;
		break;
	case EItemSlot::None:
	default:
		break;
	}

	return Item;
}

void FDropFormula::RollAffixes(EItemRarity Rarity, int32 Level, FRandomStream& Rng, FItemInstance& OutItem)
{
	OutItem.BonusCritRate = 0.0f;
	OutItem.BonusAtkSpeed = 0.0f;
	OutItem.BonusMagicAtk = 0.0f;

	const int32 AffixCount = GetAffixCount(Rarity, Rng);
	if (AffixCount <= 0)
	{
		return;
	}

	TArray<EAffixKind> AffixKinds{
		EAffixKind::CritRate,
		EAffixKind::AtkSpeed,
		EAffixKind::MagicAtk
	};
	ShuffleAffixKinds(AffixKinds, Rng);

	const int32 SafeLevel = FMath::Max(Level, 1);
	const int32 ClampedCount = FMath::Min(AffixCount, AffixKinds.Num());
	for (int32 Index = 0; Index < ClampedCount; ++Index)
	{
		switch (AffixKinds[Index])
		{
		case EAffixKind::CritRate:
			OutItem.BonusCritRate = FMath::RoundToFloat(Rng.FRandRange(0.01f, 0.05f) * 1000.0f) / 1000.0f;
			break;
		case EAffixKind::AtkSpeed:
			OutItem.BonusAtkSpeed = FMath::RoundToFloat(Rng.FRandRange(0.05f, 0.15f) * 1000.0f) / 1000.0f;
			break;
		case EAffixKind::MagicAtk:
			OutItem.BonusMagicAtk = static_cast<float>(FMath::RoundToInt(static_cast<float>(SafeLevel) * Rng.FRandRange(0.5f, 1.5f)));
			break;
		default:
			break;
		}
	}
}
