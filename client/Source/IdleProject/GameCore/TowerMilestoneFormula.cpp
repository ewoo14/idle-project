#include "GameCore/TowerMilestoneFormula.h"

float FTowerMilestoneFormula::GetTowerMilestoneMultiplier(int32 HighestFloor)
{
	const int32 SafeHighestFloor = FMath::Max(0, HighestFloor);
	const int32 MilestoneCount = SafeHighestFloor / MilestoneStep;
	return 1.0f + static_cast<float>(MilestoneCount) * MilestoneBonusPerStep;
}
