#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FRarityMigration
{
	static EItemRarity MigrateLegacy(int32 LegacyValue);
};
