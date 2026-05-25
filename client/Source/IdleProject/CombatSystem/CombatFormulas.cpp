#include "CombatSystem/CombatFormulas.h"

float FCombatFormulas::ComputeDamage(float Atk, float Def)
{
	return FMath::Max(Atk * 0.05f, Atk - Def * 0.6f);
}

float FCombatFormulas::ComputeDamage(float Atk, float Def, float CritRate, float CritDmg)
{
	const float BaseDamage = ComputeDamage(Atk, Def);
	const float SafeCritRate = FMath::Clamp(CritRate, 0.0f, 1.0f);
	const float SafeCritDmg = FMath::Max(1.0f, CritDmg);
	return BaseDamage * (1.0f + SafeCritRate * (SafeCritDmg - 1.0f));
}

float FCombatFormulas::ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float Def)
{
	const float AttackPower = ClassId == EClassId::Mage ? AttackerStats.MagicAtk : AttackerStats.PhysAtk;
	if (ClassId != EClassId::Archer)
	{
		return ComputeDamage(AttackPower, Def);
	}

	return ComputeDamage(AttackPower, Def, AttackerStats.CritRate, AttackerStats.CritDmg);
}

float FCombatFormulas::ComputeAttackPower(const FDerivedStats& AttackerStats, EClassId ClassId)
{
	if (ClassId == EClassId::Mage)
	{
		return AttackerStats.MagicAtk;
	}

	if (ClassId == EClassId::Archer)
	{
		return AttackerStats.PhysAtk;
	}

	return AttackerStats.PhysAtk;
}
