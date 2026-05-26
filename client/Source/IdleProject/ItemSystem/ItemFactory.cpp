#include "ItemSystem/ItemFactory.h"

#include "ItemSystem/DropFormula.h"

namespace
{
EItemSlot RollSlot()
{
	const float Roll = FMath::FRandRange(0.0f, 100.0f);
	if (Roll < 50.0f)
	{
		return EItemSlot::Weapon;
	}
	if (Roll < 57.0f)
	{
		return EItemSlot::Helmet;
	}
	if (Roll < 64.0f)
	{
		return EItemSlot::Top;
	}
	if (Roll < 71.0f)
	{
		return EItemSlot::Bottom;
	}
	if (Roll < 78.0f)
	{
		return EItemSlot::Shoes;
	}
	if (Roll < 84.0f)
	{
		return EItemSlot::Gloves;
	}
	if (Roll < 92.0f)
	{
		return EItemSlot::Cloak;
	}
	return EItemSlot::Accessory;
}

FString GetRarityAdjective(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Uncommon:
		return TEXT("정련된");
	case EItemRarity::Rare:
		return TEXT("광택의");
	case EItemRarity::Epic:
		return TEXT("Epic");
	case EItemRarity::Legendary:
		return TEXT("Legendary");
	case EItemRarity::None:
		return TEXT("Item");
	case EItemRarity::Common:
	default:
		return TEXT("거친");
	}
}

FString GetSlotNoun(EItemSlot Slot)
{
	switch (Slot)
	{
	case EItemSlot::Weapon:
		return TEXT("검");
	case EItemSlot::Helmet:
		return TEXT("투구");
	case EItemSlot::Top:
		return TEXT("상의");
	case EItemSlot::Bottom:
		return TEXT("하의");
	case EItemSlot::Shoes:
		return TEXT("신발");
	case EItemSlot::Gloves:
		return TEXT("장갑");
	case EItemSlot::Cloak:
		return TEXT("외투");
	case EItemSlot::Accessory:
		return TEXT("장신구");
	case EItemSlot::None:
	default:
		return TEXT("장비");
	}
}

FItemInstance BuildDropForLevel(int32 Level, bool bGuaranteeItem)
{
	const int32 SafeLevel = FMath::Max(Level, 1);
	FRandomStream RarityRng(FMath::Rand());
	EItemRarity Rarity = FDropFormula::RollRarityForLevel(SafeLevel, RarityRng);
	if (Rarity == EItemRarity::None)
	{
		if (!bGuaranteeItem)
		{
			return FItemInstance();
		}
		Rarity = EItemRarity::Common;
	}

	const EItemSlot Slot = RollSlot();

	FItemInstance Item = FDropFormula::ComputeItemBonus(Slot, SafeLevel, Rarity, FMath::FRandRange(1.0f, 2.0f));
	FRandomStream AffixRng(FMath::Rand());
	FDropFormula::RollAffixes(Rarity, SafeLevel, AffixRng, Item);
	Item.DisplayName = FText::FromString(FString::Printf(TEXT("%s %s"), *GetRarityAdjective(Rarity), *GetSlotNoun(Slot)));
	Item.ItemId = FName(*FString::Printf(TEXT("%s_%s_L%d"), *UEnum::GetValueAsString(Rarity), *UEnum::GetValueAsString(Slot), SafeLevel));

	return Item;
}
}

FItemInstance FItemFactory::RandomDropFromMonster(int32 MonsterLevel)
{
	return BuildDropForLevel(MonsterLevel, false);
}

FItemInstance FItemFactory::GuaranteedDropForLevel(int32 Level)
{
	return BuildDropForLevel(Level, true);
}
