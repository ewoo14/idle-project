#pragma once

#include "CoreMinimal.h"
#include "CombatSystem/StatusElementTypes.h"
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

	UFUNCTION(BlueprintCallable, Category = "Idle|Monster")
	void SetBoss(bool bInBoss);

	UFUNCTION(BlueprintPure, Category = "Idle|Monster")
	bool IsBoss() const { return bIsBoss; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Monster")
	void SetWeakElement(ESkillElement InWeakElement) { WeakElement = InWeakElement; }

	UFUNCTION(BlueprintPure, Category = "Idle|Monster")
	ESkillElement GetWeakElement() const { return WeakElement; }

	UFUNCTION(BlueprintCallable, Category = "Idle|Monster")
	void SetStageStatMultiplier(float InStageStatMultiplier);

	float GetConfiguredMaxHp() const;
	float GetConfiguredAttack() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UCombatComponent> Combat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UBattleAIComponent> BattleAI;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	bool bIsBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	float NormalMaxHp = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	float NormalAttack = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	float BossMaxHp = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	float BossAttack = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	ESkillElement WeakElement = ESkillElement::Fire;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Monster")
	float StageStatMultiplier = 1.0f;

	UFUNCTION()
	void HandleDeath(AActor* DyingActor);
};
