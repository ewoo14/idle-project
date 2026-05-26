#pragma once

#include "CoreMinimal.h"
#include "StatusElementTypes.generated.h"

UENUM(BlueprintType)
enum class ESkillStatusEffect : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Poison = 1 UMETA(DisplayName = "Poison"),
	Burn = 2 UMETA(DisplayName = "Burn"),
	Freeze = 3 UMETA(DisplayName = "Freeze")
};

UENUM(BlueprintType)
enum class ESkillElement : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Fire = 1 UMETA(DisplayName = "Fire"),
	Ice = 2 UMETA(DisplayName = "Ice"),
	Lightning = 3 UMETA(DisplayName = "Lightning"),
	Holy = 4 UMETA(DisplayName = "Holy")
};
