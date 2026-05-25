#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "UObject/Object.h"
#include "ApiClient.generated.h"

/**
 * REST API 호출을 담당할 최소 HTTP 클라이언트 골격입니다.
 * PR #4에서는 실제 인증 호출 없이 요청 생성과 완료 로그만 제공합니다.
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UApiClient : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FString& InBaseUrl);

	UFUNCTION(BlueprintCallable, Category = "Idle|Network")
	bool Get(const FString& Path);

	UFUNCTION(BlueprintCallable, Category = "Idle|Network")
	bool Post(const FString& Path, const FString& JsonBody);

	void RegisterGuest(TFunction<void(bool, FString)> Callback);
	bool RequestOfflinePreview(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount);
	bool ClaimOfflineRewards(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount);
	bool RequestQuestList(const FString& CharacterId);
	bool ReportQuestProgress(const FString& QuestId, const FString& CharacterId, int32 Amount);
	bool ClaimQuestReward(const FString& QuestId, const FString& CharacterId);

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetBaseUrl() const { return BaseUrl; }

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetAuthToken() const { return AuthToken; }

private:
	FString BuildUrl(const FString& Path) const;
	bool SendRequest(const FString& Verb, const FString& Path, const FString& JsonBody);
	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	UPROPERTY()
	FString BaseUrl = TEXT("http://localhost:3000");

	UPROPERTY()
	FString AuthToken;
};
