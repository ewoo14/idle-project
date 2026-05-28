#include "ItemSystem/RarityMigration.h"

EItemRarity FRarityMigration::MigrateLegacy(int32 LegacyValue)
{
	switch (LegacyValue)
	{
	case 1:
		return EItemRarity::Common;
	case 2:
	case 3:
		return EItemRarity::Rare;
	case 4:
		return EItemRarity::Epic;
	case 5:
		return EItemRarity::Legendary;
	case 6:
		return EItemRarity::Mythic;
	default:
		return EItemRarity::None;
	}
}
