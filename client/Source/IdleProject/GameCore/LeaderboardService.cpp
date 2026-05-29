#include "GameCore/LeaderboardService.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool DeserializeObject(const FString& JsonBody, TSharedPtr<FJsonObject>& OutObject)
{
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	return FJsonSerializer::Deserialize(Reader, OutObject) && OutObject.IsValid();
}

bool IsOkEnvelope(const TSharedPtr<FJsonObject>& JsonObject)
{
	bool bOk = false;
	return JsonObject.IsValid() && JsonObject->TryGetBoolField(TEXT("ok"), bOk) && bOk;
}

bool TryGetInt64Field(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName, int64& OutValue)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	FString StringValue;
	if (JsonObject->TryGetStringField(FieldName, StringValue))
	{
		return LexTryParseString(OutValue, *StringValue);
	}

	const TSharedPtr<FJsonValue> Value = JsonObject->TryGetField(FieldName);
	if (!Value.IsValid() || Value->Type != EJson::Number)
	{
		return false;
	}

	OutValue = static_cast<int64>(Value->AsNumber());
	return true;
}

FLeaderboardEntry ParseEntryObject(const TSharedPtr<FJsonObject>& EntryObject)
{
	FLeaderboardEntry Entry;
	if (!EntryObject.IsValid())
	{
		return Entry;
	}

	EntryObject->TryGetStringField(TEXT("characterId"), Entry.CharacterId);
	TryGetInt64Field(EntryObject, TEXT("score"), Entry.Score);
	int64 Rank = 0;
	if (TryGetInt64Field(EntryObject, TEXT("rank"), Rank))
	{
		Entry.Rank = static_cast<int32>(FMath::Clamp<int64>(Rank, 0, MAX_int32));
	}
	return Entry;
}
}

TArray<FLeaderboardEntry> ULeaderboardService::ParseListJson(const FString& JsonBody, ELeaderboardKind Kind)
{
	TArray<FLeaderboardEntry> ParsedEntries;

	TSharedPtr<FJsonObject> ResponseJson;
	const TArray<TSharedPtr<FJsonValue>>* DataArray = nullptr;
	if (DeserializeObject(JsonBody, ResponseJson)
		&& IsOkEnvelope(ResponseJson)
		&& ResponseJson->TryGetArrayField(TEXT("data"), DataArray)
		&& DataArray)
	{
		for (const TSharedPtr<FJsonValue>& Value : *DataArray)
		{
			const FLeaderboardEntry Entry = ParseEntryObject(Value.IsValid() ? Value->AsObject() : nullptr);
			if (!Entry.CharacterId.IsEmpty())
			{
				ParsedEntries.Add(Entry);
			}
		}
	}

	if (Kind == ELeaderboardKind::Power)
	{
		PowerEntries = ParsedEntries;
	}
	else
	{
		RebirthEntries = ParsedEntries;
	}

	return ParsedEntries;
}

FLeaderboardEntry ULeaderboardService::ParseMyRankJson(const FString& JsonBody, ELeaderboardKind Kind)
{
	FLeaderboardEntry ParsedEntry;

	TSharedPtr<FJsonObject> ResponseJson;
	const TSharedPtr<FJsonObject>* DataObject = nullptr;
	if (DeserializeObject(JsonBody, ResponseJson)
		&& IsOkEnvelope(ResponseJson)
		&& ResponseJson->TryGetObjectField(TEXT("data"), DataObject)
		&& DataObject
		&& DataObject->IsValid())
	{
		ParsedEntry = ParseEntryObject(*DataObject);
	}

	if (Kind == ELeaderboardKind::Power)
	{
		PowerMyEntry = ParsedEntry;
	}
	else
	{
		RebirthMyEntry = ParsedEntry;
	}

	return ParsedEntry;
}

TArray<FLeaderboardEntry> ULeaderboardService::GetEntries(ELeaderboardKind Kind) const
{
	return Kind == ELeaderboardKind::Power ? PowerEntries : RebirthEntries;
}

FLeaderboardEntry ULeaderboardService::GetMyEntry(ELeaderboardKind Kind) const
{
	return Kind == ELeaderboardKind::Power ? PowerMyEntry : RebirthMyEntry;
}
