#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FItemFactory
{
	static FItemInstance RandomDropFromMonster(int32 MonsterLevel);
	static FItemInstance GuaranteedDropForLevel(int32 Level);
};
