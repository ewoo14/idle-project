#pragma once

#include "CoreMinimal.h"
#include "GameCore/ConsumableTypes.h"

struct IDLEPROJECT_API FConsumableFormula
{
	static constexpr int64 BuffDurationSec = 1800;

	/** 등급별 효과 % 배수: Lesser=0.5x, Standard=1.0x, Greater=2.0x. */
	static float GetGradeMultiplier(EConsumableGrade Grade);

	/** Standard 기준 #73 기본 효과 %. */
	static float GetBuffPercent(EConsumableType Type);
	/** 등급별 효과 % (Standard=기본, Lesser=×0.5, Greater=×2.0). */
	static float GetBuffPercent(EConsumableType Type, EConsumableGrade Grade);

	/** 지속시간은 등급 무관 타입별 고정값입니다. */
	static int64 GetBuffDurationSec(EConsumableType Type);
	static int64 GetBuffDurationSec(EConsumableType Type, EConsumableGrade Grade);

	static bool IsValidType(EConsumableType Type);
	static bool IsValidGrade(EConsumableGrade Grade);
};
