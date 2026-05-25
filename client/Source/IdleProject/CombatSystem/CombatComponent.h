#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChanged, float, NewHp);

/** 캐릭터와 몬스터가 공유하는 HP/공격력 기반 전투 상태 컴포넌트입니다. */
UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float MaxHp = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float CurrentHp = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float Atk = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float Def = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float AtkSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Combat")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Combat")
	FOnHpChanged OnHpChanged;

	UFUNCTION(BlueprintCallable, Category = "Idle|Combat")
	void TakeDamage(float Damage, AActor* Instigator);

	UFUNCTION(BlueprintPure, Category = "Idle|Combat")
	bool IsDead() const;

	void InitializeCombat(float InMaxHp, float InAtk, float InDef, float InAtkSpeed);

private:
	bool bDeathBroadcast = false;
};
