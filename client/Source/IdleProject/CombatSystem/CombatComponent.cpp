#include "CombatSystem/CombatComponent.h"

#include "CombatSystem/SkillComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::InitializeCombat(float InMaxHp, float InAtk, float InDef, float InAtkSpeed, float InCritRate, float InCritDmg)
{
	MaxHp = FMath::Max(1.0f, InMaxHp);
	CurrentHp = MaxHp;
	Atk = FMath::Max(0.0f, InAtk);
	Def = FMath::Max(0.0f, InDef);
	AtkSpeed = FMath::Max(0.1f, InAtkSpeed);
	CritRate = FMath::Clamp(InCritRate, 0.0f, 1.0f);
	CritDmg = FMath::Max(1.0f, InCritDmg);
	bDeathBroadcast = false;
	OnHpChanged.Broadcast(CurrentHp);
}

void UCombatComponent::TakeDamage(float Damage, AActor* Instigator)
{
	if (IsDead())
	{
		return;
	}

	CurrentHp = FMath::Clamp(CurrentHp - FMath::Max(0.0f, Damage), 0.0f, MaxHp);
	OnHpChanged.Broadcast(CurrentHp);

	if (Damage > 0.0f)
	{
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
