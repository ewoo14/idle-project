#include "CharacterSystem/StatFormulas.h"

namespace
{
struct FClassGrowth
{
	FPrimaryStats InitialBonus;
	FPrimaryStats LevelBonus;
};

const FClassGrowth& GetClassGrowth(EClassId ClassId)
{
	static const FClassGrowth Warrior{
		{11.0f, 5.0f, 2.0f, 2.0f, 9.0f, 3.0f},
		{1.4f, 0.4f, 0.15f, 0.15f, 1.1f, 0.25f}
	};
	static const FClassGrowth Mage{
		{2.0f, 3.0f, 11.0f, 9.0f, 4.0f, 3.0f},
		{0.15f, 0.25f, 1.4f, 1.1f, 0.35f, 0.25f}
	};
	static const FClassGrowth Archer{
		{4.0f, 11.0f, 3.0f, 3.0f, 5.0f, 9.0f},
		{0.35f, 1.4f, 0.2f, 0.2f, 0.4f, 1.1f}
	};
	static const FClassGrowth Thief{
		{3.0f, 10.0f, 3.0f, 3.0f, 4.0f, 11.0f},
		{0.25f, 1.25f, 0.2f, 0.2f, 0.35f, 1.3f}
	};
	static const FClassGrowth Cleric{
		{2.0f, 3.0f, 7.0f, 11.0f, 5.0f, 4.0f},
		{0.15f, 0.2f, 1.0f, 1.4f, 0.45f, 0.3f}
	};

	switch (ClassId)
	{
	case EClassId::Mage:
		return Mage;
	case EClassId::Archer:
		return Archer;
	case EClassId::Thief:
		return Thief;
	case EClassId::Cleric:
		return Cleric;
	case EClassId::Warrior:
	default:
		return Warrior;
	}
}

float RoundStat(float Value)
{
	return FMath::RoundToFloat(Value * 10.0f) / 10.0f;
}

float RoundRate(float Value)
{
	return FMath::RoundToFloat(Value * 1000.0f) / 1000.0f;
}

float RoundWhole(float Value)
{
	return static_cast<float>(FMath::RoundToInt(Value));
}
}

FPrimaryStats FStatFormulas::DefaultPrimaryStats(EClassId ClassId, int32 Level)
{
	const int32 SafeLevel = FMath::Max(Level, 1);
	const FClassGrowth& Growth = GetClassGrowth(ClassId);
	const float SharedGrowth = 1.0f + static_cast<float>(SafeLevel - 1) * 0.5f;
	const float LevelIndex = static_cast<float>(SafeLevel - 1);

	FPrimaryStats Result;
	Result.Str = RoundStat(SharedGrowth + Growth.InitialBonus.Str + LevelIndex * Growth.LevelBonus.Str);
	Result.Dex = RoundStat(SharedGrowth + Growth.InitialBonus.Dex + LevelIndex * Growth.LevelBonus.Dex);
	Result.Int_ = RoundStat(SharedGrowth + Growth.InitialBonus.Int_ + LevelIndex * Growth.LevelBonus.Int_);
	Result.Wis = RoundStat(SharedGrowth + Growth.InitialBonus.Wis + LevelIndex * Growth.LevelBonus.Wis);
	Result.Con = RoundStat(SharedGrowth + Growth.InitialBonus.Con + LevelIndex * Growth.LevelBonus.Con);
	Result.Luk = RoundStat(SharedGrowth + Growth.InitialBonus.Luk + LevelIndex * Growth.LevelBonus.Luk);
	return Result;
}

FDerivedStats FStatFormulas::DeriveStats(const FPrimaryStats& Primary, int32 Level, const FDerivedStats& EquipmentBonus)
{
	const int32 SafeLevel = FMath::Max(Level, 1);

	FDerivedStats Result;
	Result.Hp = RoundWhole(Primary.Con * 10.0f + static_cast<float>(SafeLevel) * 20.0f + EquipmentBonus.Hp);
	Result.Mp = RoundWhole(Primary.Wis * 5.0f + Primary.Int_ * 2.0f + static_cast<float>(SafeLevel) * 4.0f + 5.0f + EquipmentBonus.Mp);
	Result.PhysAtk = RoundWhole(Primary.Str * 2.0f + EquipmentBonus.PhysAtk);
	Result.MagicAtk = RoundWhole(Primary.Int_ * 2.0f + Primary.Wis * 0.5f + EquipmentBonus.MagicAtk);
	Result.PhysDef = RoundWhole(Primary.Con * 1.5f + Primary.Dex * 0.25f + EquipmentBonus.PhysDef);
	Result.MagicDef = RoundWhole(Primary.Wis * 1.5f + Primary.Int_ * 0.25f + EquipmentBonus.MagicDef);
	Result.AtkSpeed = RoundStat(FMath::Clamp(Primary.Dex * 0.004f + 1.0f + EquipmentBonus.AtkSpeed, 0.5f, 3.0f));
	Result.MoveSpeed = RoundStat(FMath::Clamp(Primary.Dex * 0.002f + 1.0f + EquipmentBonus.MoveSpeed, 0.5f, 3.0f));
	Result.CritRate = RoundRate(FMath::Clamp(Primary.Luk * 0.002f + EquipmentBonus.CritRate, 0.0f, 1.0f));
	Result.CritDmg = RoundStat(FMath::Clamp(1.5f + Primary.Luk * 0.001f + EquipmentBonus.CritDmg, 1.0f, 3.0f));
	Result.Dodge = RoundRate(FMath::Clamp(Primary.Dex * 0.0015f + Primary.Luk * 0.001f + EquipmentBonus.Dodge, 0.0f, 1.0f));
	Result.Accuracy = RoundRate(FMath::Clamp(0.75f + Primary.Dex * 0.002f + EquipmentBonus.Accuracy, 0.0f, 1.0f));
	return Result;
}
