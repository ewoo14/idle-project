#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"
#include "RuneSystem/RuneTypes.h"
#include "RuneCodexTypes.generated.h"

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	ERuneType RuneType = ERuneType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	EItemRarity Rarity = EItemRarity::None;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	bool bUnlocked = false;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexBonus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float CoreStatAdd = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	float UtilCapExtension = 0.0f;
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneCodexCompletion
{
	GENERATED_BODY()

	FRuneCodexCompletion()
	{
		RowComplete.Init(false, 7);
	}

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	int32 UnlockedCells = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	int32 TotalCells = 63;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	TArray<bool> RowComplete;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	bool bCoreCategoryComplete = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Rune")
	bool bUtilCategoryComplete = false;
};
