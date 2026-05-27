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

float FCombatFormulas::ComputeElementMultiplier(ESkillElement SkillElement, ESkillElement TargetWeakElement)
{
	if (SkillElement == ESkillElement::None || TargetWeakElement == ESkillElement::None)
	{
		return 1.0f;
	}

	if (SkillElement == TargetWeakElement)
	{
		return 1.5f;
	}

	const bool bResisted =
		(SkillElement == ESkillElement::Fire && TargetWeakElement == ESkillElement::Ice) ||
		(SkillElement == ESkillElement::Ice && TargetWeakElement == ESkillElement::Fire) ||
		(SkillElement == ESkillElement::Lightning && TargetWeakElement == ESkillElement::Holy) ||
		(SkillElement == ESkillElement::Holy && TargetWeakElement == ESkillElement::Lightning);
	return bResisted ? 0.5f : 1.0f;
}

float FCombatFormulas::ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float Def)
{
	return ComputeDamage(AttackerStats, ClassId, Def, Def);
}

float FCombatFormulas::ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float PhysDef, float MagicDef)
{
	return (ClassId == EClassId::Mage || ClassId == EClassId::Cleric || ClassId == EClassId::Summoner)
		? ComputeMagicDamage(AttackerStats.MagicAtk, MagicDef)
		: ComputeDamage(AttackerStats.PhysAtk, PhysDef);
}

float FCombatFormulas::ComputeAttackPower(const FDerivedStats& AttackerStats, EClassId ClassId)
{
	if (ClassId == EClassId::Mage || ClassId == EClassId::Cleric || ClassId == EClassId::Summoner)
	{
		return AttackerStats.MagicAtk;
	}

	if (ClassId == EClassId::Archer || ClassId == EClassId::Thief)
	{
		return AttackerStats.PhysAtk;
	}

	return AttackerStats.PhysAtk;
}
