#include "GameCore/GuildService.h"

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
}

void UGuildService::ClearSnapshot()
{
	CachedSnapshot = FGuildSnapshot();
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

	// 내 멤버십(me) — rank.
	const TSharedPtr<FJsonObject>* MeObjectPtr = nullptr;
	if (Data->TryGetObjectField(TEXT("me"), MeObjectPtr) && MeObjectPtr && MeObjectPtr->IsValid())
	{
		FString MyRankString;
		if ((*MeObjectPtr)->TryGetStringField(TEXT("rank"), MyRankString))
		{
			Snapshot.MyRank = GuildParseRank(MyRankString);
		}
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

	CachedSnapshot = MoveTemp(Snapshot);
	return true;
}

void UGuildService::ExportSave(FString& OutGuildId, uint8& OutRank) const
{
	OutGuildId = GetCachedGuildId();
	OutRank = static_cast<uint8>(CachedSnapshot.MyRank);
}

void UGuildService::ImportSave(const FString& InGuildId, uint8 InRank)
{
	ClearSnapshot();
	if (InGuildId.IsEmpty())
	{
		return;
	}

	CachedSnapshot.bHasGuild = true;
	CachedSnapshot.Guild.Id = InGuildId;
	CachedSnapshot.MyRank = GuildRankFromByte(InRank);
}
