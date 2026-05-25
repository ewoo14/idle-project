#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IdleMonster.generated.h"

class UBattleAIComponent;
class UCombatComponent;
class UStaticMeshComponent;

/** 자동 전투 검증용 슬라임 placeholder 몬스터입니다. */
UCLASS()
class IDLEPROJECT_API AIdleMonster : public ACharacter
{
	GENERATED_BODY()

public:
	AIdleMonster();

	virtual void BeginPlay() override;

	UCombatComponent* GetCombat() const { return Combat; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UCombatComponent> Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UBattleAIComponent> BattleAI;

	UFUNCTION()
	void HandleDeath(AActor* DyingActor);
};
