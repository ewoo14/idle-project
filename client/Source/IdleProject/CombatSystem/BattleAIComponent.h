#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BattleAIComponent.generated.h"

class UCombatComponent;

UENUM(BlueprintType)
enum class EBattleState : uint8
{
	None = 0 UMETA(Hidden),
	Idle = 1 UMETA(DisplayName = "Idle"),
	Chase = 2 UMETA(DisplayName = "Chase"),
	Attack = 3 UMETA(DisplayName = "Attack"),
	Dead = 4 UMETA(DisplayName = "Dead")
};

/** 5Hz 타이머 기반 자동 전투 AI 컴포넌트입니다. */
UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API UBattleAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBattleAIComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|BattleAI")
	TSubclassOf<AActor> TargetActorClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|BattleAI")
	EBattleState State = EBattleState::Idle;

	UFUNCTION(BlueprintCallable, Category = "Idle|BattleAI")
	void StartBattle();

	UFUNCTION(BlueprintCallable, Category = "Idle|BattleAI")
	void StopBattle();

	UFUNCTION(BlueprintCallable, Category = "Idle|BattleAI")
	AActor* FindClosestEnemy();

	/** AI 갱신 주기(초). 기본 0.2s = 5Hz. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|BattleAI", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float BattleInterval = 0.2f;

	/** 비-캐릭터(예: 몬스터) 추격 시 사용하는 보간 이동 속도(units/sec). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|BattleAI", meta = (ClampMin = "10.0"))
	float NonCharacterMoveSpeed = 220.0f;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void UpdateBattle();
	void MoveTowards(AActor* TargetActor, float DeltaSeconds);
	void Attack(AActor* TargetActor);
	UCombatComponent* GetOwnerCombat() const;

	FTimerHandle BattleTimerHandle;
	float LastAttackTime = -1000.0f;
};
