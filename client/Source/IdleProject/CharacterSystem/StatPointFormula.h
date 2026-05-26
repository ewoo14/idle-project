#pragma once

#include "CoreMinimal.h"
#include "StatPointFormula.generated.h"

UENUM(BlueprintType)
enum class EPrimaryStat : uint8
{
	Str UMETA(DisplayName = "STR"),
	Dex UMETA(DisplayName = "DEX"),
	Int UMETA(DisplayName = "INT"),
	Wis UMETA(DisplayName = "WIS"),
	Con UMETA(DisplayName = "CON"),
	Luk UMETA(DisplayName = "LUK")
};

/** Level-up stat point grants mirrored by the server StatPointFormula. */
struct IDLEPROJECT_API FStatPointFormula
{
	static constexpr int32 StatPointsPerLevel = 5;

	static int32 GetStatPointsForLevelUp(int32 NewLevel);
	static int32 GetTotalStatPointsForLevel(int32 Level);
};
