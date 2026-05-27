#pragma once

#include "CoreMinimal.h"
#include "TowerEventTestReceiver.generated.h"

UCLASS()
class UTowerEventTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	int32 Count = 0;
	int32 LastHighestFloor = 0;
	int64 LastTotalReward = 0;

	UFUNCTION()
	void CaptureTowerClimbed(int32 NewHighestFloor, int64 TotalReward)
	{
		++Count;
		LastHighestFloor = NewHighestFloor;
		LastTotalReward = TotalReward;
	}
};
