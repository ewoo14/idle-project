#include "GameCore/TowerService.h"

#include "GameCore/TowerFormula.h"
#include "GameCore/TowerMilestoneFormula.h"

void UTowerService::InitializeTower()
{
	HighestFloor = 0;
}

int64 UTowerService::TryClimbTower(int64 CombatPower)
{
	if (CombatPower <= 0)
	{
		return 0;
	}

	int64 TotalReward = 0;
	int32 FloorsCleared = 0;
	int32 NextFloor = HighestFloor + 1;

	while (FloorsCleared < MaxClimbPerCall && FTowerFormula::CanClearFloor(CombatPower, NextFloor))
	{
		HighestFloor = NextFloor;
		TotalReward = FMath::Min<int64>(MAX_int64, TotalReward + FTowerFormula::GetFloorReward(NextFloor));
		++FloorsCleared;
		++NextFloor;
	}

	if (FloorsCleared > 0)
	{
		OnTowerClimbed.Broadcast(HighestFloor, TotalReward);
	}

	return TotalReward;
}

int64 UTowerService::GetNextFloorRequiredPower() const
{
	return FTowerFormula::GetFloorRequiredPower(HighestFloor + 1);
}

float UTowerService::GetMilestoneMultiplier() const
{
	return FTowerMilestoneFormula::GetTowerMilestoneMultiplier(HighestFloor);
}
