#include "ItemSystem/ItemFactory.h"

#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/DropFormula.h"

namespace
{
struct FBaseItemDefinition
{
	FName BaseItemId;
	EItemSlot Slot = EItemSlot::None;
	const TCHAR* DisplayName = TEXT("Item");
	const TCHAR* DisplayNameKey = TEXT("BASE_ITEM_LONGSWORD");
	float AtkScale = 1.0f;
	float DefScale = 1.0f;
	float HpScale = 1.0f;
};

const TArray<FBaseItemDefinition>& GetBaseItemCatalog()
{
	static const TArray<FBaseItemDefinition> Catalog = {
		{TEXT("longsword"), EItemSlot::Weapon, TEXT("Longsword"), TEXT("BASE_ITEM_LONGSWORD"), 1.05f, 1.0f, 1.0f},
		{TEXT("greatsword"), EItemSlot::Weapon, TEXT("Greatsword"), TEXT("BASE_ITEM_GREATSWORD"), 1.15f, 0.9f, 1.0f},
		{TEXT("dagger"), EItemSlot::Weapon, TEXT("Dagger"), TEXT("BASE_ITEM_DAGGER"), 0.9f, 1.0f, 1.0f},
		{TEXT("bow"), EItemSlot::Weapon, TEXT("Bow"), TEXT("BASE_ITEM_BOW"), 1.0f, 1.0f, 1.0f},
		{TEXT("staff"), EItemSlot::Weapon, TEXT("Staff"), TEXT("BASE_ITEM_STAFF"), 0.9f, 1.0f, 1.0f},
		{TEXT("wand"), EItemSlot::Weapon, TEXT("Wand"), TEXT("BASE_ITEM_WAND"), 0.85f, 1.0f, 1.0f},
		{TEXT("helm"), EItemSlot::Helmet, TEXT("Helm"), TEXT("BASE_ITEM_HELM"), 1.0f, 1.08f, 1.0f},
		{TEXT("hood"), EItemSlot::Helmet, TEXT("Hood"), TEXT("BASE_ITEM_HOOD"), 1.0f, 0.92f, 1.05f},
		{TEXT("circlet"), EItemSlot::Helmet, TEXT("Circlet"), TEXT("BASE_ITEM_CIRCLET"), 1.0f, 1.0f, 1.0f},
		{TEXT("armor"), EItemSlot::Top, TEXT("Armor"), TEXT("BASE_ITEM_ARMOR"), 1.0f, 1.1f, 1.05f},
		{TEXT("robe"), EItemSlot::Top, TEXT("Robe"), TEXT("BASE_ITEM_ROBE"), 1.0f, 0.9f, 1.0f},
		{TEXT("jacket"), EItemSlot::Top, TEXT("Jacket"), TEXT("BASE_ITEM_JACKET"), 1.0f, 1.0f, 0.95f},
		{TEXT("greaves"), EItemSlot::Bottom, TEXT("Greaves"), TEXT("BASE_ITEM_GREAVES"), 1.0f, 1.08f, 1.0f},
		{TEXT("trousers"), EItemSlot::Bottom, TEXT("Battle Trousers"), TEXT("BASE_ITEM_TROUSERS"), 1.0f, 0.95f, 0.95f},
		{TEXT("leggings"), EItemSlot::Bottom, TEXT("Arcane Leggings"), TEXT("BASE_ITEM_LEGGINGS"), 1.0f, 0.9f, 1.05f},
		{TEXT("boots"), EItemSlot::Shoes, TEXT("Boots"), TEXT("BASE_ITEM_BOOTS"), 1.0f, 1.0f, 1.0f},
		{TEXT("shoes"), EItemSlot::Shoes, TEXT("Shoes"), TEXT("BASE_ITEM_SHOES"), 1.0f, 0.92f, 0.9f},
		{TEXT("sandals"), EItemSlot::Shoes, TEXT("Blessed Sandals"), TEXT("BASE_ITEM_SANDALS"), 1.0f, 0.95f, 1.05f},
		{TEXT("gauntlets"), EItemSlot::Gloves, TEXT("Gauntlets"), TEXT("BASE_ITEM_GAUNTLETS"), 1.0f, 1.08f, 1.0f},
		{TEXT("gloves"), EItemSlot::Gloves, TEXT("Gloves"), TEXT("BASE_ITEM_GLOVES"), 1.0f, 0.92f, 0.95f},
		{TEXT("bracers"), EItemSlot::Gloves, TEXT("Bracers"), TEXT("BASE_ITEM_BRACERS"), 1.0f, 1.0f, 1.0f},
		{TEXT("cloak"), EItemSlot::Cloak, TEXT("Cloak"), TEXT("BASE_ITEM_CLOAK"), 1.0f, 1.0f, 1.0f},
		{TEXT("cape"), EItemSlot::Cloak, TEXT("Battle Cape"), TEXT("BASE_ITEM_CAPE"), 1.0f, 1.02f, 0.95f},
		{TEXT("mantle"), EItemSlot::Cloak, TEXT("Sage Mantle"), TEXT("BASE_ITEM_MANTLE"), 1.0f, 0.95f, 1.08f},
		{TEXT("ring"), EItemSlot::Accessory, TEXT("Ring"), TEXT("BASE_ITEM_RING"), 1.0f, 1.0f, 1.0f},
		{TEXT("amulet"), EItemSlot::Accessory, TEXT("Amulet"), TEXT("BASE_ITEM_AMULET"), 0.95f, 1.0f, 1.05f},
		{TEXT("talisman"), EItemSlot::Accessory, TEXT("Talisman"), TEXT("BASE_ITEM_TALISMAN"), 0.9f, 1.08f, 1.0f}
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

const TCHAR* GetRarityDisplayKey(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Rare:
		return TEXT("RARITY_RARE");
	case EItemRarity::Epic:
		return TEXT("RARITY_EPIC");
	case EItemRarity::Unique:
		return TEXT("RARITY_UNIQUE");
	case EItemRarity::Legendary:
		return TEXT("RARITY_LEGENDARY");
	case EItemRarity::Transcendent:
		return TEXT("RARITY_TRANSCENDENT");
	case EItemRarity::Mythic:
		return TEXT("RARITY_MYTHIC");
	case EItemRarity::None:
		return TEXT("RARITY_NONE");
	case EItemRarity::Common:
	default:
		return TEXT("RARITY_COMMON");
	}
}

FText BuildLocalizedItemDisplayName(EItemRarity Rarity, const FBaseItemDefinition& BaseItem)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Rarity"), IdleProject::Localization::UI(GetRarityDisplayKey(Rarity)));
	Args.Add(TEXT("BaseItem"), IdleProject::Localization::UI(BaseItem.DisplayNameKey));
	return IdleProject::Localization::UI(TEXT("ITEM_NAME_FORMAT"), Args);
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
	Item.DisplayName = BuildLocalizedItemDisplayName(Rarity, BaseItem);
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
