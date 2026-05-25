#include "CombatSystem/CombatFormulas.h"

float FCombatFormulas::ComputeDamage(float Atk, float Def)
{
	return FMath::Max(Atk * 0.05f, Atk - Def * 0.6f);
}
