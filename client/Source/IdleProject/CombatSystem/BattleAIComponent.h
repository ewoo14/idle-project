#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BattleAIComponent.generated.h"

class UCombatComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpecialAttack, AActor*, Boss);

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

	/** 지정 반경 안의 살아있는 적들을 2D 거리 기준으로 수집합니다. 회전베기 같은 AoE 스킬에 사용합니다. */
	UFUNCTION(BlueprintCallable, Category = "Idle|BattleAI")
	TArray<AActor*> FindEnemiesInRange(float Radius) const;

	UPROPERTY(BlueprintAssignable, Category = "Idle|BattleAI")
	FOnBossSpecialAttack OnBossSpecialAttack;

	UFUNCTION(BlueprintCallable, Category = "Idle|BattleAI")
	void Attack(AActor* TargetActor);

	/**
	 * 컨트롤러 없는(비-캐릭터 경로) 몬스터의 지면 추격 목표 위치를 계산합니다.
	 * 횡스크롤(X-Z 평면)에서 추격은 화면 가로축 X 로만 이루어지고,
	 * 화면 세로/중력축 Z 와 깊이축 Y 는 추격 주체 자신의 값을 유지합니다.
	 * (타깃의 Z 로 끌려가면 플레이어 캡슐 중심으로 떠올라 충돌 이탈로 위로 사라짐 — hotfix/13.)
	 */
	static FVector ComputeGroundChaseLocation(
		const FVector& OwnerLocation,
		const FVector& TargetLocation,
		float DeltaSeconds,
		float Speed);

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
	UCombatComponent* GetOwnerCombat() const;

	FTimerHandle BattleTimerHandle;
	float LastAttackTime = -1000.0f;
	float LastSpecialAttackTime = -1000.0f;
};
