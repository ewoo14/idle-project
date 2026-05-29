#pragma once

#include "CoreMinimal.h"
#include "GameCore/LeaderboardTypes.h"
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
	void EnsureCharacter(TFunction<void(bool, FString)> Callback);
	void UploadSave(const FString& CharacterId, int32 Version, const FString& PayloadJson, TFunction<void(bool, FString)> Callback);
	void DownloadSave(const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void FetchLeaderboard(ELeaderboardKind Kind, int32 Season, TFunction<void(bool, FString)> Callback);
	void FetchMyRank(ELeaderboardKind Kind, int32 Season, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void FetchWeeklyDamageLeaderboard(const FString& Week, TFunction<void(bool, FString)> Callback);
	void FetchMyWeeklyRank(const FString& Week, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	bool RequestOfflinePreview(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount);
	bool ClaimOfflineRewards(int32 Level, int64 LastSeenUnixSec, int64 NowUnixSec, int32 RebirthCount);
	bool RequestQuestList(const FString& CharacterId);
	bool ReportQuestProgress(const FString& QuestId, const FString& CharacterId, int32 Amount);
	bool ClaimQuestReward(const FString& QuestId, const FString& CharacterId);
	bool RequestPetList();
	bool EquipPet(const FString& PetId);
	bool RequestSeasonState();
	bool ClaimSeasonReward(int32 Tier);

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetBaseUrl() const { return BaseUrl; }

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetAuthToken() const { return AuthToken; }

private:
	FString BuildUrl(const FString& Path) const;
	bool SendRequest(const FString& Verb, const FString& Path, const FString& JsonBody);
	bool SendRequestWithCallback(const FString& Verb, const FString& Path, const FString& JsonBody, TFunction<void(bool, FString)> Callback);
	void HandleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CreateCharacter(TFunction<void(bool, FString)> Callback);
	void VerifyCharacterOrCreate(const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void CacheCharacterId(const FString& CharacterId);
	FString LoadCachedCharacterId() const;

	UPROPERTY()
	FString BaseUrl = TEXT("http://localhost:3000");

	UPROPERTY()
	FString AuthToken;

	UPROPERTY()
	FString CachedCharacterId;
};
