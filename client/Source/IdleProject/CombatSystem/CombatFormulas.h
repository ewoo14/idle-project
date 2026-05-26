#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/StatusElementTypes.h"

/** 서버 combat.ts와 동일한 전투 수식을 담는 정적 유틸리티입니다. */
struct IDLEPROJECT_API FCombatFormulas
{
	static float ComputeDamage(float Atk, float Def);
	static float ComputeMagicDamage(float MagicAtk, float MagicDef);
	static bool RollCrit(float CritRate, FRandomStream& RandomStream);
	static float ApplyCrit(float BaseDamage, bool bIsCrit, float CritDmg);
	static float ComputeElementMultiplier(ESkillElement SkillElement, ESkillElement TargetWeakElement);
	static float ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float Def);
	static float ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float PhysDef, float MagicDef);
	static float ComputeAttackPower(const FDerivedStats& AttackerStats, EClassId ClassId);
};
