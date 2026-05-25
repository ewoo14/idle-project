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

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void LoadDefaultWarriorSkills();

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets);

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	bool IsReady(FName SkillId, float Now) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCooldownRemaining(FName SkillId, float Now) const;

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCooldownRatio(FName SkillId, float Now) const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	bool MarkSkillCast(FName SkillId, float Now);

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	void AddGauge(float Amount);

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	float GetCurrentGauge() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Skill")
	bool IsUltimateReady() const;

	UFUNCTION(BlueprintCallable, Category = "Idle|Skill")
	bool TryConsumeUltimateGauge();

	void ApplyPassivesToStats(FDerivedStats& InOutStats) const;
	float GetGaugeGainOnHit() const;
	float GetGaugeGainOnTakeDamage() const;

private:
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
