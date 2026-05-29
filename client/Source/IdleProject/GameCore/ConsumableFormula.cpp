#include "GameCore/ConsumableFormula.h"

float FConsumableFormula::GetBuffPercent(EConsumableType Type)
{
	switch (Type)
	{
	case EConsumableType::AttackTonic:
	case EConsumableType::GuardTonic:
	case EConsumableType::FortuneScroll:
		return 0.30f;
	case EConsumableType::AllStatElixir:
		return 0.20f;
	case EConsumableType::GoldFeast:
	case EConsumableType::WisdomBooster:
		return 0.50f;
	default:
		return 0.0f;
	}
}

int64 FConsumableFormula::GetBuffDurationSec(EConsumableType Type)
{
	return IsValidType(Type) ? BuffDurationSec : 0;
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
