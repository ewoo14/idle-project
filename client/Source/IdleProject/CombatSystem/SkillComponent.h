#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "Components/ActorComponent.h"
#include "SkillComponent.generated.h"

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	Active = 0 UMETA(DisplayName = "Active"),
	Passive = 1 UMETA(DisplayName = "Passive"),
	Ultimate = 2 UMETA(DisplayName = "Ultimate")
};

UENUM(BlueprintType)
enum class ESkillEffectType : uint8
{
	DamageSingle = 0 UMETA(DisplayName = "Damage Single"),
	DamageAoe = 1 UMETA(DisplayName = "Damage AOE"),
	SelfBuff = 2 UMETA(DisplayName = "Self Buff"),
	DashDamage = 3 UMETA(DisplayName = "Dash Damage")
};

/** 전사 스킬 V1 정적 정의입니다. SkillDB/DataTable 미러링은 후속 슬라이스에서 처리합니다. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FSkillDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	FName SkillId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	EClassId ClassId = EClassId::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	ESkillType Type = ESkillType::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	ESkillEffectType EffectType = ESkillEffectType::DamageSingle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill", meta = (ClampMin = "0.0"))
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill", meta = (ClampMin = "0.0"))
	float DamageCoeff = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	float BuffMagnitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill", meta = (ClampMin = "0.0"))
	float BuffDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	float GaugeGainOnHit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	float GaugeGainOnTakeDamage = 0.0f;
};

/** 스킬 쿨다운, 궁극기 게이지, 패시브 스탯 보너스, 간단한 자기 버프를 관리합니다. */
UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle|Skill")
	TArray<FSkillDefinition> Skills;

	/** 전사 기본 스킬 7종을 정적 정의에서 다시 로드하고 런타임 상태를 초기화합니다. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void LoadDefaultWarriorSkills();

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void LoadDefaultMageSkills();

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void LoadDefaultArcherSkills();

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void LoadSkillsForClass(EClassId ClassId);

	/** 전투 틱에서 준비된 액티브/궁극기 스킬을 자동 발동합니다. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets);

	/** 지정 스킬이 현재 시간 기준으로 쿨다운 없이 발동 가능한지 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	bool IsReady(FName SkillId, float Now) const;

	/** HUD 표시용 남은 쿨다운 초를 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCooldownRemaining(FName SkillId, float Now) const;

	/** HUD 표시용 남은 쿨다운 비율(0~1)을 반환합니다. */
	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCooldownRatio(FName SkillId, float Now) const;

	/** 테스트/스킬 실행에서 마지막 발동 시간을 기록합니다. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	bool MarkSkillCast(FName SkillId, float Now);

	/** 공격/피격 이벤트로 얻은 궁극기 게이지를 0~100 범위에 누적합니다. */
	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void AddGauge(float Amount);

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCurrentGauge() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	bool IsUltimateReady() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	bool TryConsumeUltimateGauge();

	/** 영구 패시브 스킬 효과를 파생 능력치에 합산합니다. */
	void ApplyPassivesToStats(FDerivedStats& InOutStats) const;
	float GetGaugeGainOnHit() const;
	float GetGaugeGainOnTakeDamage() const;

private:
	void ResetSkillState();
	const FSkillDefinition* FindSkill(FName SkillId) const;
	void UpdateTimedBuffs(float Now);
	void ExecuteSkill(const FSkillDefinition& Skill, float Now, AActor* Target, const TArray<AActor*>& AoeTargets);
	void ApplyDamageSkill(const FSkillDefinition& Skill, AActor* Target);
	void ApplySelfBuff(const FSkillDefinition& Skill, float Now);

	UPROPERTY(VisibleAnywhere, Category = "Idle|Skill")
	TMap<FName, float> LastCastTimeBySkill;

	UPROPERTY(VisibleAnywhere, Category = "Idle|Skill")
	float CurrentGauge = 0.0f;

	float LastAppliedDefBonus = 0.0f;
	float DefBuffMagnitude = 0.0f;
	float DefBuffEndTime = 0.0f;
	float LastAppliedAtkSpeedBonus = 0.0f;
	float AtkSpeedBuffMagnitude = 0.0f;
	float AtkSpeedBuffEndTime = 0.0f;
};
