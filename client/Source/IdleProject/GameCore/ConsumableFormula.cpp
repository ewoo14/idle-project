#include "GameCore/ConsumableFormula.h"

float FConsumableFormula::GetGradeMultiplier(EConsumableGrade Grade)
{
	switch (Grade)
	{
	case EConsumableGrade::Lesser:
		return 0.5f;
	case EConsumableGrade::Standard:
		return 1.0f;
	case EConsumableGrade::Greater:
		return 2.0f;
	default:
		return 1.0f;
	}
}

float FConsumableFormula::GetBuffPercent(EConsumableType Type)
{
	return GetBuffPercent(Type, EConsumableGrade::Standard);
}

float FConsumableFormula::GetBuffPercent(EConsumableType Type, EConsumableGrade Grade)
{
	if (!IsValidGrade(Grade))
	{
		return 0.0f;
	}

	float Base = 0.0f;
	switch (Type)
	{
	case EConsumableType::AttackTonic:
	case EConsumableType::GuardTonic:
	case EConsumableType::FortuneScroll:
		Base = 0.30f;
		break;
	case EConsumableType::AllStatElixir:
		Base = 0.20f;
		break;
	case EConsumableType::GoldFeast:
	case EConsumableType::WisdomBooster:
		Base = 0.50f;
		break;
	default:
		return 0.0f;
	}

	return Base * GetGradeMultiplier(Grade);
}

int64 FConsumableFormula::GetBuffDurationSec(EConsumableType Type)
{
	return IsValidType(Type) ? BuffDurationSec : 0;
}

int64 FConsumableFormula::GetBuffDurationSec(EConsumableType Type, EConsumableGrade Grade)
{
	// 지속시간은 등급과 무관하게 타입별 고정값을 유지합니다.
	return (IsValidType(Type) && IsValidGrade(Grade)) ? BuffDurationSec : 0;
}

bool FConsumableFormula::IsValidType(EConsumableType Type)
{
	switch (Type)
	{
	case EConsumableType::AttackTonic:
	case EConsumableType::GuardTonic:
	case EConsumableType::AllStatElixir:
	case EConsumableType::FortuneScroll:
	case EConsumableType::GoldFeast:
	case EConsumableType::WisdomBooster:
		return true;
	default:
		return false;
	}
}

bool FConsumableFormula::IsValidGrade(EConsumableGrade Grade)
{
	switch (Grade)
	{
	case EConsumableGrade::Lesser:
	case EConsumableGrade::Standard:
	case EConsumableGrade::Greater:
		return true;
	default:
		return false;
	}
}
