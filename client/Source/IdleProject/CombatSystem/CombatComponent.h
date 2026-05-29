#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatSystem/StatusElementTypes.h"
#include "CombatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DyingActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChanged, float, NewHp);

UENUM(BlueprintType)
enum class EDamageKind : uint8
{
	Physical = 0 UMETA(DisplayName = "Physical"),
	Magic = 1 UMETA(DisplayName = "Magic")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, float, Amount, bool, bWasCrit, EDamageKind, Kind);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnAnyDamageReceived, AActor* /* DamagedActor */, float /* Amount */, bool /* bWasCrit */, EDamageKind /* Kind */);

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FActiveSkillStatus
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	ESkillStatusEffect Type = ESkillStatusEffect::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	float EndTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	float Magnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	float NextTickTime = 0.0f;
};

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
	float MagicAtk = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float MagicDef = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float AtkSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float CritRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float CritDmg = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Combat")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Combat")
	FOnHpChanged OnHpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Idle|Combat")
	FOnDamageReceived OnDamageReceived;

	static FOnAnyDamageReceived OnAnyDamageReceived;

	UFUNCTION(BlueprintCallable, Category = "Idle|Combat")
	void TakeDamage(float Damage, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Idle|Combat")
	void TakeDamageTyped(float Damage, AActor* Instigator, bool bWasCrit, EDamageKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Idle|Combat|Status")
	void ApplyStatus(ESkillStatusEffect Type, float Duration, float Magnitude, float Now);

	UFUNCTION(BlueprintCallable, Category = "Idle|Combat|Status")
	void TickStatuses(float Now);

	UFUNCTION(BlueprintPure, Category = "Idle|Combat|Status")
	bool HasActiveStatus(ESkillStatusEffect Type) const;

	/** 지정한 상태가 활성일 때 가장 강한 Magnitude를 반환합니다(없으면 0). 저주 피해 증폭 등에서 사용합니다. */
	UFUNCTION(BlueprintPure, Category = "Idle|Combat|Status")
	float GetActiveStatusMagnitude(ESkillStatusEffect Type) const;

	const TArray<FActiveSkillStatus>& GetActiveStatuses() const { return ActiveStatuses; }

	UFUNCTION(BlueprintPure, Category = "Idle|Combat")
	bool IsDead() const;

	void InitializeCombat(float InMaxHp, float InAtk, float InDef, float InAtkSpeed, float InCritRate = 0.0f, float InCritDmg = 1.5f);
	void InitializeCombat(float InMaxHp, float InAtk, float InDef, float InAtkSpeed, float InMagicAtk, float InMagicDef, float InCritRate, float InCritDmg);
	bool RollCrit();

private:
	void RemoveStatusAttackSpeedSlow();
	void ApplyCurrentStatusAttackSpeedSlow(float Now);

	bool bDeathBroadcast = false;
	FRandomStream CritRandomStream;
	TArray<FActiveSkillStatus> ActiveStatuses;
	float LastAppliedStatusAtkSpeedPenalty = 0.0f;
};
