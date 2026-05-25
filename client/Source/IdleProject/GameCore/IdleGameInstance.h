#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IdleGameInstance.generated.h"

class UApiClient;

/**
 * 게임 전역 서비스 컨테이너입니다.
 * PR #4에서는 API 클라이언트 생성과 기본 URL 보관만 담당합니다.
 */
UCLASS()
class IDLEPROJECT_API UIdleGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintPure, Category = "Idle|Services")
	UApiClient* GetApiClient() const { return ApiClient; }

	UFUNCTION(BlueprintPure, Category = "Idle|Network")
	const FString& GetApiBaseUrl() const { return ApiBaseUrl; }

private:
	UPROPERTY(Transient)
	TObjectPtr<UApiClient> ApiClient;

	/** 환경 변수 IDLE_API_BASE_URL이 없을 때 사용하는 로컬 기본 주소입니다. */
	UPROPERTY()
	FString ApiBaseUrl = TEXT("http://localhost:3000");
};
