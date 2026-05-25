#include "ItemSystem/ItemFactory.h"

namespace
{
EItemRarity RollRarity()
{
	const float Roll = FMath::FRand();
	if (Roll < 0.02f)
	{
		return EItemRarity::None;
	}
	if (Roll < 0.72f)
	{
		return EItemRarity::Common;
	}
	if (Roll < 0.92f)
	{
		return EItemRarity::Uncommon;
	}
	return EItemRarity::Rare;
}

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
}

FItemInstance FItemFactory::RandomDropFromMonster(int32 MonsterLevel)
{
	const int32 SafeLevel = FMath::Max(MonsterLevel, 1);
	const EItemRarity Rarity = RollRarity();
	if (Rarity == EItemRarity::None)
	{
		return FItemInstance();
	}

	const EItemSlot Slot = RollSlot();
	const float BaseBonus = static_cast<float>(SafeLevel) * FMath::FRandRange(1.0f, 2.0f);

	FItemInstance Item;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.EnhanceLevel = 0;
	Item.DisplayName = FText::FromString(FString::Printf(TEXT("%s %s"), *GetRarityAdjective(Rarity), *GetSlotNoun(Slot)));
	Item.ItemId = FName(*FString::Printf(TEXT("%s_%s_L%d"), *UEnum::GetValueAsString(Rarity), *UEnum::GetValueAsString(Slot), SafeLevel));

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
