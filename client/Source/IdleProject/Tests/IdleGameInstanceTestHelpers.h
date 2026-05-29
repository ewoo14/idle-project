#pragma once

#include "GameCore/IdleGameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

struct FIdleGameInstanceWorldContextAccessor : UIdleGameInstance
{
	static void Attach(UIdleGameInstance* Instance, FWorldContext* Context)
	{
		static_cast<FIdleGameInstanceWorldContextAccessor*>(Instance)->WorldContext = Context;
	}
};

#endif
