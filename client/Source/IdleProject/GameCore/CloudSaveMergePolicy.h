#pragma once

#include "CoreMinimal.h"

enum class ECloudSaveMergeDecision : uint8
{
	AdoptServer,
	KeepLocal
};

struct IDLEPROJECT_API FCloudSaveProgressSnapshot
{
	int32 RebirthCount = 0;
	int32 Level = 1;
	int64 Gold = 0;
	int64 LastSeenUnixSec = 0;
};

struct IDLEPROJECT_API FCloudSaveMergePolicy
{
	static ECloudSaveMergeDecision Decide(
		const FCloudSaveProgressSnapshot& Local,
		const FCloudSaveProgressSnapshot& Server)
	{
		if (Server.RebirthCount != Local.RebirthCount)
		{
			return Server.RebirthCount > Local.RebirthCount
				? ECloudSaveMergeDecision::AdoptServer
				: ECloudSaveMergeDecision::KeepLocal;
		}

		if (Server.Level != Local.Level)
		{
			return Server.Level > Local.Level
				? ECloudSaveMergeDecision::AdoptServer
				: ECloudSaveMergeDecision::KeepLocal;
		}

		if (Server.Gold != Local.Gold)
		{
			return Server.Gold > Local.Gold
				? ECloudSaveMergeDecision::AdoptServer
				: ECloudSaveMergeDecision::KeepLocal;
		}

		return Server.LastSeenUnixSec > Local.LastSeenUnixSec
			? ECloudSaveMergeDecision::AdoptServer
			: ECloudSaveMergeDecision::KeepLocal;
	}
};
