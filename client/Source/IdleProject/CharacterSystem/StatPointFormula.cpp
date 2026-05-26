#include "CharacterSystem/StatPointFormula.h"

int32 FStatPointFormula::GetStatPointsForLevelUp(int32 NewLevel)
{
	return NewLevel > 1 ? StatPointsPerLevel : 0;
}

int32 FStatPointFormula::GetTotalStatPointsForLevel(int32 Level)
{
	return FMath::Max(0, Level - 1) * StatPointsPerLevel;
}
