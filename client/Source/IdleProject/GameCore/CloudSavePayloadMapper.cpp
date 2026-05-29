#include "GameCore/CloudSavePayloadMapper.h"

#include "CharacterSystem/LevelFormulas.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MasteryFormula.h"
#include "ItemSystem/ItemTypes.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
static const TCHAR* FullSaveFieldName = TEXT("clientSave");

int32 GetMaxEquipmentGrade(const UIdleSaveGame& SaveGame)
{
	int32 MaxGrade = 0;
	for (const FItemInstance& Item : SaveGame.InventoryItems)
	{
		MaxGrade = FMath::Max(MaxGrade, static_cast<int32>(Item.Rarity));
	}
	return FMath::Clamp(MaxGrade, 0, static_cast<int32>(EItemRarity::Mythic));
}

int64 ComputeTotalExp(const UIdleSaveGame& SaveGame)
{
	const int64 LevelFloor = FLevelFormulas::CumulativeExp(SaveGame.CharacterLevel);
	return FMath::Max<int64>(LevelFloor, LevelFloor + FMath::Max<int64>(0, SaveGame.CurrentExp));
}

TArray<int32> ComputeMasteryLevels(const UIdleSaveGame& SaveGame)
{
	TArray<int32> Levels;
	Levels.Init(0, FMasteryFormula::TrackCount);
	for (const FMasterySaveEntry& Entry : SaveGame.Mastery)
	{
		if (Entry.Track >= FMasteryFormula::TrackCount)
		{
			continue;
		}

		int32 Level = 0;
		int64 Into = 0;
		int64 Need = 0;
		FMasteryFormula::LevelFromTotalXp(Entry.TotalXp, Level, Into, Need);
		Levels[Entry.Track] = Level;
	}
	return Levels;
}

bool DeserializePayload(const FString& PayloadJson, TSharedPtr<FJsonObject>& OutPayload)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PayloadJson);
	return FJsonSerializer::Deserialize(Reader, OutPayload) && OutPayload.IsValid();
}
}

bool FCloudSavePayloadMapper::SaveToPayloadJson(const UIdleSaveGame& SaveGame, FString& OutPayloadJson)
{
	TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetNumberField(TEXT("level"), FMath::Clamp(SaveGame.CharacterLevel, 1, 1000));
	Payload->SetNumberField(TEXT("rebirthCount"), FMath::Max(0, SaveGame.RebirthCount));
	Payload->SetNumberField(TEXT("maxEquipmentGrade"), GetMaxEquipmentGrade(SaveGame));
	Payload->SetNumberField(TEXT("totalExp"), static_cast<double>(ComputeTotalExp(SaveGame)));
	Payload->SetNumberField(TEXT("gold"), static_cast<double>(FMath::Max<int64>(0, SaveGame.Gold)));
	Payload->SetNumberField(TEXT("lastSeenUnixSec"), static_cast<double>(FMath::Max<int64>(0, SaveGame.LastSeenUnixSec)));
	Payload->SetNumberField(TEXT("transcendCount"), FMath::Max(0, SaveGame.TranscendCount));
	Payload->SetNumberField(TEXT("towerHighestFloor"), FMath::Max(0, SaveGame.TowerHighestFloor));
	Payload->SetNumberField(TEXT("skillPoints"), FMath::Max(0, SaveGame.SkillPoints));
	const TArray<int32> MasteryLevels = ComputeMasteryLevels(SaveGame);
	int64 WorldPower = 0;
	TArray<TSharedPtr<FJsonValue>> MasteryLevelValues;
	MasteryLevelValues.Reserve(MasteryLevels.Num());
	for (const int32 Level : MasteryLevels)
	{
		const int32 SafeLevel = FMath::Max(0, Level);
		WorldPower += SafeLevel;
		MasteryLevelValues.Add(MakeShared<FJsonValueNumber>(SafeLevel));
	}
	Payload->SetNumberField(TEXT("worldPower"), static_cast<double>(WorldPower));
	Payload->SetArrayField(TEXT("masteryLevels"), MasteryLevelValues);

	TSharedRef<FJsonObject> FullSave = MakeShared<FJsonObject>();
	if (!FJsonObjectConverter::UStructToJsonObject(UIdleSaveGame::StaticClass(), &SaveGame, FullSave))
	{
		return false;
	}
	Payload->SetObjectField(FullSaveFieldName, FullSave);

	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutPayloadJson);
	return FJsonSerializer::Serialize(Payload, Writer);
}

bool FCloudSavePayloadMapper::PayloadJsonToSave(const FString& PayloadJson, UIdleSaveGame& OutSaveGame)
{
	TSharedPtr<FJsonObject> Payload;
	if (!DeserializePayload(PayloadJson, Payload))
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* FullSave = nullptr;
	if (Payload->TryGetObjectField(FullSaveFieldName, FullSave) && FullSave && FullSave->IsValid())
	{
		if (!FJsonObjectConverter::JsonObjectToUStruct((*FullSave).ToSharedRef(), UIdleSaveGame::StaticClass(), &OutSaveGame))
		{
			return false;
		}
	}

	OutSaveGame.bHasSave = true;
	OutSaveGame.SaveVersion = FMath::Max(OutSaveGame.SaveVersion, 2);
	OutSaveGame.CharacterLevel = FMath::Clamp(static_cast<int32>(Payload->GetNumberField(TEXT("level"))), 1, 1000);
	OutSaveGame.RebirthCount = FMath::Max(0, static_cast<int32>(Payload->GetNumberField(TEXT("rebirthCount"))));
	OutSaveGame.Gold = FMath::Max<int64>(0, static_cast<int64>(Payload->GetNumberField(TEXT("gold"))));
	OutSaveGame.LastSeenUnixSec = FMath::Max<int64>(0, static_cast<int64>(Payload->GetNumberField(TEXT("lastSeenUnixSec"))));

	double NumericValue = 0.0;
	if (Payload->TryGetNumberField(TEXT("transcendCount"), NumericValue))
	{
		OutSaveGame.TranscendCount = FMath::Max(0, static_cast<int32>(NumericValue));
	}
	if (Payload->TryGetNumberField(TEXT("towerHighestFloor"), NumericValue))
	{
		OutSaveGame.TowerHighestFloor = FMath::Max(0, static_cast<int32>(NumericValue));
	}
	if (Payload->TryGetNumberField(TEXT("skillPoints"), NumericValue))
	{
		OutSaveGame.SkillPoints = FMath::Max(0, static_cast<int32>(NumericValue));
	}

	return true;
}

bool FCloudSavePayloadMapper::ExtractSnapshot(const FString& PayloadJson, FCloudSaveProgressSnapshot& OutSnapshot)
{
	TSharedPtr<FJsonObject> Payload;
	if (!DeserializePayload(PayloadJson, Payload))
	{
		return false;
	}

	OutSnapshot.Level = FMath::Clamp(static_cast<int32>(Payload->GetNumberField(TEXT("level"))), 1, 1000);
	OutSnapshot.RebirthCount = FMath::Max(0, static_cast<int32>(Payload->GetNumberField(TEXT("rebirthCount"))));
	OutSnapshot.Gold = FMath::Max<int64>(0, static_cast<int64>(Payload->GetNumberField(TEXT("gold"))));
	OutSnapshot.LastSeenUnixSec = FMath::Max<int64>(0, static_cast<int64>(Payload->GetNumberField(TEXT("lastSeenUnixSec"))));
	return true;
}
