#pragma once

#include "CoreMinimal.h"

/** 서버 combat.ts와 동일한 전투 수식을 담는 정적 유틸리티입니다. */
struct IDLEPROJECT_API FCombatFormulas
{
	static float ComputeDamage(float Atk, float Def);
};
