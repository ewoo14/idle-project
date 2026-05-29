#include "NetworkClient/ApiClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool TryGetResponseDataObject(const TSharedPtr<FJsonObject>& ResponseJson, TSharedPtr<FJsonObject>& OutData)
{
	const TSharedPtr<FJsonObject>* DataObject = nullptr;
	if (ResponseJson.IsValid() && ResponseJson->TryGetObjectField(TEXT("data"), DataObject) && DataObject && DataObject->IsValid())
	{
		OutData = *DataObject;
		return true;
	}
	return false;
}

bool IsOkEnvelope(const TSharedPtr<FJsonObject>& ResponseJson)
{
	bool bOk = false;
	return ResponseJson.IsValid() && ResponseJson->TryGetBoolField(TEXT("ok"), bOk) && bOk;
}

FString SerializeJsonObject(const TSharedRef<FJsonObject>& JsonObject)
{
	FString JsonBody;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);
	return JsonBody;
}

const TCHAR* GetLeaderboardPathSegment(ELeaderboardKind Kind)
{
	return Kind == ELeaderboardKind::Power ? TEXT("power") : TEXT("rebirth");
}
}

void UApiClient::Initialize(const FString& InBaseUrl)
{
	BaseUrl = InBaseUrl.IsEmpty() ? TEXT("http://localhost:3000") : InBaseUrl;
	BaseUrl.RemoveFromEnd(TEXT("/"));
}

bool UApiClient::Get(const FString& Path)
{
	return SendRequest(TEXT("GET"), Path, FString());
}

bool UApiClient::Post(const FString& Path, const FString& JsonBody)
{
	return SendRequest(TEXT("POST"), Path, JsonBody);
}

void UApiClient::RegisterGuest(TFunction<void(bool, FString)> Callback)
{
	const FString GuestId = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString Email = FString::Printf(TEXT("guest_%s@idle.local"), *GuestId);
	const FString Password = FString::Printf(TEXT("Guest-%s!"), *GuestId.Left(12));

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("email"), Email);
	JsonObject->SetStringField(TEXT("password"), Password);
	JsonObject->SetStringField(TEXT("nickname"), FString::Printf(TEXT("Guest%s"), *GuestId.Left(8)));

	const FString JsonBody = SerializeJsonObject(JsonObject);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BuildUrl(TEXT("/v1/auth/register")));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(JsonBody);
	Request->SetTimeout(5.0f);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [this, Callback = MoveTemp(Callback)](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response, bool bWasSuccessful) mutable
	{
		const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
		const FString Body = Response.IsValid() ? Response->GetContentAsString() : FString();
		const bool bOk = bWasSuccessful && StatusCode >= 200 && StatusCode < 300;

		if (bOk)
		{
			TSharedPtr<FJsonObject> ResponseJson;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
			if (FJsonSerializer::Deserialize(Reader, ResponseJson) && ResponseJson.IsValid())
			{
				ResponseJson->TryGetStringField(TEXT("accessToken"), AuthToken);
				if (AuthToken.IsEmpty())
				{
					ResponseJson->TryGetStringField(TEXT("token"), AuthToken);
				}
				if (AuthToken.IsEmpty())
				{
					TSharedPtr<FJsonObject> DataJson;
					if (TryGetResponseDataObject(ResponseJson, DataJson))
					{
						DataJson->TryGetStringField(TEXT("accessToken"), AuthToken);
						if (AuthToken.IsEmpty())
						{
							DataJson->TryGetStringField(TEXT("token"), AuthToken);
						}
					}
				}
			}
		}

		const FString Message = bOk
			? FString::Printf(TEXT("registered status=%d"), StatusCode)
			: FString::Printf(TEXT("register skipped status=%d success=%s"), StatusCode, bWasSuccessful ? TEXT("true") : TEXT("false"));

		UE_LOG(LogTemp, Display, TEXT("[NetworkClient] guest %s"), *Message);
		if (Callback)
		{
			Callback(bOk, Message);
		}
	});

	if (!Request->ProcessRequest())
	{
		const FString Message = TEXT("register request could not start");
		UE_LOG(LogTemp, Warning, TEXT("[NetworkClient] guest %s"), *Message);
		if (Callback)
		{
			Callback(false, Message);
		}
	}
}

void UApiClient::EnsureCharacter(TFunction<void(bool, FString)> Callback)
{
	if (AuthToken.IsEmpty())
	{
		RegisterGuest([this, Callback = MoveTemp(Callback)](bool bSuccess, FString Message) mutable
		{
			if (!bSuccess)
			{
				if (Callback)
				{
					Callback(false, Message);
				}
				return;
			}

			EnsureCharacter(MoveTemp(Callback));
		});
		return;
	}

	if (CachedCharacterId.IsEmpty())
	{
		CachedCharacterId = LoadCachedCharacterId();
	}

	if (!CachedCharacterId.IsEmpty())
	{
		VerifyCharacterOrCreate(CachedCharacterId, MoveTemp(Callback));
		return;
	}

	CreateCharacter(MoveTemp(Callback));
}

void UApiClient::UploadSave(const FString& CharacterId, int32 Version, const FString& PayloadJson, TFunction<void(bool, FString)> Callback)
{
	if (CharacterId.IsEmpty() || Version <= 0 || PayloadJson.IsEmpty())
	{
		if (Callback)
		{
			Callback(false, TEXT("invalid save upload input"));
		}
		return;
	}

	TSharedPtr<FJsonObject> PayloadObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PayloadJson);
	if (!FJsonSerializer::Deserialize(Reader, PayloadObject) || !PayloadObject.IsValid())
	{
		if (Callback)
		{
			Callback(false, TEXT("invalid save payload json"));
		}
		return;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("characterId"), CharacterId);
	JsonObject->SetNumberField(TEXT("version"), Version);
	JsonObject->SetObjectField(TEXT("payload"), PayloadObject);

	SendRequestWithCallback(TEXT("PUT"), TEXT("/v1/save/"), SerializeJsonObject(JsonObject), MoveTemp(Callback));
}

void UApiClient::DownloadSave(const FString& CharacterId, TFunction<void(bool, FString)> Callback)
{
	if (CharacterId.IsEmpty())
	{
		if (Callback)
		{
			Callback(false, TEXT("invalid character id"));
		}
		return;
	}

	const FString Path = FString::Printf(TEXT("/v1/save/?characterId=%s"), *CharacterId);
	SendRequestWithCallback(TEXT("GET"), Path, FString(), [Callback = MoveTemp(Callback)](bool bSuccess, FString Body) mutable
	{
		if (!bSuccess)
		{
			if (Callback)
			{
				Callback(false, Body);
			}
			return;
		}

		TSharedPtr<FJsonObject> ResponseJson;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !IsOkEnvelope(ResponseJson))
		{
			if (Callback)
			{
				Callback(false, TEXT("invalid save response"));
			}
			return;
		}

		const TSharedPtr<FJsonValue> DataValue = ResponseJson->TryGetField(TEXT("data"));
		if (!DataValue.IsValid() || DataValue->Type == EJson::Null)
		{
			if (Callback)
			{
				Callback(true, FString());
			}
			return;
		}

		const TSharedPtr<FJsonObject> DataObject = DataValue->AsObject();
		const TSharedPtr<FJsonObject>* PayloadObject = nullptr;
		if (!DataObject.IsValid() || !DataObject->TryGetObjectField(TEXT("payload"), PayloadObject) || !PayloadObject || !PayloadObject->IsValid())
		{
			if (Callback)
			{
				Callback(false, TEXT("missing save payload"));
			}
			return;
		}

		if (Callback)
		{
			Callback(true, SerializeJsonObject((*PayloadObject).ToSharedRef()));
		}
	});
}

void UApiClient::FetchLeaderboard(ELeaderboardKind Kind, int32 Season, TFunction<void(bool, FString)> Callback)
{
	const FString Path = FString::Printf(
		TEXT("/v1/leaderboard/%s?season=%d&limit=100"),
		GetLeaderboardPathSegment(Kind),
		Season);
	SendRequestWithCallback(TEXT("GET"), Path, FString(), MoveTemp(Callback));
}

void UApiClient::FetchMyRank(ELeaderboardKind Kind, int32 Season, const FString& CharacterId, TFunction<void(bool, FString)> Callback)
{
	if (CharacterId.IsEmpty())
	{
		if (Callback)
		{
			Callback(false, TEXT("invalid character id"));
		}
		return;
	}

	const FString Path = FString::Printf(
		TEXT("/v1/leaderboard/%s/me?season=%d&characterId=%s"),
		GetLeaderboardPathSegment(Kind),
		Season,
		*CharacterId);
	SendRequestWithCallback(TEXT("GET"), Path, FString(), MoveTemp(Callback));
}

bool UApiClient::RequestOfflinePreview(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount)
{
	const FString Path = FString::Printf(
		TEXT("/v1/offline/preview?level=%d&lastSeenUnixSec=%lld&nowUnixSec=%lld&rebirthCount=%d"),
		Level,
		LastSeenUnixSec,
		NowUnixSec,
		RebirthCount);
	return Get(Path);
}

bool UApiClient::ClaimOfflineRewards(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("level"), Level);
	JsonObject->SetNumberField(TEXT("lastSeenUnixSec"), static_cast<double>(LastSeenUnixSec));
	JsonObject->SetNumberField(TEXT("nowUnixSec"), static_cast<double>(NowUnixSec));
	JsonObject->SetNumberField(TEXT("rebirthCount"), RebirthCount);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(TEXT("/v1/offline/claim"), JsonBody);
}

bool UApiClient::RequestQuestList(const FString& CharacterId)
{
	FString ResolvedCharacterId = CharacterId;
	if (ResolvedCharacterId.IsEmpty())
	{
		if (CachedCharacterId.IsEmpty())
		{
			CachedCharacterId = LoadCachedCharacterId();
		}
		ResolvedCharacterId = CachedCharacterId;
	}
	if (ResolvedCharacterId.IsEmpty() || AuthToken.IsEmpty())
	{
		EnsureCharacter([this](bool bSuccess, FString ResolvedCharacterId)
		{
			if (bSuccess)
			{
				RequestQuestList(ResolvedCharacterId);
			}
		});
		return true;
	}

	const FString Path = FString::Printf(TEXT("/v1/quests?characterId=%s"), *ResolvedCharacterId);
	return Get(Path);
}

bool UApiClient::ReportQuestProgress(const FString& QuestId, const FString& CharacterId, int32 Amount)
{
	if (QuestId.IsEmpty() || Amount <= 0)
	{
		return false;
	}
	if (CharacterId.IsEmpty())
	{
		EnsureCharacter([this, QuestId, Amount](bool bSuccess, FString ResolvedCharacterId)
		{
			if (bSuccess)
			{
				ReportQuestProgress(QuestId, ResolvedCharacterId, Amount);
			}
		});
		return true;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("characterId"), CharacterId);
	JsonObject->SetNumberField(TEXT("amount"), Amount);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(FString::Printf(TEXT("/v1/quests/%s/progress"), *QuestId), JsonBody);
}

bool UApiClient::ClaimQuestReward(const FString& QuestId, const FString& CharacterId)
{
	if (QuestId.IsEmpty())
	{
		return false;
	}
	if (CharacterId.IsEmpty())
	{
		EnsureCharacter([this, QuestId](bool bSuccess, FString ResolvedCharacterId)
		{
			if (bSuccess)
			{
				ClaimQuestReward(QuestId, ResolvedCharacterId);
			}
		});
		return true;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("characterId"), CharacterId);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(FString::Printf(TEXT("/v1/quests/%s/claim"), *QuestId), JsonBody);
}

bool UApiClient::RequestPetList()
{
	if (AuthToken.IsEmpty())
	{
		RegisterGuest([this](bool bSuccess, FString)
		{
			if (bSuccess)
			{
				RequestPetList();
			}
		});
		return true;
	}
	return Get(TEXT("/v1/pets"));
}

bool UApiClient::EquipPet(const FString& PetId)
{
	if (PetId.IsEmpty())
	{
		return false;
	}
	if (AuthToken.IsEmpty())
	{
		RegisterGuest([this, PetId](bool bSuccess, FString)
		{
			if (bSuccess)
			{
				EquipPet(PetId);
			}
		});
		return true;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("petId"), PetId);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(FString::Printf(TEXT("/v1/pets/%s/equip"), *PetId), JsonBody);
}

bool UApiClient::RequestSeasonState()
{
	if (AuthToken.IsEmpty())
	{
		RegisterGuest([this](bool bSuccess, FString)
		{
			if (bSuccess)
			{
				RequestSeasonState();
			}
		});
		return true;
	}
	return Get(TEXT("/v1/season"));
}

bool UApiClient::ClaimSeasonReward(int32 Tier)
{
	if (Tier <= 0)
	{
		return false;
	}
	if (CachedCharacterId.IsEmpty())
	{
		CachedCharacterId = LoadCachedCharacterId();
	}
	if (CachedCharacterId.IsEmpty() || AuthToken.IsEmpty())
	{
		EnsureCharacter([this, Tier](bool bSuccess, FString)
		{
			if (bSuccess)
			{
				ClaimSeasonReward(Tier);
			}
		});
		return true;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("tier"), Tier);
	JsonObject->SetStringField(TEXT("characterId"), CachedCharacterId);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(FString::Printf(TEXT("/v1/season/tiers/%d/claim"), Tier), JsonBody);
}

FString UApiClient::BuildUrl(const FString& Path) const
{
	if (Path.StartsWith(TEXT("http://")) || Path.StartsWith(TEXT("https://")))
	{
		return Path;
	}

	FString CleanPath = Path.TrimStartAndEnd();
	while (CleanPath.StartsWith(TEXT("/")))
	{
		CleanPath.RightChopInline(1, EAllowShrinking::No);
	}

	return FString::Printf(TEXT("%s/%s"), *BaseUrl, *CleanPath);
}

bool UApiClient::SendRequest(const FString& Verb, const FString& Path, const FString& JsonBody)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BuildUrl(Path));
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	if (!AuthToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}

	if (Verb == TEXT("POST") || Verb == TEXT("PUT"))
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(JsonBody);
	}

	Request->OnProcessRequestComplete().BindUObject(this, &UApiClient::HandleResponse);
	return Request->ProcessRequest();
}

bool UApiClient::SendRequestWithCallback(const FString& Verb, const FString& Path, const FString& JsonBody, TFunction<void(bool, FString)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(BuildUrl(Path));
	Request->SetVerb(Verb);
	Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
	if (!AuthToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AuthToken));
	}

	if (Verb == TEXT("POST") || Verb == TEXT("PUT"))
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(JsonBody);
	}

	Request->SetTimeout(5.0f);
	Request->OnProcessRequestComplete().BindWeakLambda(this, [Callback = MoveTemp(Callback)](FHttpRequestPtr RequestPtr, FHttpResponsePtr Response, bool bWasSuccessful) mutable
	{
		const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
		const FString Body = Response.IsValid() ? Response->GetContentAsString() : FString();
		const bool bOk = bWasSuccessful && StatusCode >= 200 && StatusCode < 300;
		UE_LOG(LogTemp, Display, TEXT("ApiClient %s %s -> %d success=%s"),
			RequestPtr.IsValid() ? *RequestPtr->GetVerb() : TEXT("UNKNOWN"),
			RequestPtr.IsValid() ? *RequestPtr->GetURL() : TEXT("UNKNOWN"),
			StatusCode,
			bWasSuccessful ? TEXT("true") : TEXT("false"));

		if (Callback)
		{
			Callback(bOk, Body);
		}
	});

	if (!Request->ProcessRequest())
	{
		if (Callback)
		{
			Callback(false, TEXT("request could not start"));
		}
		return false;
	}
	return true;
}

void UApiClient::HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const int32 StatusCode = Response.IsValid() ? Response->GetResponseCode() : 0;
	UE_LOG(LogTemp, Display, TEXT("ApiClient %s %s -> %d success=%s"),
		Request.IsValid() ? *Request->GetVerb() : TEXT("UNKNOWN"),
		Request.IsValid() ? *Request->GetURL() : TEXT("UNKNOWN"),
		StatusCode,
		bWasSuccessful ? TEXT("true") : TEXT("false"));
}

void UApiClient::CreateCharacter(TFunction<void(bool, FString)> Callback)
{
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("classId"), 1);

	SendRequestWithCallback(TEXT("POST"), TEXT("/v1/characters/"), SerializeJsonObject(JsonObject), [this, Callback = MoveTemp(Callback)](bool bSuccess, FString Body) mutable
	{
		if (!bSuccess)
		{
			if (Callback)
			{
				Callback(false, Body);
			}
			return;
		}

		TSharedPtr<FJsonObject> ResponseJson;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
		TSharedPtr<FJsonObject> DataJson;
		FString CharacterId;
		if (FJsonSerializer::Deserialize(Reader, ResponseJson)
			&& IsOkEnvelope(ResponseJson)
			&& TryGetResponseDataObject(ResponseJson, DataJson)
			&& DataJson->TryGetStringField(TEXT("id"), CharacterId)
			&& !CharacterId.IsEmpty())
		{
			CacheCharacterId(CharacterId);
			if (Callback)
			{
				Callback(true, CharacterId);
			}
			return;
		}

		if (Callback)
		{
			Callback(false, TEXT("invalid character response"));
		}
	});
}

void UApiClient::VerifyCharacterOrCreate(const FString& CharacterId, TFunction<void(bool, FString)> Callback)
{
	SendRequestWithCallback(TEXT("GET"), FString::Printf(TEXT("/v1/characters/%s"), *CharacterId), FString(), [this, CharacterId, Callback = MoveTemp(Callback)](bool bSuccess, FString Body) mutable
	{
		if (bSuccess)
		{
			TSharedPtr<FJsonObject> ResponseJson;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
			if (FJsonSerializer::Deserialize(Reader, ResponseJson) && IsOkEnvelope(ResponseJson))
			{
				if (Callback)
				{
					Callback(true, CharacterId);
				}
				return;
			}
		}

		CreateCharacter(MoveTemp(Callback));
	});
}

void UApiClient::CacheCharacterId(const FString& CharacterId)
{
	CachedCharacterId = CharacterId;
	if (!GConfig || CharacterId.IsEmpty())
	{
		return;
	}

	GConfig->SetString(
		TEXT("/Script/IdleProject.ApiClient"),
		TEXT("CloudCharacterId"),
		*CharacterId,
		GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

FString UApiClient::LoadCachedCharacterId() const
{
	if (!GConfig)
	{
		return FString();
	}

	FString CharacterId;
	GConfig->GetString(
		TEXT("/Script/IdleProject.ApiClient"),
		TEXT("CloudCharacterId"),
		CharacterId,
		GGameUserSettingsIni);
	return CharacterId;
}
