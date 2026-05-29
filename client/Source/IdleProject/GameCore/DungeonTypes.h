#pragma once

#include "CoreMinimal.h"
#include "DungeonTypes.generated.h"

UENUM(BlueprintType)
enum class EDungeonType : uint8
{
	None = 0 UMETA(Hidden),
	Gold = 1 UMETA(DisplayName = "Gold"),
	Exp = 2 UMETA(DisplayName = "Exp"),
	Essence = 3 UMETA(DisplayName = "Essence")
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FDungeonRunResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	EDungeonType Type = EDungeonType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	int32 Tier = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	int64 GoldReward = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	int64 ExpReward = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Dungeon")
	int64 EssenceReward = 0;
};
