#include "GameCore/IdleGameInstance.h"

#include "NetworkClient/ApiClient.h"

void UIdleGameInstance::Init()
{
	Super::Init();

	FString EnvBaseUrl = FPlatformMisc::GetEnvironmentVariable(TEXT("IDLE_API_BASE_URL"));
	if (!EnvBaseUrl.IsEmpty())
	{
		ApiBaseUrl = MoveTemp(EnvBaseUrl);
	}

	ApiClient = NewObject<UApiClient>(this);
	ApiClient->Initialize(ApiBaseUrl);
}

void UIdleGameInstance::Shutdown()
{
	ApiClient = nullptr;
	Super::Shutdown();
}
