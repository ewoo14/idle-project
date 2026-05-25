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
	EnsureQuestService();
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
	QuestService = nullptr;
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
	RecordQuestProgress(EQuestObjective::ClaimOffline, 1);
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

void UIdleGameInstance::RecordQuestProgress(EQuestObjective Objective, int32 Amount)
{
	EnsureQuestService();
	if (!QuestService)
	{
		return;
	}

	QuestService->ResetDailyQuestsIfNeeded(UQuestService::GetCurrentUtcDateString());
	QuestService->RecordProgress(Objective, Amount);
}

void UIdleGameInstance::RecordMonsterKilled()
{
	RecordQuestProgress(EQuestObjective::KillMonster, 1);
}

void UIdleGameInstance::RecordGearEnhanced()
{
	RecordQuestProgress(EQuestObjective::Enhance, 1);
}

FQuestClaimResult UIdleGameInstance::ClaimQuest(const FString& QuestId)
{
	EnsureQuestService();
	FQuestClaimResult Result;
	if (!QuestService)
	{
		Result.Message = TEXT("quest_service_unavailable");
		return Result;
	}

	QuestService->ResetDailyQuestsIfNeeded(UQuestService::GetCurrentUtcDateString());
	Result = QuestService->ClaimQuest(QuestId);
	if (!Result.bSuccess)
	{
		return Result;
	}

	AddGold(Result.RewardGold);
	AddExp(Result.RewardExp);
	if (ApiClient)
	{
		ApiClient->ClaimQuestReward(QuestId, FString());
	}
	return Result;
}

TArray<FQuestState> UIdleGameInstance::GetActiveQuestStates() const
{
	return QuestService ? QuestService->GetActiveQuestStates() : TArray<FQuestState>();
}

bool UIdleGameInstance::GetQuestState(const FString& QuestId, FQuestState& OutState) const
{
	return QuestService ? QuestService->GetQuestState(QuestId, OutState) : false;
}

void UIdleGameInstance::InitializeQuestServiceForTests(const FString& CurrentUtcDate)
{
	QuestService = NewObject<UQuestService>(this);
	QuestService->InitializeDefaultQuests(CurrentUtcDate);
}

int64 UIdleGameInstance::GetCurrentUnixSeconds()
{
	return FDateTime::UtcNow().ToUnixTimestamp();
}

void UIdleGameInstance::EnsureQuestService()
{
	if (!QuestService)
	{
		QuestService = NewObject<UQuestService>(this);
		QuestService->InitializeDefaultQuests(UQuestService::GetCurrentUtcDateString());
	}
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
