#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "IdleAnimInstance.generated.h"

/** AnimBlueprint State Machine에서 참조하는 캐릭터 상태 변수 베이스입니다. */
UCLASS(Blueprintable)
class IDLEPROJECT_API UIdleAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Anim")
	bool bIsMoving = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Anim")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Anim")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Anim")
	float MovementSpeed = 0.0f;
};
