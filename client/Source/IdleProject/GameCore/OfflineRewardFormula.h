#pragma once

#include "CoreMinimal.h"
#include "OfflineRewardFormula.generated.h"

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FOfflineRewardResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Offline")
	int64 CappedSeconds = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Offline")
	int64 Gold = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Offline")
	int64 Exp = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Offline")
	double TimeBonusMultiplier = 1.0;
};

struct IDLEPROJECT_API FOfflineRewardFormula
{
	static constexpr int64 OFFLINE_CAP_SECONDS = 12 * 3600;
	static constexpr double OFFLINE_EFFICIENCY = 0.75;
	static constexpr double OFFLINE_TIME_BONUS_PER_HOUR = 0.005;
	static constexpr double OFFLINE_REBIRTH_BONUS = 0.05;

	static int64 BaseGoldPerSec(int32 Level);
	static int64 BaseExpPerSec(int32 Level);
	static double ComputeTimeBonusMultiplier(int64 CappedSeconds, int32 RebirthCount);
	static FOfflineRewardResult ComputeOfflineRewards(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount = 0);

private:
	static int64 RoundReward(double Value);
};
