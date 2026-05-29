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

/**
 * 길드 레벨 기반 영구 버프(전 멤버 로컬 적용). 서버 `getGuildBuff(level)` parity —
 * 공격력 +0.4%*Lv, 골드획득 +0.4%*Lv. 비율(0.04 = +4%) 단위로 보관한다.
 */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildBuff
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	float AttackPct = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	float GoldPct = 0.0f;
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

/**
 * 길드 상점 1행(서버 `GET /:id/shop` items 항목). 서버 GUILD_SHOP_CATALOG parity —
 * 소비형 보상이며 가격은 개인 기여 포인트. 스냅샷이 아닌 별도 fetch 로 로드한다.
 */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FGuildShopItemInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString Id;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 Price = 0;
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

	// ── 길드 레벨/버프/기여(PR-G2, 서버 스냅샷 응답 파싱) ───────────────────────
	/** 길드 레벨(서버 권위 — guild.level). FGuildSummary.Level 과 동일 값. */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int32 GuildLevel = 1;

	/** 길드 누적 EXP(서버 bigint 문자열, int64). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 GuildExp = 0;

	/** 내 개인 기여 포인트(상점 화폐, 서버 me.contributionPoints). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 ContributionPoints = 0;

	/** 길드 레벨 기반 영구 버프(전 멤버 로컬 적용). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	FGuildBuff Buff;

	/** 오늘 출석 가능 여부(서버 me.canAttend). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	bool bCanAttendToday = false;

	/** 오늘 헌납 가능 여부(서버 me.donationRemaining > 0 파생). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	bool bCanDonateToday = false;

	/** 오늘 남은 헌납 가능 골드(서버 me.donationRemaining, 일일 상한 - 사용분). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 DonationRemaining = 0;

	/** 내 주간 기여 누적(서버 me.weeklyContribution, 주간 리셋). */
	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	int64 WeeklyContribution = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	TArray<FGuildMemberInfo> Members;

	UPROPERTY(BlueprintReadOnly, Category = "Idle|Guild")
	TArray<FGuildJoinRequestInfo> Requests;
};
