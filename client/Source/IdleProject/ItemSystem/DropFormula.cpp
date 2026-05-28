#include "ItemSystem/DropFormula.h"

namespace
{
enum class EAffixKind : uint8
{
	CritRate,
	AtkSpeed,
	MagicAtk,
	PhysDef,
	MagicDef,
	Hp,
	CritDmg
};

int32 GetAffixCount(EItemRarity Rarity, FRandomStream& Rng)
{
	switch (Rarity)
	{
	case EItemRarity::Rare:
		return 1;
	case EItemRarity::Epic:
	case EItemRarity::Unique:
		return 2;
	case EItemRarity::Legendary:
	case EItemRarity::Transcendent:
		return Rng.GetFraction() < 0.5f ? 2 : 3;
	case EItemRarity::Mythic:
		return 3;
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
	case EItemRarity::Rare:
		return 1.7f;
	case EItemRarity::Epic:
		return 2.3f;
	case EItemRarity::Unique:
		return 2.75f;
	case EItemRarity::Legendary:
		return 3.2f;
	case EItemRarity::Transcendent:
		return 3.85f;
	case EItemRarity::Mythic:
		return 4.5f;
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
	const float RareChance = 0.28f + 0.02f * LevelScale;
	const float EpicChance = 0.06f * LevelScale;
	const float UniqueChance = 0.025f * LevelScale;
	const float LegendaryChance = 0.015f * LevelScale;
	const float TranscendentChance = 0.007f * LevelScale;
	const float MythicChance = 0.005f * LevelScale;
	const float CommonChance = FMath::Max(0.0f, 1.0f - NoneChance - RareChance - EpicChance - UniqueChance - LegendaryChance - TranscendentChance - MythicChance);

	const float Roll = Rng.GetFraction();
	if (Roll < NoneChance)
	{
		return EItemRarity::None;
	}
	if (Roll < NoneChance + CommonChance)
	{
		return EItemRarity::Common;
	}
	if (Roll < NoneChance + CommonChance + RareChance)
	{
		return EItemRarity::Rare;
	}
	if (Roll < NoneChance + CommonChance + RareChance + EpicChance)
	{
		return EItemRarity::Epic;
	}
	if (Roll < NoneChance + CommonChance + RareChance + EpicChance + UniqueChance)
	{
		return EItemRarity::Unique;
	}
	if (Roll < NoneChance + CommonChance + RareChance + EpicChance + UniqueChance + LegendaryChance)
	{
		return EItemRarity::Legendary;
	}
	if (Roll < NoneChance + CommonChance + RareChance + EpicChance + UniqueChance + LegendaryChance + TranscendentChance)
	{
		return EItemRarity::Transcendent;
	}
	return EItemRarity::Mythic;
}

EItemSet FDropFormula::RollItemSet(EItemRarity Rarity, FRandomStream& Rng)
{
	if (Rarity == EItemRarity::None || Rarity == EItemRarity::Common)
	{
		return EItemSet::None;
	}

	const int32 Roll = Rng.RandRange(0, 6);
	switch (Roll)
	{
	case 0:
		return EItemSet::Warrior;
	case 1:
		return EItemSet::Guardian;
	case 2:
		return EItemSet::Arcane;
	case 3:
		return EItemSet::Assassin;
	case 4:
		return EItemSet::Hunter;
	case 5:
		return EItemSet::Holy;
	case 6:
	default:
		return EItemSet::Berserker;
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
	OutItem.BonusPhysDef = 0.0f;
	OutItem.BonusMagicDef = 0.0f;
	OutItem.BonusAffixHp = 0.0f;
	OutItem.BonusCritDmg = 0.0f;

	const int32 AffixCount = GetAffixCount(Rarity, Rng);
	if (AffixCount <= 0)
	{
		return;
	}

	TArray<EAffixKind> AffixKinds{
		EAffixKind::CritRate,
		EAffixKind::AtkSpeed,
		EAffixKind::MagicAtk,
		EAffixKind::PhysDef,
		EAffixKind::MagicDef,
		EAffixKind::Hp,
		EAffixKind::CritDmg
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
		case EAffixKind::PhysDef:
			OutItem.BonusPhysDef = static_cast<float>(FMath::Max(1, FMath::RoundToInt(static_cast<float>(SafeLevel) * Rng.FRandRange(0.3f, 1.0f))));
			break;
		case EAffixKind::MagicDef:
			OutItem.BonusMagicDef = static_cast<float>(FMath::Max(1, FMath::RoundToInt(static_cast<float>(SafeLevel) * Rng.FRandRange(0.3f, 1.0f))));
			break;
		case EAffixKind::Hp:
			OutItem.BonusAffixHp = static_cast<float>(FMath::Max(1, FMath::RoundToInt(static_cast<float>(SafeLevel) * Rng.FRandRange(2.0f, 5.0f))));
			break;
		case EAffixKind::CritDmg:
			OutItem.BonusCritDmg = FMath::RoundToFloat(Rng.FRandRange(0.05f, 0.20f) * 1000.0f) / 1000.0f;
			break;
		default:
			break;
		}
	}
}
