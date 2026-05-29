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

	// ── 길드 `/v1/guilds` (콜백형, {ok,data} 래퍼) — 서버 권위 멤버십 ────────────
	void CreateGuild(const FString& CharacterId, const FString& Name, TFunction<void(bool, FString)> Callback);
	void ListGuilds(const FString& Query, int32 Limit, int32 Offset, TFunction<void(bool, FString)> Callback);
	void GetGuild(const FString& GuildId, TFunction<void(bool, FString)> Callback);
	void GetMyGuild(const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void JoinGuild(const FString& GuildId, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void LeaveGuild(const FString& GuildId, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void ApproveRequest(const FString& GuildId, const FString& TargetCharacterId, const FString& ActorCharacterId, TFunction<void(bool, FString)> Callback);
	void RejectRequest(const FString& GuildId, const FString& TargetCharacterId, const FString& ActorCharacterId, TFunction<void(bool, FString)> Callback);
	void SetMemberRank(const FString& GuildId, const FString& TargetCharacterId, const FString& ActorCharacterId, const FString& Rank, TFunction<void(bool, FString)> Callback);
	void UpdateGuildSettings(const FString& GuildId, const FString& ActorCharacterId, const FString& Name, const FString& Notice, const FString& JoinMode, TFunction<void(bool, FString)> Callback);

	// ── 길드 기여/상점 `/v1/guilds/:id/...` (PR-G2, {ok,data} 래퍼) ───────────────
	void GuildAttendance(const FString& GuildId, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void GuildDonate(const FString& GuildId, const FString& CharacterId, int64 Gold, TFunction<void(bool, FString)> Callback);
	void GuildContribute(const FString& GuildId, const FString& CharacterId, int64 Amount, TFunction<void(bool, FString)> Callback);
	void GetGuildShop(const FString& GuildId, const FString& CharacterId, TFunction<void(bool, FString)> Callback);
	void BuyGuildShopItem(const FString& GuildId, const FString& CharacterId, const FString& ItemId, TFunction<void(bool, FString)> Callback);
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
