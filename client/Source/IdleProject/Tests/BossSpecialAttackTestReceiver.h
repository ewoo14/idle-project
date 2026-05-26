#pragma once

#include "CoreMinimal.h"
#include "BossSpecialAttackTestReceiver.generated.h"

UCLASS()
class UBossSpecialAttackTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	int32 Count = 0;
	TObjectPtr<AActor> LastBoss = nullptr;

	UFUNCTION()
	void Capture(AActor* Boss)
	{
		++Count;
		LastBoss = Boss;
	}
};
