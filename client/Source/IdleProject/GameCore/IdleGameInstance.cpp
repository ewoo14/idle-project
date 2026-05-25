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

	ApiClient->RegisterGuest([](bool bSuccess, FString Message)
	{
		UE_LOG(LogTemp, Display, TEXT("[NetworkClient] guest register callback success=%s message=%s"),
			bSuccess ? TEXT("true") : TEXT("false"),
			*Message);
	});
}

void UIdleGameInstance::Shutdown()
{
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
