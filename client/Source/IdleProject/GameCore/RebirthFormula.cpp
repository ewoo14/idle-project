#include "GameCore/RebirthFormula.h"

int32 FRebirthFormula::GetRebirthPointsReward(int32 RebirthCount, int32 LevelAtRebirth)
{
	const int32 SafeRebirthCount = FMath::Max(0, RebirthCount);
	const int32 SafeLevel = FMath::Max(100, LevelAtRebirth);
	const int32 LevelBonus = (SafeLevel - 100) / 10;

	return 5 + SafeRebirthCount * 2 + LevelBonus;
}
