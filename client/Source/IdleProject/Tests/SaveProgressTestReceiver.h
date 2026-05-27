#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SaveProgressTestReceiver.generated.h"

UCLASS()
class USaveProgressTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SavedCount = 0;

	UFUNCTION()
	void HandleProgressSaved()
	{
		++SavedCount;
	}
};
