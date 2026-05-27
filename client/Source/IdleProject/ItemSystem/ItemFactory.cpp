#include "ItemSystem/ItemFactory.h"

#include "ItemSystem/DropFormula.h"

namespace
{
struct FBaseItemDefinition
{
	FName BaseItemId;
	EItemSlot Slot = EItemSlot::None;
	const TCHAR* DisplayName = TEXT("Item");
	float AtkScale = 1.0f;
	float DefScale = 1.0f;
	float HpScale = 1.0f;
};

const TArray<FBaseItemDefinition>& GetBaseItemCatalog()
{
	static const TArray<FBaseItemDefinition> Catalog = {
		{TEXT("longsword"), EItemSlot::Weapon, TEXT("Longsword"), 1.05f, 1.0f, 1.0f},
		{TEXT("greatsword"), EItemSlot::Weapon, TEXT("Greatsword"), 1.15f, 0.9f, 1.0f},
		{TEXT("dagger"), EItemSlot::Weapon, TEXT("Dagger"), 0.9f, 1.0f, 1.0f},
		{TEXT("bow"), EItemSlot::Weapon, TEXT("Bow"), 1.0f, 1.0f, 1.0f},
		{TEXT("staff"), EItemSlot::Weapon, TEXT("Staff"), 0.9f, 1.0f, 1.0f},
		{TEXT("wand"), EItemSlot::Weapon, TEXT("Wand"), 0.85f, 1.0f, 1.0f},
		{TEXT("helm"), EItemSlot::Helmet, TEXT("Helm"), 1.0f, 1.08f, 1.0f},
		{TEXT("hood"), EItemSlot::Helmet, TEXT("Hood"), 1.0f, 0.92f, 1.05f},
		{TEXT("circlet"), EItemSlot::Helmet, TEXT("Circlet"), 1.0f, 1.0f, 1.0f},
		{TEXT("armor"), EItemSlot::Top, TEXT("Armor"), 1.0f, 1.1f, 1.05f},
		{TEXT("robe"), EItemSlot::Top, TEXT("Robe"), 1.0f, 0.9f, 1.0f},
		{TEXT("jacket"), EItemSlot::Top, TEXT("Jacket"), 1.0f, 1.0f, 0.95f},
		{TEXT("greaves"), EItemSlot::Bottom, TEXT("Greaves"), 1.0f, 1.08f, 1.0f},
		{TEXT("trousers"), EItemSlot::Bottom, TEXT("Battle Trousers"), 1.0f, 0.95f, 0.95f},
		{TEXT("leggings"), EItemSlot::Bottom, TEXT("Arcane Leggings"), 1.0f, 0.9f, 1.05f},
		{TEXT("boots"), EItemSlot::Shoes, TEXT("Boots"), 1.0f, 1.0f, 1.0f},
		{TEXT("shoes"), EItemSlot::Shoes, TEXT("Shoes"), 1.0f, 0.92f, 0.9f},
		{TEXT("sandals"), EItemSlot::Shoes, TEXT("Blessed Sandals"), 1.0f, 0.95f, 1.05f},
		{TEXT("gauntlets"), EItemSlot::Gloves, TEXT("Gauntlets"), 1.0f, 1.08f, 1.0f},
		{TEXT("gloves"), EItemSlot::Gloves, TEXT("Gloves"), 1.0f, 0.92f, 0.95f},
		{TEXT("bracers"), EItemSlot::Gloves, TEXT("Bracers"), 1.0f, 1.0f, 1.0f},
		{TEXT("cloak"), EItemSlot::Cloak, TEXT("Cloak"), 1.0f, 1.0f, 1.0f},
		{TEXT("cape"), EItemSlot::Cloak, TEXT("Battle Cape"), 1.0f, 1.02f, 0.95f},
		{TEXT("mantle"), EItemSlot::Cloak, TEXT("Sage Mantle"), 1.0f, 0.95f, 1.08f},
		{TEXT("ring"), EItemSlot::Accessory, TEXT("Ring"), 1.0f, 1.0f, 1.0f},
		{TEXT("amulet"), EItemSlot::Accessory, TEXT("Amulet"), 0.95f, 1.0f, 1.05f},
		{TEXT("talisman"), EItemSlot::Accessory, TEXT("Talisman"), 0.9f, 1.08f, 1.0f}
	};
	return Catalog;
}

EItemSlot RollSlot(FRandomStream& Rng)
{
	const float Roll = Rng.FRandRange(0.0f, 100.0f);
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

const FBaseItemDefinition& RollBaseItem(EItemSlot Slot, FRandomStream& Rng)
{
	TArray<const FBaseItemDefinition*> Candidates;
	for (const FBaseItemDefinition& Definition : GetBaseItemCatalog())
	{
		if (Definition.Slot == Slot)
		{
			Candidates.Add(&Definition);
		}
	}

	if (Candidates.IsEmpty())
	{
		return GetBaseItemCatalog()[0];
	}

	return *Candidates[Rng.RandRange(0, Candidates.Num() - 1)];
}

FString GetRarityAdjective(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Uncommon:
		return TEXT("Sturdy");
	case EItemRarity::Rare:
		return TEXT("Gleaming");
	case EItemRarity::Epic:
		return TEXT("Epic");
	case EItemRarity::Legendary:
		return TEXT("Legendary");
	case EItemRarity::Mythic:
		return TEXT("Mythic");
	case EItemRarity::None:
		return TEXT("Item");
	case EItemRarity::Common:
	default:
		return TEXT("Rough");
	}
}

FItemInstance BuildDropForLevel(int32 Level, bool bGuaranteeItem, FRandomStream& Rng)
{
	const int32 SafeLevel = FMath::Max(Level, 1);
	EItemRarity Rarity = FDropFormula::RollRarityForLevel(SafeLevel, Rng);
	if (Rarity == EItemRarity::None)
	{
		if (!bGuaranteeItem)
		{
			return FItemInstance();
		}
		Rarity = EItemRarity::Common;
	}

	const EItemSlot Slot = RollSlot(Rng);
	const FBaseItemDefinition& BaseItem = RollBaseItem(Slot, Rng);

	FItemInstance Item = FDropFormula::ComputeItemBonus(Slot, SafeLevel, Rarity, Rng.FRandRange(1.0f, 2.0f));
	Item.BaseItemId = BaseItem.BaseItemId;
	Item.BonusAtk *= BaseItem.AtkScale;
	Item.BonusDef *= BaseItem.DefScale;
	Item.BonusHp *= BaseItem.HpScale;
	Item.ItemSet = FDropFormula::RollItemSet(Rarity, Rng);
	FDropFormula::RollAffixes(Rarity, SafeLevel, Rng, Item);
	Item.DisplayName = FText::FromString(FString::Printf(TEXT("%s %s"), *GetRarityAdjective(Rarity), BaseItem.DisplayName));
	Item.ItemId = FName(*FString::Printf(TEXT("%s_%s_L%d"), *BaseItem.BaseItemId.ToString(), *UEnum::GetValueAsString(Rarity), SafeLevel));

	return Item;
}
}

FItemInstance FItemFactory::RandomDropFromMonster(int32 MonsterLevel)
{
	FRandomStream Rng(FMath::Rand());
	return BuildDropForLevel(MonsterLevel, false, Rng);
}

FItemInstance FItemFactory::GuaranteedDropForLevel(int32 Level)
{
	FRandomStream Rng(FMath::Rand());
	return BuildDropForLevel(Level, true, Rng);
}

FItemInstance FItemFactory::RandomDropFromMonster(int32 MonsterLevel, FRandomStream& Rng)
{
	return BuildDropForLevel(MonsterLevel, false, Rng);
}

FItemInstance FItemFactory::GuaranteedDropForLevel(int32 Level, FRandomStream& Rng)
{
	return BuildDropForLevel(Level, true, Rng);
}
