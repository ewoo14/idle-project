#include "RuneSystem/RuneSetFormula.h"

namespace
{
void ResetCoreBonus(FRuneCoreMultipliers& Core)
{
	Core.PhysAtk = 0.0f;
	Core.MagicAtk = 0.0f;
	Core.PhysDef = 0.0f;
	Core.MagicDef = 0.0f;
	Core.Hp = 0.0f;
}

void ResetUtilBonus(FRuneUtilValues& Util)
{
	Util.CritDamage = 0.0f;
	Util.GoldFind = 0.0f;
	Util.ExpBoost = 0.0f;
	Util.OfflineEff = 0.0f;
}
}

float FRuneSetFormula::GetSetTierBonus(int32 EquippedCount)
{
	if (EquippedCount >= Tier3Count)
	{
		return Tier3Bonus;
	}
	if (EquippedCount >= Tier2Count)
	{
		return Tier2Bonus;
	}
	if (EquippedCount >= Tier1Count)
	{
		return Tier1Bonus;
	}
	return 0.0f;
}

void FRuneSetFormula::ComputeSetBonus(const TMap<ERuneSet, int32>& SetCounts, FRuneCoreMultipliers& OutCore, FRuneUtilValues& OutUtil)
{
	ResetCoreBonus(OutCore);
	ResetUtilBonus(OutUtil);

	for (const TPair<ERuneSet, int32>& Pair : SetCounts)
	{
		const float Bonus = GetSetTierBonus(Pair.Value);
		if (Bonus <= 0.0f)
		{
			continue;
		}

		switch (Pair.Key)
		{
		case ERuneSet::Offense:
			OutCore.PhysAtk += Bonus;
			OutCore.MagicAtk += Bonus;
			break;
		case ERuneSet::Bastion:
			OutCore.PhysDef += Bonus;
			OutCore.MagicDef += Bonus;
			break;
		case ERuneSet::Vitality:
			OutCore.Hp += Bonus;
			OutUtil.OfflineEff += Bonus;
			break;
		case ERuneSet::Fortune:
			OutUtil.GoldFind += Bonus;
			OutUtil.ExpBoost += Bonus;
			OutUtil.CritDamage += Bonus;
			break;
		case ERuneSet::None:
		default:
			break;
		}
	}
}

ERuneSet FRuneSetFormula::RollRuneSet(EItemRarity Rarity, FRandomStream& Rng)
{
	if (Rarity == EItemRarity::None || Rarity == EItemRarity::Common)
	{
		return ERuneSet::None;
	}

	return static_cast<ERuneSet>(Rng.RandRange(static_cast<int32>(ERuneSet::None), static_cast<int32>(ERuneSet::Fortune)));
}
