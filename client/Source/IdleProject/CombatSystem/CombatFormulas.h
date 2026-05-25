#pragma once

#include "CoreMinimal.h"
#include "CharacterSystem/StatFormulas.h"

/** 서버 combat.ts와 동일한 전투 수식을 담는 정적 유틸리티입니다. */
struct IDLEPROJECT_API FCombatFormulas
{
	static float ComputeDamage(float Atk, float Def);
	static float ComputeDamage(float Atk, float Def, float CritRate, float CritDmg);
	static float ComputeDamage(const FDerivedStats& AttackerStats, EClassId ClassId, float Def);
	static float ComputeAttackPower(const FDerivedStats& AttackerStats, EClassId ClassId);
};
