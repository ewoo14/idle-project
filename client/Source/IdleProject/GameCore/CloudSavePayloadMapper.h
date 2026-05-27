#pragma once

#include "CoreMinimal.h"
#include "GameCore/CloudSaveMergePolicy.h"

class UIdleSaveGame;

struct IDLEPROJECT_API FCloudSavePayloadMapper
{
	static bool SaveToPayloadJson(const UIdleSaveGame& SaveGame, FString& OutPayloadJson);
	static bool PayloadJsonToSave(const FString& PayloadJson, UIdleSaveGame& OutSaveGame);
	static bool ExtractSnapshot(const FString& PayloadJson, FCloudSaveProgressSnapshot& OutSnapshot);
};
