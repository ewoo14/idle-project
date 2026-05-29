#pragma once

#include "CoreMinimal.h"
#include "GuildTypes.generated.h"

/**
 * 길드 계급 — 서버(`guild.service.ts`)의 GuildRank 와 1:1 parity.
 * master/member 는 항상 사용 가능, vice/officer 는 인원 기반 해금(GuildFormula).
 */
UENUM(BlueprintType)
enum class EGuildRank : uint8
{
	Master,
	Vice,
	Officer,
	Member
};

/** 길드 가입 모드 — 서버 GuildJoinMode 와 parity(open=자유, approval=승인). */
UENUM(BlueprintType)
enum class EGuildJoinMode : uint8
{
	Open,
	Approval
};

/** 길드 멤버 1인 요약(서버 스냅샷 members 항목). 서버 권위 — 로컬은 캐시만 보관. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildMemberInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString Nickname;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	EGuildRank Rank = EGuildRank::Member;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 Contribution = 0;
};

/** 길드 공개 요약(서버 toGuildResponse). */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildSummary
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int32 Level = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int32 MemberCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	EGuildJoinMode JoinMode = EGuildJoinMode::Open;
};

/** 길드 가입 신청 1건(승인제, 권한자에게만 노출). */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildJoinRequestInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString CharacterId;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString RequestedAt;
};

/**
 * 내 길드 스냅샷(서버 `GET /v1/guilds/me` 응답의 클라 캐시 표현).
 * bHasGuild=false 면 무소속이며 나머지 필드는 무의미하다.
 */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	bool bHasGuild = false;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	EGuildRank MyRank = EGuildRank::Member;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FGuildSummary Guild;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	TArray<FGuildMemberInfo> Members;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	TArray<FGuildJoinRequestInfo> Requests;
};
