#pragma once

#include "CoreMinimal.h"
#include "ConsumableTypes.generated.h"

UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	AttackTonic = 0,
	GuardTonic = 1,
	AllStatElixir = 2,
	FortuneScroll = 3,
	GoldFeast = 4,
	WisdomBooster = 5
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FConsumableSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	uint8 Type = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	int32 Count = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Idle|Consumable")
	int64 BuffEndUnixSec = 0;
};
