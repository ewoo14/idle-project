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
	ActiveStatuses.Reset();
	LastAppliedStatusAtkSpeedPenalty = 0.0f;
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

	// 저주(Curse): 받는 피해 증폭 디버프. 단일 적용 지점에서 Magnitude 비율만큼 증폭합니다.
	const float CurseMagnitude = GetActiveStatusMagnitude(ESkillStatusEffect::Curse);
	const float AmplifiedDamage = Damage * (1.0f + FMath::Max(0.0f, CurseMagnitude));
	const float AppliedDamage = FMath::Max(0.0f, AmplifiedDamage);
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

void UCombatComponent::ApplyStatus(ESkillStatusEffect Type, float Duration, float Magnitude, float Now)
{
	if (Type == ESkillStatusEffect::None || Duration <= 0.0f || Magnitude <= 0.0f || IsDead())
	{
		return;
	}

	if (Type == ESkillStatusEffect::Freeze)
	{
		RemoveStatusAttackSpeedSlow();
	}

	ActiveStatuses.RemoveAll([Type](const FActiveSkillStatus& Status)
	{
		return Status.Type == Type;
	});

	FActiveSkillStatus Status;
	Status.Type = Type;
	Status.EndTime = Now + Duration;
	Status.Magnitude = Magnitude;
	Status.NextTickTime = Now + 1.0f;
	ActiveStatuses.Add(Status);

	if (Type == ESkillStatusEffect::Freeze)
	{
		ApplyCurrentStatusAttackSpeedSlow(Now);
	}
}

void UCombatComponent::TickStatuses(float Now)
{
	if (IsDead())
	{
		return;
	}

	RemoveStatusAttackSpeedSlow();

	for (FActiveSkillStatus& Status : ActiveStatuses)
	{
		while ((Status.Type == ESkillStatusEffect::Poison || Status.Type == ESkillStatusEffect::Burn) &&
			Now >= Status.NextTickTime &&
			Status.NextTickTime < Status.EndTime &&
			!IsDead())
		{
			TakeDamageTyped(Status.Magnitude, GetOwner(), false, EDamageKind::Magic);
			Status.NextTickTime += 1.0f;
		}
	}

	ActiveStatuses.RemoveAll([Now](const FActiveSkillStatus& Status)
	{
		return Now >= Status.EndTime;
	});

	ApplyCurrentStatusAttackSpeedSlow(Now);
}

bool UCombatComponent::HasActiveStatus(ESkillStatusEffect Type) const
{
	return ActiveStatuses.ContainsByPredicate([Type](const FActiveSkillStatus& Status)
	{
		return Status.Type == Type;
	});
}

float UCombatComponent::GetActiveStatusMagnitude(ESkillStatusEffect Type) const
{
	float MaxMagnitude = 0.0f;
	for (const FActiveSkillStatus& Status : ActiveStatuses)
	{
		if (Status.Type == Type)
		{
			MaxMagnitude = FMath::Max(MaxMagnitude, Status.Magnitude);
		}
	}
	return MaxMagnitude;
}

void UCombatComponent::RemoveStatusAttackSpeedSlow()
{
	if (LastAppliedStatusAtkSpeedPenalty != 0.0f)
	{
		AtkSpeed = FMath::Max(0.1f, AtkSpeed + LastAppliedStatusAtkSpeedPenalty);
		LastAppliedStatusAtkSpeedPenalty = 0.0f;
	}
}

void UCombatComponent::ApplyCurrentStatusAttackSpeedSlow(float Now)
{
	float SlowMagnitude = 0.0f;
	for (const FActiveSkillStatus& Status : ActiveStatuses)
	{
		if (Status.Type == ESkillStatusEffect::Freeze && Status.EndTime > Now)
		{
			SlowMagnitude = FMath::Max(SlowMagnitude, Status.Magnitude);
		}
	}

	if (SlowMagnitude > 0.0f)
	{
		LastAppliedStatusAtkSpeedPenalty = AtkSpeed * FMath::Clamp(SlowMagnitude, 0.0f, 0.9f);
		AtkSpeed = FMath::Max(0.1f, AtkSpeed - LastAppliedStatusAtkSpeedPenalty);
	}
}
