#include "CombatSystem/CombatFormulas.h"

float FCombatFormulas::ComputeDamage(float Atk, float Def)
{
	return FMath::Max(Atk * 0.05f, Atk - Def * 0.6f);
}

float FCombatFormulas::ComputeMagicDamage(float MagicAtk, float MagicDef)
{
	return ComputeDamage(MagicAtk, MagicDef);
}

bool FCombatFormulas::RollCrit(float CritRate, FRandomStream& RandomStream)
{
	return RandomStream.GetFraction() < FMath::Clamp(CritRate, 0.0f, 1.0f);
}

float FCombatFormulas::ApplyCrit(float BaseDamage, bool bIsCrit, float CritDmg)
{
	return bIsCrit ? BaseDamage * FMath::Max(1.0f, CritDmg) : BaseDamage;
}

float FCombatFormulas::ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float Def)
{
	return ComputeDamage(AttackerStats, ClassId, Def, Def);
}

float FCombatFormulas::ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float PhysDef, float MagicDef)
{
	return ClassId == EClassId::Mage
		? ComputeMagicDamage(AttackerStats.MagicAtk, MagicDef)
		: ComputeDamage(AttackerStats.PhysAtk, PhysDef);
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
