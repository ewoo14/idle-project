#include "GameCore/IdleGameInstance.h"

#include "CharacterSystem/LevelFormulas.h"
#include "NetworkClient/ApiClient.h"

void UIdleGameInstance::Init()
{
	Super::Init();

	if (GConfig)
	{
		FString ConfigBaseUrl;
		if (GConfig->GetString(TEXT("/Script/IdleProject.IdleGameInstance"), TEXT("ServerBaseUrl"), ConfigBaseUrl, GEngineIni) && !ConfigBaseUrl.IsEmpty())
		{
			ApiBaseUrl = MoveTemp(ConfigBaseUrl);
		}
	}

	FString EnvBaseUrl = FPlatformMisc::GetEnvironmentVariable(TEXT("IDLE_API_BASE_URL"));
	if (!EnvBaseUrl.IsEmpty())
	{
		ApiBaseUrl = MoveTemp(EnvBaseUrl);
	}

	ApiClient = NewObject<UApiClient>(this);
	ApiClient->Initialize(ApiBaseUrl);
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	LoadLastSeenUnixSec();

	ApiClient->RegisterGuest([](bool bSuccess, FString Message)
	{
		UE_LOG(LogTemp, Display, TEXT("[NetworkClient] guest register callback success=%s message=%s"),
			bSuccess ? TEXT("true") : TEXT("false"),
			*Message);
	});
}

void UIdleGameInstance::Shutdown()
{
	LastSeenUnixSec = GetCurrentUnixSeconds();
	SaveLastSeenUnixSec();
	ApiClient = nullptr;
	Super::Shutdown();
}

void UIdleGameInstance::AddGold(int64 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	Gold += Amount;
	OnGoldChanged.Broadcast(Gold);
}

void UIdleGameInstance::AddExp(int64 Amount)
{
	if (Amount <= 0 || CharacterLevel >= FLevelFormulas::LEVEL_CAP)
	{
		return;
	}

	CurrentExp += Amount;
	while (NextExp > 0 && CurrentExp >= NextExp && CharacterLevel < FLevelFormulas::LEVEL_CAP)
	{
		CurrentExp -= NextExp;
		LevelUp();
	}

	OnExpChanged.Broadcast(CurrentExp, NextExp);
}

void UIdleGameInstance::LevelUp()
{
	if (CharacterLevel >= FLevelFormulas::LEVEL_CAP)
	{
		NextExp = 0;
		return;
	}

	++CharacterLevel;
	NextExp = FLevelFormulas::ExpToNext(CharacterLevel);
	OnLevelUp.Broadcast(CharacterLevel);
}

FOfflineRewardResult UIdleGameInstance::ClaimOfflineRewards()
{
	return ClaimOfflineRewardsAt(GetCurrentUnixSeconds(), 0);
}

FOfflineRewardResult UIdleGameInstance::ClaimOfflineRewardsAt(int64 NowUnixSec, int32 RebirthCount)
{
	const FOfflineRewardResult Reward = PreviewOfflineRewards(NowUnixSec, RebirthCount);
	if (ApiClient)
	{
		ApiClient->ClaimOfflineRewards(CharacterLevel, LastSeenUnixSec, NowUnixSec, RebirthCount);
	}
	if (Reward.CappedSeconds <= 0)
	{
		LastSeenUnixSec = FMath::Max(LastSeenUnixSec, NowUnixSec);
		return Reward;
	}

	AddGold(Reward.Gold);
	AddExp(Reward.Exp);
	LastSeenUnixSec = FMath::Max(LastSeenUnixSec, NowUnixSec);
	return Reward;
}

FOfflineRewardResult UIdleGameInstance::PreviewOfflineRewards(int64 NowUnixSec, int32 RebirthCount) const
{
	return FOfflineRewardFormula::ComputeOfflineRewards(CharacterLevel, LastSeenUnixSec, NowUnixSec, RebirthCount);
}

void UIdleGameInstance::SetLastSeenUnixSec(int64 UnixSec)
{
	LastSeenUnixSec = FMath::Max<int64>(0, UnixSec);
}

int64 UIdleGameInstance::GetCurrentUnixSeconds()
{
	return FDateTime::UtcNow().ToUnixTimestamp();
}

void UIdleGameInstance::LoadLastSeenUnixSec()
{
	LastSeenUnixSec = GetCurrentUnixSeconds();
	if (!GConfig)
	{
		return;
	}

	FString SavedLastSeen;
	if (GConfig->GetString(TEXT("/Script/IdleProject.IdleGameInstance"), TEXT("LastSeenUnixSec"), SavedLastSeen, GGameUserSettingsIni))
	{
		int64 ParsedLastSeen = 0;
		if (LexTryParseString(ParsedLastSeen, *SavedLastSeen))
		{
			LastSeenUnixSec = FMath::Max<int64>(0, ParsedLastSeen);
		}
	}
}

void UIdleGameInstance::SaveLastSeenUnixSec() const
{
	if (!GConfig)
	{
		return;
	}

	GConfig->SetString(
		TEXT("/Script/IdleProject.IdleGameInstance"),
		TEXT("LastSeenUnixSec"),
		*FString::Printf(TEXT("%lld"), LastSeenUnixSec),
		GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}
