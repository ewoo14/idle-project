#include "CombatSystem/CombatComponent.h"

#include "CombatSystem/CombatFormulas.h"
#include "CombatSystem/SkillComponent.h"

FOnAnyDamageReceived UCombatComponent::OnAnyDamageReceived;

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CritRandomStream.Initialize(FMath::Rand());
}

void UCombatComponent::InitializeCombat(float InMaxHp, float InAtk, float InDef, float InAtkSpeed, float InCritRate, float InCritDmg)
{
	InitializeCombat(InMaxHp, InAtk, InDef, InAtkSpeed, 0.0f, 0.0f, InCritRate, InCritDmg);
}

void UCombatComponent::InitializeCombat(
	float InMaxHp,
	float InAtk,
	float InDef,
	float InAtkSpeed,
	float InMagicAtk,
	float InMagicDef,
	float InCritRate,
	float InCritDmg)
{
	MaxHp = FMath::Max(1.0f, InMaxHp);
	CurrentHp = MaxHp;
	Atk = FMath::Max(0.0f, InAtk);
	Def = FMath::Max(0.0f, InDef);
	MagicAtk = FMath::Max(0.0f, InMagicAtk);
	MagicDef = FMath::Max(0.0f, InMagicDef);
	AtkSpeed = FMath::Max(0.1f, InAtkSpeed);
	CritRate = FMath::Clamp(InCritRate, 0.0f, 1.0f);
	CritDmg = FMath::Max(1.0f, InCritDmg);
	bDeathBroadcast = false;
	OnHpChanged.Broadcast(CurrentHp);
}

bool UCombatComponent::RollCrit()
{
	return FCombatFormulas::RollCrit(CritRate, CritRandomStream);
}

void UCombatComponent::TakeDamage(float Damage, AActor* Instigator)
{
	TakeDamageTyped(Damage, Instigator, false, EDamageKind::Physical);
}

void UCombatComponent::TakeDamageTyped(float Damage, AActor* Instigator, bool bWasCrit, EDamageKind Kind)
{
	if (IsDead())
	{
		return;
	}

	const float AppliedDamage = FMath::Max(0.0f, Damage);
	CurrentHp = FMath::Clamp(CurrentHp - AppliedDamage, 0.0f, MaxHp);
	OnHpChanged.Broadcast(CurrentHp);

	if (AppliedDamage > 0.0f)
	{
		OnDamageReceived.Broadcast(AppliedDamage, bWasCrit, Kind);
		OnAnyDamageReceived.Broadcast(GetOwner(), AppliedDamage, bWasCrit, Kind);

		if (USkillComponent* Skills = GetOwner() ? GetOwner()->FindComponentByClass<USkillComponent>() : nullptr)
		{
			Skills->AddGauge(Skills->GetGaugeGainOnTakeDamage());
		}
	}

	if (CurrentHp <= 0.0f && !bDeathBroadcast)
	{
		bDeathBroadcast = true;
		OnDeath.Broadcast(GetOwner());
	}
}

bool UCombatComponent::IsDead() const
{
	return CurrentHp <= 0.0f;
}
