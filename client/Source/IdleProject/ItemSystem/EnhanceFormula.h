#pragma once

#include "CoreMinimal.h"
#include "ItemSystem/ItemTypes.h"

struct IDLEPROJECT_API FEnhanceAttemptOutcome
{
	bool bAttempted = false;
	bool bSuccess = false;
	bool bConsumedProtection = false;
	int32 NewLevel = 0;
	int32 NewFailStreak = 0;
	bool bPityTriggered = false;
};

struct IDLEPROJECT_API FEnhanceFormula
{
	static constexpr int32 MaxEnhanceLevel = 50;
	static constexpr int32 SafeMaxLevel = 9;
	static constexpr int32 PityThreshold = 12;

	static int64 GetEnhanceCost(int32 CurrentLevel);
	static int64 GetEnhanceCost(int32 CurrentLevel, EItemRarity Rarity);
	static int64 GetRarityCostMultiplier(EItemRarity Rarity);
	static float GetEnhanceSuccessRate(int32 CurrentLevel);
	static bool RollEnhanceSuccess(float SuccessRate, FRandomStream& Stream);
	static bool IsRiskLevel(int32 CurrentLevel);
	static FEnhanceAttemptOutcome ResolveAttempt(int32 CurrentLevel, int32 FailStreak, bool bUseProtection, bool bHasProtection, float Roll);
};
