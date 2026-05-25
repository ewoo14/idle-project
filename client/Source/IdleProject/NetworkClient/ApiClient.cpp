#include "NetworkClient/ApiClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

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
	if (CharacterId.IsEmpty())
	{
		return false;
	}

	const FString Path = FString::Printf(TEXT("/v1/quests?characterId=%s"), *CharacterId);
	return Get(Path);
}

bool UApiClient::ReportQuestProgress(const FString& QuestId, const FString& CharacterId, int32 Amount)
{
	if (QuestId.IsEmpty() || CharacterId.IsEmpty() || Amount <= 0)
	{
		return false;
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
	if (QuestId.IsEmpty() || CharacterId.IsEmpty())
	{
		return false;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetStringField(TEXT("characterId"), CharacterId);

	FString JsonBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonBody);
	FJsonSerializer::Serialize(JsonObject, Writer);

	return Post(FString::Printf(TEXT("/v1/quests/%s/claim"), *QuestId), JsonBody);
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

	if (Verb == TEXT("POST"))
	{
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		Request->SetContentAsString(JsonBody);
	}

	Request->OnProcessRequestComplete().BindUObject(this, &UApiClient::HandleResponse);
	return Request->ProcessRequest();
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
