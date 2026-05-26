#include "ItemSystem/DropFormula.h"

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

FItemInstance FDropFormula::ComputeItemBonus(EItemSlot Slot, int32 Level, EItemRarity Rarity, float Variance)
{
	FItemInstance Item;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
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
