#include "GameCore/TranscendFormula.h"

float FTranscendFormula::GetTranscendStatMultiplier(int32 TranscendCount)
{
	const int32 SafeTranscendCount = FMath::Max(0, TranscendCount);
	return 1.0f + static_cast<float>(SafeTranscendCount) * 0.25f;
}

bool FTranscendFormula::CanTranscend(int32 RebirthCount)
{
	return RebirthCount >= TranscendRebirthThreshold;
}
