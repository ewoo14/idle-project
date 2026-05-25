#include "CombatSystem/SkillComponent.h"

#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"

namespace
{
const FName ShieldUpId(TEXT("shield_up"));
const FName WeaponMasteryId(TEXT("weapon_mastery"));
const FName ToughnessId(TEXT("toughness"));
const FName BerserkersFuryId(TEXT("berserkers_fury"));

FSkillDefinition MakeSkill(
	const TCHAR* SkillId,
	const TCHAR* DisplayName,
	ESkillType Type,
	ESkillEffectType EffectType,
	float Cooldown,
	float DamageCoeff,
	float BuffMagnitude,
	float BuffDuration,
	float GaugeGainOnHit = 0.0f,
	float GaugeGainOnTakeDamage = 0.0f)
{
	FSkillDefinition Skill;
	Skill.SkillId = FName(SkillId);
	Skill.DisplayName = FText::FromString(DisplayName);
	Skill.Type = Type;
	Skill.EffectType = EffectType;
	Skill.Cooldown = Cooldown;
	Skill.DamageCoeff = DamageCoeff;
	Skill.BuffMagnitude = BuffMagnitude;
	Skill.BuffDuration = BuffDuration;
	Skill.GaugeGainOnHit = GaugeGainOnHit;
	Skill.GaugeGainOnTakeDamage = GaugeGainOnTakeDamage;
	return Skill;
}
}

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::LoadDefaultWarriorSkills()
{
	Skills.Reset();
	LastCastTimeBySkill.Reset();
	CurrentGauge = 0.0f;
	LastAppliedDefBonus = 0.0f;
	DefBuffMagnitude = 0.0f;
	DefBuffEndTime = 0.0f;
	LastAppliedAtkSpeedBonus = 0.0f;
	AtkSpeedBuffMagnitude = 0.0f;
	AtkSpeedBuffEndTime = 0.0f;

	Skills.Add(MakeSkill(TEXT("heavy_strike"), TEXT("Heavy Strike"), ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.5f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("whirlwind"), TEXT("Whirlwind"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.8f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("shield_up"), TEXT("Shield Up"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.5f, 4.0f));
	Skills.Add(MakeSkill(TEXT("charge"), TEXT("Charge"), ESkillType::Active, ESkillEffectType::DashDamage, 10.0f, 2.0f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("weapon_mastery"), TEXT("Weapon Mastery"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("toughness"), TEXT("Toughness"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("berserkers_fury"), TEXT("Berserker's Fury"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.0f, 0.3f, 4.0f, 8.0f, 5.0f));
}

void USkillComponent::TickSkills(float Now, AActor* Target, const TArray<AActor*>& AoeTargets)
{
	UpdateTimedBuffs(Now);

	for (const FSkillDefinition& Skill : Skills)
	{
		if (Skill.Type == ESkillType::Ultimate && IsUltimateReady())
		{
			ExecuteSkill(Skill, Now, Target, AoeTargets);
			return;
		}
	}

	for (const FSkillDefinition& Skill : Skills)
	{
		if (Skill.Type == ESkillType::Active && IsReady(Skill.SkillId, Now))
		{
			ExecuteSkill(Skill, Now, Target, AoeTargets);
		}
	}
}

bool USkillComponent::IsReady(FName SkillId, float Now) const
{
	return FindSkill(SkillId) && GetCooldownRemaining(SkillId, Now) <= 0.0f;
}

float USkillComponent::GetCooldownRemaining(FName SkillId, float Now) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill)
	{
		return 0.0f;
	}

	const float* LastCastTime = LastCastTimeBySkill.Find(SkillId);
	if (!LastCastTime)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, Skill->Cooldown - (Now - *LastCastTime));
}

float USkillComponent::GetCooldownRatio(FName SkillId, float Now) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill || Skill->Cooldown <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetCooldownRemaining(SkillId, Now) / Skill->Cooldown, 0.0f, 1.0f);
}

bool USkillComponent::MarkSkillCast(FName SkillId, float Now)
{
	if (!FindSkill(SkillId))
	{
		return false;
	}

	LastCastTimeBySkill.Add(SkillId, Now);
	return true;
}

void USkillComponent::AddGauge(float Amount)
{
	CurrentGauge = FMath::Clamp(CurrentGauge + Amount, 0.0f, 100.0f);
}

float USkillComponent::GetCurrentGauge() const
{
	return CurrentGauge;
}

bool USkillComponent::IsUltimateReady() const
{
	return CurrentGauge >= 100.0f;
}

bool USkillComponent::TryConsumeUltimateGauge()
{
	if (!IsUltimateReady())
	{
		return false;
	}

	CurrentGauge = 0.0f;
	return true;
}

void USkillComponent::ApplyPassivesToStats(FDerivedStats& InOutStats) const
{
	if (FindSkill(WeaponMasteryId))
	{
		InOutStats.PhysAtk = FMath::RoundToFloat(InOutStats.PhysAtk * 1.15f);
	}
	if (FindSkill(ToughnessId))
	{
		InOutStats.Hp = FMath::RoundToFloat(InOutStats.Hp * 1.2f);
	}
}

float USkillComponent::GetGaugeGainOnHit() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	return Ultimate ? Ultimate->GaugeGainOnHit : 0.0f;
}

float USkillComponent::GetGaugeGainOnTakeDamage() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	return Ultimate ? Ultimate->GaugeGainOnTakeDamage : 0.0f;
}

const FSkillDefinition* USkillComponent::FindSkill(FName SkillId) const
{
	return Skills.FindByPredicate([SkillId](const FSkillDefinition& Skill)
	{
		return Skill.SkillId == SkillId;
	});
}

void USkillComponent::UpdateTimedBuffs(float Now)
{
	UCombatComponent* Combat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!Combat)
	{
		return;
	}

	if (LastAppliedDefBonus != 0.0f)
	{
		Combat->Def = FMath::Max(0.0f, Combat->Def - LastAppliedDefBonus);
		LastAppliedDefBonus = 0.0f;
	}
	if (LastAppliedAtkSpeedBonus != 0.0f)
	{
		Combat->AtkSpeed = FMath::Max(0.1f, Combat->AtkSpeed - LastAppliedAtkSpeedBonus);
		LastAppliedAtkSpeedBonus = 0.0f;
	}

	if (DefBuffEndTime > Now)
	{
		LastAppliedDefBonus = Combat->Def * DefBuffMagnitude;
		Combat->Def += LastAppliedDefBonus;
	}
	if (AtkSpeedBuffEndTime > Now)
	{
		LastAppliedAtkSpeedBonus = Combat->AtkSpeed * AtkSpeedBuffMagnitude;
		Combat->AtkSpeed += LastAppliedAtkSpeedBonus;
	}
}

void USkillComponent::ExecuteSkill(const FSkillDefinition& Skill, float Now, AActor* Target, const TArray<AActor*>& AoeTargets)
{
	if (Skill.Type == ESkillType::Ultimate && !TryConsumeUltimateGauge())
	{
		return;
	}

	if (Skill.EffectType == ESkillEffectType::SelfBuff)
	{
		ApplySelfBuff(Skill, Now);
	}
	else if (Skill.EffectType == ESkillEffectType::DamageAoe)
	{
		for (AActor* AoeTarget : AoeTargets)
		{
			ApplyDamageSkill(Skill, AoeTarget);
		}
	}
	else
	{
		ApplyDamageSkill(Skill, Target);
	}
	if (Skill.Type == ESkillType::Ultimate && Skill.BuffMagnitude > 0.0f)
	{
		ApplySelfBuff(Skill, Now);
	}

	MarkSkillCast(Skill.SkillId, Now);
}

void USkillComponent::ApplyDamageSkill(const FSkillDefinition& Skill, AActor* Target)
{
	UCombatComponent* OwnerCombat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	UCombatComponent* TargetCombat = Target ? Target->FindComponentByClass<UCombatComponent>() : nullptr;
	if (!OwnerCombat || !TargetCombat || TargetCombat->IsDead())
	{
		return;
	}

	TargetCombat->TakeDamage(FCombatFormulas::ComputeDamage(OwnerCombat->Atk * Skill.DamageCoeff, TargetCombat->Def), GetOwner());
	if (Skill.Type != ESkillType::Ultimate)
	{
		AddGauge(Skill.GaugeGainOnHit);
	}
}

void USkillComponent::ApplySelfBuff(const FSkillDefinition& Skill, float Now)
{
	if (Skill.SkillId == ShieldUpId)
	{
		DefBuffMagnitude = Skill.BuffMagnitude;
		DefBuffEndTime = Now + Skill.BuffDuration;
	}
	else if (Skill.SkillId == BerserkersFuryId)
	{
		AtkSpeedBuffMagnitude = Skill.BuffMagnitude;
		AtkSpeedBuffEndTime = Now + Skill.BuffDuration;
	}

	UpdateTimedBuffs(Now);
}
