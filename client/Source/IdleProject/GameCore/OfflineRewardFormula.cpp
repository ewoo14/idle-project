#include "GameCore/OfflineRewardFormula.h"

int64 FOfflineRewardFormula::BaseGoldPerSec(int32 Level)
{
	return static_cast<int64>(Level) * 4;
}

int64 FOfflineRewardFormula::BaseExpPerSec(int32 Level)
{
	return static_cast<int64>(Level) * 4;
}

double FOfflineRewardFormula::ComputeTimeBonusMultiplier(int64 CappedSeconds, int32 RebirthCount)
{
	const int32 SafeRebirthCount = FMath::Max(0, RebirthCount);
	const double CappedHours = FMath::Min(static_cast<double>(CappedSeconds) / 3600.0, 12.0);
	return 1.0 + CappedHours * OFFLINE_TIME_BONUS_PER_HOUR + SafeRebirthCount * OFFLINE_REBIRTH_BONUS;
}

FOfflineRewardResult FOfflineRewardFormula::ComputeOfflineRewards(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount)
{
	if (Level < 1 || LastSeenUnixSec < 0 || NowUnixSec < 0)
	{
		return {};
	}

	const int64 ElapsedSeconds = FMath::Max<int64>(0, NowUnixSec - LastSeenUnixSec);
	const int64 CappedSeconds = FMath::Min<int64>(ElapsedSeconds, OFFLINE_CAP_SECONDS);
	if (CappedSeconds == 0)
	{
		return {};
	}

	const double TimeBonusMultiplier = ComputeTimeBonusMultiplier(CappedSeconds, RebirthCount);
	return {
		CappedSeconds,
		RoundReward(static_cast<double>(BaseGoldPerSec(Level)) * static_cast<double>(CappedSeconds) * OFFLINE_EFFICIENCY * TimeBonusMultiplier),
		RoundReward(static_cast<double>(BaseExpPerSec(Level)) * static_cast<double>(CappedSeconds) * OFFLINE_EFFICIENCY * TimeBonusMultiplier),
		TimeBonusMultiplier
	};
}

int64 FOfflineRewardFormula::RoundReward(double Value)
{
	return static_cast<int64>(FMath::FloorToDouble(Value + 0.5));
}
