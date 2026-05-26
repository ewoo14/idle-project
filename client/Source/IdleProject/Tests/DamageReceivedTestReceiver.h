#pragma once

#include "CoreMinimal.h"
#include "CombatSystem/CombatComponent.h"
#include "DamageReceivedTestReceiver.generated.h"

UCLASS()
class UDamageReceivedTestReceiver : public UObject
{
	GENERATED_BODY()

public:
	float LastAmount = 0.0f;
	bool bLastWasCrit = false;
	EDamageKind LastKind = EDamageKind::Physical;

	UFUNCTION()
	void Capture(float Amount, bool bWasCrit, EDamageKind Kind)
	{
		LastAmount = Amount;
		bLastWasCrit = bWasCrit;
		LastKind = Kind;
	}
};
