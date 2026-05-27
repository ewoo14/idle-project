#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TowerService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerClimbed, int32, NewHighestFloor, int64, TotalReward);

UCLASS(BlueprintType)
class IDLEPROJECT_API UTowerService : public UObject
{
	GENERATED_BODY()

public:
	static constexpr int32 MaxClimbPerCall = 100;

	UFUNCTION(BlueprintCallable, Category = "Idle|Tower")
	void InitializeTower();

	UFUNCTION(BlueprintCallable, Category = "Idle|Tower")
	int64 TryClimbTower(int64 CombatPower);

	UFUNCTION(BlueprintPure, Category = "Idle|Tower")
	int32 GetHighestFloor() const { return HighestFloor; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Tower")
	void SetHighestFloor(int32 Floor);

	UFUNCTION(BlueprintPure, Category = "Idle|Tower")
	int64 GetNextFloorRequiredPower() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Tower")
	float GetMilestoneMultiplier() const;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Tower")
	FOnTowerClimbed OnTowerClimbed;

private:
	UPROPERTY()
	int32 HighestFloor = 0;
};
