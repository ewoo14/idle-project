#include "RuneSystem/ClassRuneFormula.h"

#include "RuneSystem/RuneFormula.h"

namespace
{
bool IsValidClassId(EClassId ClassId)
{
	return ClassId >= EClassId::Warrior && ClassId <= EClassId::Summoner;
}

FName MakeClassRuneId(EClassId ClassId, EItemRarity Rarity, int32 ProgressIndex)
{
	return FName(*FString::Printf(
		TEXT("class_rune_%d_%d_%d"),
		static_cast<int32>(ClassId),
		static_cast<int32>(Rarity),
		FMath::Max(0, ProgressIndex)));
}

EItemRarity ClampClassRuneRarity(EItemRarity Rarity)
{
	return Rarity >= EItemRarity::Common && Rarity <= EItemRarity::Mythic ? Rarity : EItemRarity::Common;
}
}

FRuneCoreMultipliers FClassRuneFormula::GetClassMasteryMultipliers(EClassId ClassId, EItemRarity Rarity, int32 EnhanceLevel)
{
	FRuneCoreMultipliers Result;
	Result.PhysAtk = 0.0f;
	Result.MagicAtk = 0.0f;
	Result.PhysDef = 0.0f;
	Result.MagicDef = 0.0f;
	Result.Hp = 0.0f;

	if (!IsValidClassId(ClassId))
	{
		return Result;
	}

	const float Unit = FRuneFormula::GetCoreRuneMultiplier(Rarity, EnhanceLevel);
	if (Unit <= 0.0f)
	{
		return Result;
	}

	switch (ClassId)
	{
	case EClassId::Warrior:
		Result.PhysAtk = Unit;
		Result.PhysDef = Unit;
		break;
	case EClassId::Mage:
		Result.MagicAtk = Unit;
		break;
	case EClassId::Archer:
	case EClassId::Thief:
	case EClassId::Berserker:
		Result.PhysAtk = Unit;
		break;
	case EClassId::Cleric:
		Result.MagicAtk = Unit;
		Result.Hp = Unit;
		break;
	case EClassId::Paladin:
		Result.PhysDef = Unit;
		Result.Hp = Unit;
		break;
	case EClassId::Summoner:
		Result.MagicAtk = Unit;
		break;
	default:
		break;
	}

	return Result;
}

int64 FClassRuneFormula::GetClassRuneCraftCost(EItemRarity Rarity)
{
	switch (ClampClassRuneRarity(Rarity))
	{
	case EItemRarity::Common:
		return 25;
	case EItemRarity::Rare:
		return 60;
	case EItemRarity::Epic:
		return 150;
	case EItemRarity::Unique:
		return 400;
	case EItemRarity::Legendary:
		return 1000;
	case EItemRarity::Transcendent:
		return 1700;
	case EItemRarity::Mythic:
		return 2500;
	default:
		return 25;
	}
}

FRuneInstance FClassRuneFormula::MakeClassRune(EClassId ClassId, EItemRarity Rarity, int32 ProgressIndex)
{
	FRuneInstance Rune;
	if (!IsValidClassId(ClassId))
	{
		return Rune;
	}

	Rune.RuneType = ERuneType::ClassMastery;
	Rune.Rarity = ClampClassRuneRarity(Rarity);
	Rune.EnhanceLevel = 0;
	Rune.ClassRestriction = ClassId;
	Rune.RuneId = MakeClassRuneId(ClassId, Rune.Rarity, ProgressIndex);
	return Rune;
}

bool FClassRuneFormula::RollClassRuneDrop(int32 MonsterLevel, bool bIsBoss, EClassId ClassId, FRandomStream& Rng, FRuneInstance& OutRune)
{
	const float DropChance = bIsBoss ? 0.05f : 0.005f;
	if (!IsValidClassId(ClassId) || Rng.FRand() >= DropChance)
	{
		OutRune = FRuneInstance();
		return false;
	}

	EItemRarity Rarity = EItemRarity::Common;
	FRuneInstance RegularRune = FRuneFormula::RollShopRune(MonsterLevel, Rng);
	if (RegularRune.Rarity >= EItemRarity::Common && RegularRune.Rarity <= EItemRarity::Mythic)
	{
		Rarity = RegularRune.Rarity;
	}

	OutRune = MakeClassRune(ClassId, Rarity, MonsterLevel);
	return true;
}
