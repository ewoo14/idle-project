#include "NetworkClient/ApiClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

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
