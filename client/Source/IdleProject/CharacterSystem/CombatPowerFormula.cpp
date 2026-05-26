#include "CharacterSystem/CombatPowerFormula.h"

int64 FCombatPowerFormula::ComputeCombatPower(const FDerivedStats& Stats)
{
	const double RawCombatPower =
		static_cast<double>(Stats.PhysAtk)
		+ static_cast<double>(Stats.MagicAtk)
		+ static_cast<double>(Stats.PhysDef) * 2.0
		+ static_cast<double>(Stats.MagicDef) * 2.0
		+ static_cast<double>(Stats.Hp) * 0.1
		+ static_cast<double>(Stats.CritRate) * 500.0
		+ static_cast<double>(Stats.CritDmg) * 100.0
		+ static_cast<double>(Stats.AtkSpeed) * 200.0;

	return FMath::Max<int64>(0, FMath::RoundToInt64(RawCombatPower));
}
