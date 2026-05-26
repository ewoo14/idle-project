#include "CombatSystem/SkillComponent.h"

#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/CombatFormulas.h"
#include "Internationalization/IdleLocalization.h"

namespace
{
const FName ShieldUpId(TEXT("shield_up"));
const FName ManaShieldId(TEXT("mana_shield"));
const FName FocusId(TEXT("focus"));
const FName WeaponMasteryId(TEXT("weapon_mastery"));
const FName ToughnessId(TEXT("toughness"));
const FName BerserkersFuryId(TEXT("berserkers_fury"));
const FName SpellMasteryId(TEXT("spell_mastery"));
const FName ManaFlowId(TEXT("mana_flow"));
const FName ArcaneOverloadId(TEXT("arcane_overload"));
const FName CriticalEyeId(TEXT("critical_eye"));
const FName QuickDrawId(TEXT("quick_draw"));
const FName EagleEyeId(TEXT("eagle_eye"));

FSkillDefinition MakeSkill(
	const TCHAR* SkillId,
	EClassId ClassId,
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
	Skill.ClassId = ClassId;
	Skill.DisplayName = IdleProject::Localization::Text(TEXT("Skill"), SkillId);
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
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("heavy_strike"), EClassId::Warrior, TEXT("강타"), ESkillType::Active, ESkillEffectType::DamageSingle, 4.0f, 2.5f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("whirlwind"), EClassId::Warrior, TEXT("회전베기"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.8f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("shield_up"), EClassId::Warrior, TEXT("방패 올리기"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.5f, 4.0f));
	Skills.Add(MakeSkill(TEXT("charge"), EClassId::Warrior, TEXT("돌진"), ESkillType::Active, ESkillEffectType::DashDamage, 10.0f, 2.0f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("weapon_mastery"), EClassId::Warrior, TEXT("무기 숙련"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("toughness"), EClassId::Warrior, TEXT("강인함"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("berserkers_fury"), EClassId::Warrior, TEXT("광전사의 분노"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 6.0f, 0.3f, 4.0f, 8.0f, 5.0f));
}

void USkillComponent::LoadDefaultMageSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("arcane_bolt"), EClassId::Mage, TEXT("Arcane Bolt"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.0f, 2.4f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("chain_lightning"), EClassId::Mage, TEXT("Chain Lightning"), ESkillType::Active, ESkillEffectType::DamageAoe, 7.0f, 1.7f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("mana_shield"), EClassId::Mage, TEXT("Mana Shield"), ESkillType::Active, ESkillEffectType::SelfBuff, 12.0f, 0.0f, 0.35f, 4.0f));
	Skills.Add(MakeSkill(TEXT("meteor"), EClassId::Mage, TEXT("Meteor"), ESkillType::Active, ESkillEffectType::DamageAoe, 14.0f, 2.8f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("spell_mastery"), EClassId::Mage, TEXT("Spell Mastery"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.15f, 0.0f));
	Skills.Add(MakeSkill(TEXT("mana_flow"), EClassId::Mage, TEXT("Mana Flow"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.2f, 0.0f));
	Skills.Add(MakeSkill(TEXT("arcane_overload"), EClassId::Mage, TEXT("Arcane Overload"), ESkillType::Ultimate, ESkillEffectType::DamageAoe, 0.0f, 5.5f, 0.25f, 4.0f, 9.0f, 3.0f));
}

void USkillComponent::LoadDefaultArcherSkills()
{
	ResetSkillState();

	Skills.Add(MakeSkill(TEXT("precision_shot"), EClassId::Archer, TEXT("Precision Shot"), ESkillType::Active, ESkillEffectType::DamageSingle, 3.5f, 2.2f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("arrow_rain"), EClassId::Archer, TEXT("Arrow Rain"), ESkillType::Active, ESkillEffectType::DamageAoe, 8.0f, 1.6f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("focus"), EClassId::Archer, TEXT("Focus"), ESkillType::Active, ESkillEffectType::SelfBuff, 10.0f, 0.0f, 0.2f, 4.0f));
	Skills.Add(MakeSkill(TEXT("piercing_arrow"), EClassId::Archer, TEXT("Piercing Arrow"), ESkillType::Active, ESkillEffectType::DashDamage, 9.0f, 2.0f, 0.0f, 0.0f));
	Skills.Add(MakeSkill(TEXT("critical_eye"), EClassId::Archer, TEXT("Critical Eye"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.05f, 0.0f));
	Skills.Add(MakeSkill(TEXT("quick_draw"), EClassId::Archer, TEXT("Quick Draw"), ESkillType::Passive, ESkillEffectType::SelfBuff, 0.0f, 0.0f, 0.1f, 0.0f));
	Skills.Add(MakeSkill(TEXT("eagle_eye"), EClassId::Archer, TEXT("Eagle Eye"), ESkillType::Ultimate, ESkillEffectType::DamageSingle, 0.0f, 5.0f, 0.25f, 4.0f, 10.0f, 2.0f));
}

void USkillComponent::LoadSkillsForClass(EClassId ClassId)
{
	switch (ClassId)
	{
	case EClassId::Mage:
		LoadDefaultMageSkills();
		break;
	case EClassId::Archer:
		LoadDefaultArcherSkills();
		break;
	case EClassId::Warrior:
	default:
		LoadDefaultWarriorSkills();
		break;
	}
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

	const float EffectiveCooldown = GetEffectiveCooldown(SkillId);
	return FMath::Max(0.0f, EffectiveCooldown - (Now - *LastCastTime));
}

float USkillComponent::GetCooldownRatio(FName SkillId, float Now) const
{
	const float EffectiveCooldown = GetEffectiveCooldown(SkillId);
	if (EffectiveCooldown <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(GetCooldownRemaining(SkillId, Now) / EffectiveCooldown, 0.0f, 1.0f);
}

void USkillComponent::GrantSkillPoint(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SkillPoints += Amount;
}

int32 USkillComponent::GetSkillPoints() const
{
	return SkillPoints;
}

int32 USkillComponent::GetSkillRank(FName SkillId) const
{
	const int32* Rank = SkillRanks.Find(SkillId);
	return Rank ? FMath::Clamp(*Rank, 0, MaxRank) : 0;
}

bool USkillComponent::CanRankUp(FName SkillId) const
{
	return SkillPoints > 0 && FindSkill(SkillId) && GetSkillRank(SkillId) < MaxRank;
}

bool USkillComponent::RankUpSkill(FName SkillId)
{
	if (!CanRankUp(SkillId))
	{
		return false;
	}

	SkillRanks.FindOrAdd(SkillId) = GetSkillRank(SkillId) + 1;
	--SkillPoints;
	return true;
}

float USkillComponent::GetEffectiveDamageCoeff(FName SkillId) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill)
	{
		return 0.0f;
	}

	return Skill->DamageCoeff * (1.0f + static_cast<float>(GetSkillRank(SkillId)) * 0.1f);
}

float USkillComponent::GetEffectiveCooldown(FName SkillId) const
{
	const FSkillDefinition* Skill = FindSkill(SkillId);
	if (!Skill || Skill->Cooldown <= 0.0f)
	{
		return 0.0f;
	}

	const float ReducedCooldown = Skill->Cooldown * (1.0f - static_cast<float>(GetSkillRank(SkillId)) * 0.05f);
	return FMath::Max(0.1f, ReducedCooldown);
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

void USkillComponent::ResetSkillState()
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
	if (FindSkill(SpellMasteryId))
	{
		InOutStats.MagicAtk = FMath::RoundToFloat(InOutStats.MagicAtk * 1.15f);
	}
	if (FindSkill(ManaFlowId))
	{
		InOutStats.Mp = FMath::RoundToFloat(InOutStats.Mp * 1.2f);
	}
	if (FindSkill(CriticalEyeId))
	{
		InOutStats.CritRate = FMath::Clamp(InOutStats.CritRate + 0.05f, 0.0f, 1.0f);
	}
	if (FindSkill(QuickDrawId))
	{
		InOutStats.AtkSpeed = FMath::RoundToFloat(InOutStats.AtkSpeed * 1.1f * 10.0f) / 10.0f;
	}
}

float USkillComponent::GetGaugeGainOnHit() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	if (!Ultimate)
	{
		Ultimate = FindSkill(ArcaneOverloadId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(EagleEyeId);
	}
	return Ultimate ? Ultimate->GaugeGainOnHit : 0.0f;
}

float USkillComponent::GetGaugeGainOnTakeDamage() const
{
	const FSkillDefinition* Ultimate = FindSkill(BerserkersFuryId);
	if (!Ultimate)
	{
		Ultimate = FindSkill(ArcaneOverloadId);
	}
	if (!Ultimate)
	{
		Ultimate = FindSkill(EagleEyeId);
	}
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

	const bool bMagicDamage = Skill.ClassId == EClassId::Mage;
	const float AttackPower = bMagicDamage ? OwnerCombat->MagicAtk : OwnerCombat->Atk;
	const float Defense = bMagicDamage ? TargetCombat->MagicDef : TargetCombat->Def;
	const float EffectiveDamageCoeff = GetEffectiveDamageCoeff(Skill.SkillId);
	const float BaseDamage = bMagicDamage
		? FCombatFormulas::ComputeMagicDamage(AttackPower * EffectiveDamageCoeff, Defense)
		: FCombatFormulas::ComputeDamage(AttackPower * EffectiveDamageCoeff, Defense);
	const bool bWasCrit = OwnerCombat->RollCrit();
	const float FinalDamage = FCombatFormulas::ApplyCrit(BaseDamage, bWasCrit, OwnerCombat->CritDmg);
	const EDamageKind DamageKind = bMagicDamage ? EDamageKind::Magic : EDamageKind::Physical;

	TargetCombat->TakeDamageTyped(FinalDamage, GetOwner(), bWasCrit, DamageKind);
	if (Skill.Type != ESkillType::Ultimate)
	{
		AddGauge(Skill.GaugeGainOnHit);
	}
}

void USkillComponent::ApplySelfBuff(const FSkillDefinition& Skill, float Now)
{
	if (Skill.SkillId == ShieldUpId || Skill.SkillId == ManaShieldId)
	{
		DefBuffMagnitude = Skill.BuffMagnitude;
		DefBuffEndTime = Now + Skill.BuffDuration;
	}
	else if (Skill.SkillId == BerserkersFuryId || Skill.SkillId == ArcaneOverloadId || Skill.SkillId == FocusId || Skill.SkillId == EagleEyeId)
	{
		AtkSpeedBuffMagnitude = Skill.BuffMagnitude;
		AtkSpeedBuffEndTime = Now + Skill.BuffDuration;
	}

	UpdateTimedBuffs(Now);
}
