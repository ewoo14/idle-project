#include "GameCore/GuildService.h"

#include "GameCore/GuildFormula.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
// jumbo(unity) 빌드 ODR 회피용 고유 prefix(Guild~) 헬퍼.
EGuildRank GuildParseRank(const FString& RankString)
{
	if (RankString == TEXT("master"))
	{
		return EGuildRank::Master;
	}
	if (RankString == TEXT("vice"))
	{
		return EGuildRank::Vice;
	}
	if (RankString == TEXT("officer"))
	{
		return EGuildRank::Officer;
	}
	return EGuildRank::Member;
}

EGuildJoinMode GuildParseJoinMode(const FString& JoinModeString)
{
	return JoinModeString == TEXT("approval") ? EGuildJoinMode::Approval : EGuildJoinMode::Open;
}

EGuildRank GuildRankFromByte(uint8 RankByte)
{
	switch (RankByte)
	{
	case static_cast<uint8>(EGuildRank::Master):
		return EGuildRank::Master;
	case static_cast<uint8>(EGuildRank::Vice):
		return EGuildRank::Vice;
	case static_cast<uint8>(EGuildRank::Officer):
		return EGuildRank::Officer;
	default:
		return EGuildRank::Member;
	}
}

/** 서버는 bigint 를 문자열로 직렬화하므로 숫자/문자열 모두 허용. */
int64 GuildParseInt64Field(const TSharedPtr<FJsonObject>& Object, const TCHAR* FieldName)
{
	if (!Object.IsValid())
	{
		return 0;
	}

	FString StringValue;
	if (Object->TryGetStringField(FieldName, StringValue))
	{
		return FCString::Atoi64(*StringValue);
	}

	double NumberValue = 0.0;
	if (Object->TryGetNumberField(FieldName, NumberValue))
	{
		return static_cast<int64>(NumberValue);
	}
	return 0;
}
}

void UGuildService::ApplySnapshot(const FGuildSnapshot& Snapshot)
{
	CachedSnapshot = Snapshot;

	if (CachedSnapshot.bHasGuild)
	{
		// 레벨은 누적 exp 로 재계산해 서버 권위와 항상 일치(스냅샷 denorm 무시).
		// exp 가 0(직접 ApplySnapshot 호출 등)이면 스냅샷이 전달한 레벨을 신뢰.
		const int32 LevelFromExp = FGuildFormula::GetGuildLevel(CachedSnapshot.GuildExp);
		CachedLevel = CachedSnapshot.GuildExp > 0
			? LevelFromExp
			: FMath::Max(1, CachedSnapshot.GuildLevel);
		CachedSnapshot.GuildLevel = CachedLevel;
		CachedSnapshot.Guild.Level = CachedLevel;
		// 버프는 레벨에서 단일 공식으로 파생(서버 getGuildBuff parity).
		CachedBuff = FGuildFormula::GetGuildBuff(CachedLevel);
		CachedSnapshot.Buff = CachedBuff;
		CachedContributionPoints = FMath::Max<int64>(0, CachedSnapshot.ContributionPoints);
	}
	else
	{
		CachedLevel = 1;
		CachedBuff = FGuildBuff();
		CachedContributionPoints = 0;
	}
}

void UGuildService::ClearSnapshot()
{
	CachedSnapshot = FGuildSnapshot();
	CachedBuff = FGuildBuff();
	CachedLevel = 1;
	CachedContributionPoints = 0;
	// PendingAutoContribution/LastAttendanceDate 는 멤버십 캐시와 독립(세이브 영속)이라 유지.
}

bool UGuildService::ParseSnapshotJson(const FString& JsonBody)
{
	ClearSnapshot();

	if (JsonBody.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FJsonObject> ResponseJson;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonBody);
	if (!FJsonSerializer::Deserialize(Reader, ResponseJson) || !ResponseJson.IsValid())
	{
		return false;
	}

	bool bOk = false;
	if (!ResponseJson->TryGetBoolField(TEXT("ok"), bOk) || !bOk)
	{
		return false;
	}

	const TSharedPtr<FJsonObject>* DataObjectPtr = nullptr;
	if (!ResponseJson->TryGetObjectField(TEXT("data"), DataObjectPtr) || !DataObjectPtr || !DataObjectPtr->IsValid())
	{
		return false;
	}
	const TSharedPtr<FJsonObject>& Data = *DataObjectPtr;

	// guild 가 null 이면 무소속.
	const TSharedPtr<FJsonObject>* GuildObjectPtr = nullptr;
	if (!Data->TryGetObjectField(TEXT("guild"), GuildObjectPtr) || !GuildObjectPtr || !GuildObjectPtr->IsValid())
	{
		return false;
	}
	const TSharedPtr<FJsonObject>& GuildObject = *GuildObjectPtr;

	FGuildSnapshot Snapshot;
	Snapshot.bHasGuild = true;

	GuildObject->TryGetStringField(TEXT("id"), Snapshot.Guild.Id);
	GuildObject->TryGetStringField(TEXT("name"), Snapshot.Guild.Name);

	int32 Level = 1;
	if (GuildObject->TryGetNumberField(TEXT("level"), Level))
	{
		Snapshot.Guild.Level = FMath::Max(1, Level);
	}
	int32 MemberCount = 0;
	if (GuildObject->TryGetNumberField(TEXT("memberCount"), MemberCount))
	{
		Snapshot.Guild.MemberCount = FMath::Max(0, MemberCount);
	}
	FString JoinModeString;
	if (GuildObject->TryGetStringField(TEXT("joinMode"), JoinModeString))
	{
		Snapshot.Guild.JoinMode = GuildParseJoinMode(JoinModeString);
	}
	// 서버는 guild.exp 를 bigint 문자열로 직렬화 → 레벨·버프 파생 소스.
	Snapshot.GuildExp = FMath::Max<int64>(0, GuildParseInt64Field(GuildObject, TEXT("exp")));
	Snapshot.GuildLevel = Snapshot.Guild.Level;

	// 내 멤버십(me) — rank / 기여 포인트 / 출석·헌납 가능 여부.
	const TSharedPtr<FJsonObject>* MeObjectPtr = nullptr;
	if (Data->TryGetObjectField(TEXT("me"), MeObjectPtr) && MeObjectPtr && MeObjectPtr->IsValid())
	{
		const TSharedPtr<FJsonObject>& MeObject = *MeObjectPtr;
		FString MyRankString;
		if (MeObject->TryGetStringField(TEXT("rank"), MyRankString))
		{
			Snapshot.MyRank = GuildParseRank(MyRankString);
		}
		Snapshot.ContributionPoints = FMath::Max<int64>(0, GuildParseInt64Field(MeObject, TEXT("contributionPoints")));
		Snapshot.WeeklyContribution = FMath::Max<int64>(0, GuildParseInt64Field(MeObject, TEXT("weeklyContribution")));
		MeObject->TryGetBoolField(TEXT("canAttend"), Snapshot.bCanAttendToday);
		// 서버는 donationRemaining(bigint 문자열)을 주므로 >0 이면 헌납 가능.
		Snapshot.DonationRemaining = FMath::Max<int64>(0, GuildParseInt64Field(MeObject, TEXT("donationRemaining")));
		Snapshot.bCanDonateToday = Snapshot.DonationRemaining > 0;
	}

	// 멤버 목록.
	const TArray<TSharedPtr<FJsonValue>>* MemberArray = nullptr;
	if (Data->TryGetArrayField(TEXT("members"), MemberArray) && MemberArray)
	{
		Snapshot.Members.Reserve(MemberArray->Num());
		for (const TSharedPtr<FJsonValue>& Value : *MemberArray)
		{
			const TSharedPtr<FJsonObject> MemberObject = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!MemberObject.IsValid())
			{
				continue;
			}

			FGuildMemberInfo Info;
			MemberObject->TryGetStringField(TEXT("characterId"), Info.CharacterId);
			MemberObject->TryGetStringField(TEXT("nickname"), Info.Nickname);
			FString MemberRankString;
			if (MemberObject->TryGetStringField(TEXT("rank"), MemberRankString))
			{
				Info.Rank = GuildParseRank(MemberRankString);
			}
			// 서버는 totalContribution 을 bigint 문자열로 직렬화.
			Info.Contribution = FMath::Max<int64>(0, GuildParseInt64Field(MemberObject, TEXT("totalContribution")));
			Snapshot.Members.Add(MoveTemp(Info));
		}
	}

	// 길드 보스 상태(서버 snapshot.boss — 무소속/미생성 시 누락 가능).
	const TSharedPtr<FJsonObject>* BossObjectPtr = nullptr;
	if (Data->TryGetObjectField(TEXT("boss"), BossObjectPtr) && BossObjectPtr && BossObjectPtr->IsValid())
	{
		const TSharedPtr<FJsonObject>& BossObject = *BossObjectPtr;
		Snapshot.BossHp = FMath::Max<int64>(0, GuildParseInt64Field(BossObject, TEXT("hp")));
		Snapshot.BossAccumDamage = FMath::Max<int64>(0, GuildParseInt64Field(BossObject, TEXT("accumDamage")));
		int32 BossDefeated = 0;
		if (BossObject->TryGetNumberField(TEXT("defeatedCount"), BossDefeated))
		{
			Snapshot.BossDefeatedCount = FMath::Max(0, BossDefeated);
		}
		int32 BossChallenges = 0;
		if (BossObject->TryGetNumberField(TEXT("challengesRemaining"), BossChallenges))
		{
			Snapshot.BossChallengesRemaining = FMath::Max(0, BossChallenges);
		}
		int32 BossUnclaimed = 0;
		if (BossObject->TryGetNumberField(TEXT("unclaimedDefeats"), BossUnclaimed))
		{
			Snapshot.BossUnclaimedDefeats = FMath::Max(0, BossUnclaimed);
		}
	}

	// 가입 신청(권한자에게만 채워짐).
	const TArray<TSharedPtr<FJsonValue>>* RequestArray = nullptr;
	if (Data->TryGetArrayField(TEXT("requests"), RequestArray) && RequestArray)
	{
		Snapshot.Requests.Reserve(RequestArray->Num());
		for (const TSharedPtr<FJsonValue>& Value : *RequestArray)
		{
			const TSharedPtr<FJsonObject> RequestObject = Value.IsValid() ? Value->AsObject() : nullptr;
			if (!RequestObject.IsValid())
			{
				continue;
			}

			FGuildJoinRequestInfo Info;
			RequestObject->TryGetStringField(TEXT("characterId"), Info.CharacterId);
			RequestObject->TryGetStringField(TEXT("requestedAt"), Info.RequestedAt);
			Snapshot.Requests.Add(MoveTemp(Info));
		}
	}

	// ApplySnapshot 경유로 레벨/버프/포인트 캐시를 단일 공식(parity)으로 파생.
	ApplySnapshot(Snapshot);
	return true;
}

void UGuildService::AddPendingAutoContribution(int64 Delta)
{
	if (Delta <= 0)
	{
		return;
	}
	// 포화 합산(int64 오버플로 방지).
	PendingAutoContribution = PendingAutoContribution > MAX_int64 - Delta
		? MAX_int64
		: PendingAutoContribution + Delta;
}

int64 UGuildService::ConsumePendingAutoContribution()
{
	if (PendingAutoContribution <= 0)
	{
		return 0;
	}
	// 주간 자동 상한으로 클램프(서버 contribute 가 잔여 상한으로 한 번 더 캡하지만,
	// 클라도 동일 상한으로 선클램프해 과대 보고를 막는다 — parity).
	const int64 Flushed = FMath::Min<int64>(PendingAutoContribution, FGuildFormula::AUTO_WEEKLY_CAP);
	PendingAutoContribution -= Flushed;
	return Flushed;
}

void UGuildService::ExportSave(
	FString& OutGuildId,
	uint8& OutRank,
	int32& OutLevel,
	float& OutAttackPct,
	float& OutGoldPct,
	int64& OutContributionPoints,
	int64& OutPendingAutoContribution,
	FString& OutLastAttendanceDate,
	int32& OutBossDefeatedCount,
	int32& OutBossChallengesRemaining) const
{
	OutGuildId = GetCachedGuildId();
	OutRank = static_cast<uint8>(CachedSnapshot.MyRank);
	OutLevel = CachedLevel;
	OutAttackPct = CachedBuff.AttackPct;
	OutGoldPct = CachedBuff.GoldPct;
	OutContributionPoints = CachedContributionPoints;
	OutPendingAutoContribution = PendingAutoContribution;
	OutLastAttendanceDate = LastAttendanceDate;
	// 보스 진행 표시 캐시(서버 권위 — 재접속 직후 UI 표시용).
	OutBossDefeatedCount = CachedSnapshot.BossDefeatedCount;
	OutBossChallengesRemaining = CachedSnapshot.BossChallengesRemaining;
}

void UGuildService::ImportSave(
	const FString& InGuildId,
	uint8 InRank,
	int32 InLevel,
	float InAttackPct,
	float InGoldPct,
	int64 InContributionPoints,
	int64 InPendingAutoContribution,
	const FString& InLastAttendanceDate,
	int32 InBossDefeatedCount,
	int32 InBossChallengesRemaining)
{
	ClearSnapshot();

	// pending/출석일은 멤버십과 독립적으로 영속(무소속이어도 복원).
	PendingAutoContribution = FMath::Max<int64>(0, InPendingAutoContribution);
	LastAttendanceDate = InLastAttendanceDate;

	if (InGuildId.IsEmpty())
	{
		return;
	}

	CachedSnapshot.bHasGuild = true;
	CachedSnapshot.Guild.Id = InGuildId;
	CachedSnapshot.MyRank = GuildRankFromByte(InRank);

	// 오프라인 버프 적용: 세이브에서 복원한 레벨/버프 캐시를 그대로 사용(서버 재동기화 전까지).
	CachedLevel = FMath::Max(1, InLevel);
	CachedBuff.AttackPct = FMath::Max(0.0f, InAttackPct);
	CachedBuff.GoldPct = FMath::Max(0.0f, InGoldPct);
	CachedContributionPoints = FMath::Max<int64>(0, InContributionPoints);

	CachedSnapshot.GuildLevel = CachedLevel;
	CachedSnapshot.Guild.Level = CachedLevel;
	CachedSnapshot.Buff = CachedBuff;
	CachedSnapshot.ContributionPoints = CachedContributionPoints;

	// 보스 진행 표시 캐시 복원(HP 는 서버 재동기화로 갱신, 격파/도전잔여만 표시 복원).
	CachedSnapshot.BossDefeatedCount = FMath::Max(0, InBossDefeatedCount);
	CachedSnapshot.BossChallengesRemaining = FMath::Max(0, InBossChallengesRemaining);
}
