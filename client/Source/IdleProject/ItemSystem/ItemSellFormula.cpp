#include "ItemSystem/ItemSellFormula.h"

namespace
{
	// 서버 RARITY_SELL_BASE 1:1. EItemRarity(None0..Mythic7).
	int64 RaritySellBase(EItemRarity Rarity)
	{
		switch (Rarity)
		{
		case EItemRarity::Common:       return 100;
		case EItemRarity::Rare:         return 400;
		case EItemRarity::Epic:         return 1500;
		case EItemRarity::Unique:       return 6000;
		case EItemRarity::Legendary:    return 25000;
		case EItemRarity::Transcendent: return 100000;
		case EItemRarity::Mythic:       return 400000;
		default:                        return 0; // None
		}
	}
}

int64 FItemSellFormula::ComputeSellValue(EItemRarity Rarity, int32 EnhanceLevel)
{
	const int32 Lv = FMath::Max(0, EnhanceLevel);
	const double Value = static_cast<double>(RaritySellBase(Rarity)) * (1.0 + 0.2 * Lv);
	return static_cast<int64>(FMath::RoundToDouble(Value));
}

int64 FItemSellFormula::ComputeItemPower(const FItemInstance& Item)
{
	const double Raw =
		Item.BonusAtk + Item.BonusMagicAtk +
		Item.BonusDef + Item.BonusPhysDef + Item.BonusMagicDef +
		(Item.BonusHp + Item.BonusAffixHp) / 10.0 +
		Item.BonusCritRate * 1000.0 + Item.BonusCritDmg * 100.0;
	const int32 Lv = FMath::Max(0, Item.EnhanceLevel);
	return static_cast<int64>(FMath::RoundToDouble(FMath::Max(0.0, Raw) * (1.0 + 0.1 * Lv)));
}
