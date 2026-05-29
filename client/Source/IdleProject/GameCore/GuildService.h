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

	// ── 길드 레벨/버프/기여(PR-G2) ──────────────────────────────────────────────
	/** 길드 레벨 기반 영구 버프(공격/골드). 오프라인에도 캐시 적용 — 단일 소스. */
	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	FGuildBuff GetGuildBuff() const { return CachedBuff; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int32 GetGuildLevel() const { return CachedLevel; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int64 GetContributionPoints() const { return CachedContributionPoints; }

	// ── 길드 보스 상태 접근자(PR-G3, 서버 권위 — 스냅샷 캐시 표시용) ──────────────
	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int64 GetBossHp() const { return CachedSnapshot.BossHp; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int64 GetBossAccumDamage() const { return CachedSnapshot.BossAccumDamage; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int32 GetBossDefeatedCount() const { return CachedSnapshot.BossDefeatedCount; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int32 GetBossChallengesRemaining() const { return CachedSnapshot.BossChallengesRemaining; }

	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int32 GetBossUnclaimedDefeats() const { return CachedSnapshot.BossUnclaimedDefeats; }

	/**
	 * 던전 클리어·보스 도전 등 자동 기여 델타를 로컬에 누적. 다음 플러시(Contribute)
	 * 까지 보관한다. 음수/0 은 무시. 주간 상한 클램프는 플러시 시 적용.
	 */
	void AddPendingAutoContribution(int64 Delta);

	/**
	 * 누적 델타를 소비해 반환(서버 contribute 로 플러시할 양). 주간 자동 상한
	 * (AUTO_WEEKLY_CAP)으로 클램프하며, 소비한 만큼 pending 을 차감한다.
	 */
	int64 ConsumePendingAutoContribution();

	/** 현재 누적된(미플러시) 자동 기여 델타. */
	UFUNCTION(BlueprintPure, Category = "Idle|Guild")
	int64 GetPendingAutoContribution() const { return PendingAutoContribution; }

	// ── 세이브 캐시(SaveVer 19) 라운드트립 ──────────────────────────────────────
	/**
	 * 세이브에 저장할 캐시(길드 id / 내 rank / 레벨·버프·포인트·pending·출석일
	 * + 보스 진행 표시 캐시[격파 횟수·내 도전 잔여], SaveVer 19).
	 * 보스 진행은 서버 권위이며 여기 캐시는 재접속 직후 UI 표시 목적의 표시용이다.
	 */
	void ExportSave(
		FString& OutGuildId,
		uint8& OutRank,
		int32& OutLevel,
		float& OutAttackPct,
		float& OutGoldPct,
		int64& OutContributionPoints,
		int64& OutPendingAutoContribution,
		FString& OutLastAttendanceDate,
		int32& OutBossDefeatedCount,
		int32& OutBossChallengesRemaining) const;
	/** 세이브에서 캐시를 복원(버프는 오프라인에도 적용되도록 캐시 직접 복원). */
	void ImportSave(
		const FString& InGuildId,
		uint8 InRank,
		int32 InLevel,
		float InAttackPct,
		float InGoldPct,
		int64 InContributionPoints,
		int64 InPendingAutoContribution,
		const FString& InLastAttendanceDate,
		int32 InBossDefeatedCount,
		int32 InBossChallengesRemaining);

private:
	UPROPERTY()
	FGuildSnapshot CachedSnapshot;

	/** 레벨 기반 영구 버프 캐시(스냅샷·세이브 복원 시 갱신). */
	UPROPERTY()
	FGuildBuff CachedBuff;

	UPROPERTY()
	int32 CachedLevel = 1;

	UPROPERTY()
	int64 CachedContributionPoints = 0;

	/** 미플러시 자동 기여 델타(세이브 영속, 재접속 시 서버로 플러시). */
	UPROPERTY()
	int64 PendingAutoContribution = 0;

	/** 마지막 출석 UTC 날짜 캐시(UI 표시·중복 출석 가드용). */
	UPROPERTY()
	FString LastAttendanceDate;
};
