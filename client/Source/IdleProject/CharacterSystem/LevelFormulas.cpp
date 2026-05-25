#include "CharacterSystem/LevelFormulas.h"

namespace
{
int64 RoundToInt64(double Value)
{
	return FMath::RoundToInt64(Value);
}
}

int64 FLevelFormulas::ExpToNext(int32 Level)
{
	const int32 SafeLevel = FMath::Max(Level, 1);
	if (SafeLevel == LEVEL_CAP)
	{
		return 0;
	}

	return RoundToInt64(50.0 * FMath::Pow(static_cast<double>(SafeLevel), 1.7) + 100.0 * static_cast<double>(SafeLevel));
}

int64 FLevelFormulas::CumulativeExp(int32 Level)
{
	const int32 SafeLevel = FMath::Clamp(Level, 1, LEVEL_CAP);
	int64 Total = 0;
	for (int32 CurrentLevel = 1; CurrentLevel < SafeLevel; ++CurrentLevel)
	{
		Total += ExpToNext(CurrentLevel);
	}
	return Total;
}

float FLevelFormulas::EnhanceSuccessRate(int32 CurrentStage)
{
	const int32 SafeStage = FMath::Clamp(CurrentStage, 0, 15);
	if (SafeStage <= 5)
	{
		return 1.0f;
	}
	if (SafeStage <= 10)
	{
		return 0.9f;
	}

	switch (SafeStage)
	{
	case 11:
		return 0.7f;
	case 12:
		return 0.6f;
	case 13:
		return 0.5f;
	case 14:
		return 0.4f;
	case 15:
	default:
		return 0.3f;
	}
}
