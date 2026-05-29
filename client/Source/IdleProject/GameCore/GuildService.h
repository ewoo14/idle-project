#pragma once

#include "CoreMinimal.h"
#include "GameCore/GuildTypes.h"
#include "UObject/Object.h"
#include "GuildService.generated.h"

/**
 * 길드 상태 캐시 서비스. 서버 권위 — 로컬은 `GET /v1/guilds/me` 스냅샷을 보관만 한다.
 * 멤버십 변경 로직은 여기서 구현하지 않으며(서버 트랜잭션), 접근자/세이브 캐시만 제공한다.
 * WeeklyBossService 와 동일한 UObject 서비스 패턴을 따른다.
 */
UCLASS(BlueprintType)
class IDLEPROJECT_API UGuildService : public UObject
{
	GENERATED_BODY()

public:
	/** 서버 스냅샷을 캐시에 반영(서버 권위 — 무조건 덮어쓴다). */
	UFUNCTION(BlueprintCallable, Category = "Idle|Guild")
	void ApplySnapshot(const FGuildSnapshot& Snapshot);

	/**
	 * `{ok,data}` 래퍼 응답 본문을 파싱해 캐시에 반영한다.
	 * 파싱 실패/무소속이면 캐시를 비우고 false 반환(데이터 없음 포함).
	 */
	UFUNCTION(BlueprintCallable, Category = "Idle|Guild")
	bool ParseSnapshotJson(const FString& JsonBody);

	/** 캐시를 무소속 상태로 초기화. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Guild")
	void ClearSnapshot();

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	bool HasGuild() const { return CachedSnapshot.bHasGuild; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	EGuildRank GetMyRank() const { return CachedSnapshot.MyRank; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	const TArray<FGuildMemberInfo>& GetMembers() const { return CachedSnapshot.Members; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	const TArray<FGuildJoinRequestInfo>& GetRequests() const { return CachedSnapshot.Requests; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	FGuildSummary GetGuildSummary() const { return CachedSnapshot.Guild; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	const FGuildSnapshot& GetSnapshot() const { return CachedSnapshot; }

	/** 세이브 캐시용 길드 id(무소속이면 빈 문자열). */
	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	FString GetCachedGuildId() const { return CachedSnapshot.bHasGuild ? CachedSnapshot.Guild.Id : FString(); }

	// ── 세이브 캐시(SaveVer 17) 라운드트립 ──────────────────────────────────────
	/** 세이브에 저장할 최소 캐시(길드 id / 내 rank). */
	void ExportSave(FString& OutGuildId, uint8& OutRank) const;
	/** 세이브에서 최소 캐시를 복원(스텁 — 서버 재동기화 전까지의 표시용). */
	void ImportSave(const FString& InGuildId, uint8 InRank);

private:
	UPROPERTY()
	FGuildSnapshot CachedSnapshot;
};
