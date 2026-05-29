#include "RuneSystem/RuneFormula.h"

#include "RuneSystem/RuneSetFormula.h"

namespace
{
struct FRuneRarityTuning
{
	float CoreBase = 0.0f;
	float CoreStep = 0.0f;
	float UtilBase = 0.0f;
	int64 DisenchantBase = 0;
};

FRuneRarityTuning GetRarityTuning(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Common:
		return {0.02f, 0.004f, 0.02f, 1};
	case EItemRarity::Rare:
		return {0.05f, 0.009f, 0.04f, 5};
	case EItemRarity::Epic:
		return {0.08f, 0.014f, 0.06f, 12};
	case EItemRarity::Unique:
		return {0.10f, 0.017f, 0.075f, 20};
	case EItemRarity::Legendary:
		return {0.12f, 0.02f, 0.09f, 30};
	case EItemRarity::Transcendent:
		return {0.15f, 0.025f, 0.105f, 50};
	case EItemRarity::Mythic:
		return {0.18f, 0.03f, 0.12f, 80};
	default:
		return {};
	}
}

float GetUtilStep(ERuneType Type)
{
	switch (Type)
	{
	case ERuneType::CritDamage:
	case ERuneType::GoldFind:
	case ERuneType::ExpBoost:
		return 0.005f;
	case ERuneType::OfflineEff:
		return 0.003f;
	default:
		return 0.0f;
	}
}

EItemRarity RollRuneRarity(int32 ProgressIndex, FRandomStream& Rng)
{
	const float Roll = Rng.FRand();
	const float MythicChance = ProgressIndex >= 100 ? 0.005f : 0.0f;
	if (Roll < MythicChance)
	{
		return EItemRarity::Mythic;
	}
	const float TranscendentChance = ProgressIndex >= 100 ? 0.003f : 0.0f;
	if (Roll < MythicChance + TranscendentChance)
	{
		return EItemRarity::Transcendent;
	}
	if (Roll < MythicChance + TranscendentChance + 0.002f)
	{
		return EItemRarity::Legendary;
	}
	if (Roll < MythicChance + TranscendentChance + 0.012f)
	{
		return EItemRarity::Unique;
	}
	if (Roll < MythicChance + TranscendentChance + 0.030f)
	{
		return EItemRarity::Epic;
	}
	if (Roll < MythicChance + TranscendentChance + 0.180f)
	{
		return EItemRarity::Rare;
	}
	return EItemRarity::Common;
}

ERuneType RollRuneType(FRandomStream& Rng)
{
	return static_cast<ERuneType>(Rng.RandRange(static_cast<int32>(ERuneType::PhysAtk), static_cast<int32>(ERuneType::OfflineEff)));
}

FName MakeRuneId(ERuneType Type, EItemRarity Rarity, int32 ProgressIndex)
{
	return FName(*FString::Printf(TEXT("rune_%d_%d_%d"), static_cast<int32>(Type), static_cast<int32>(Rarity), FMath::Max(0, ProgressIndex)));
}

// 서버 rune.ts parity 미러. 레어도 정수(None=0, Common=1 .. Mythic=7) 인덱싱. Mythic(7) 상승 비용/확률=0.
constexpr int64 RuneRerollSetEssenceByRarity[8] = {0, 20, 50, 100, 200, 400, 800, 1500};
constexpr int64 RuneRarityUpgradeEssenceByRarity[8] = {0, 100, 250, 600, 1500, 4000, 10000, 0};
constexpr int64 RuneRarityUpgradeGoldByRarity[8] = {0, 5000, 15000, 50000, 150000, 500000, 1500000, 0};
constexpr float RuneRarityUpgradeChanceByRarity[8] = {0.0f, 0.6f, 0.45f, 0.3f, 0.2f, 0.12f, 0.05f, 0.0f};
constexpr int64 RuneTransferEssenceBase = 50;
constexpr int64 RuneTransferEssenceStep = 25;

bool RuneIsValidRarity(EItemRarity Rarity)
{
	const int32 Value = static_cast<int32>(Rarity);
	return Value >= static_cast<int32>(EItemRarity::Common) && Value <= static_cast<int32>(EItemRarity::Mythic);
}

int64 SquareCost(int64 BaseCost, int32 CurrentLevel)
{
	const int64 NextLevel = static_cast<int64>(FMath::Max(0, CurrentLevel)) + 1;
	if (NextLevel > 0 && NextLevel > MAX_int64 / NextLevel)
	{
		return MAX_int64;
	}
	const int64 Squared = NextLevel * NextLevel;
	return BaseCost > 0 && Squared > MAX_int64 / BaseCost ? MAX_int64 : BaseCost * Squared;
}
}

bool FRuneFormula::IsCoreType(ERuneType Type)
{
	return Type >= ERuneType::PhysAtk && Type <= ERuneType::Hp;
}

bool FRuneFormula::IsUtilType(ERuneType Type)
{
	return Type >= ERuneType::CritDamage && Type <= ERuneType::OfflineEff;
}

float FRuneFormula::GetCoreRuneMultiplier(EItemRarity Rarity, int32 EnhanceLevel)
{
	const FRuneRarityTuning Tuning = GetRarityTuning(Rarity);
	if (Tuning.CoreBase <= 0.0f)
	{
		return 0.0f;
	}
	return Tuning.CoreBase + static_cast<float>(FMath::Max(0, EnhanceLevel)) * Tuning.CoreStep;
}

float FRuneFormula::GetUtilRuneValue(ERuneType Type, EItemRarity Rarity, int32 EnhanceLevel)
{
	if (!IsUtilType(Type))
	{
		return 0.0f;
	}

	const FRuneRarityTuning Tuning = GetRarityTuning(Rarity);
	if (Tuning.UtilBase <= 0.0f)
	{
		return 0.0f;
	}

	const float RawValue = Tuning.UtilBase + static_cast<float>(FMath::Max(0, EnhanceLevel)) * GetUtilStep(Type);
	return FMath::Min(RawValue, GetUtilCap(Type));
}

float FRuneFormula::GetUtilCap(ERuneType Type)
{
	switch (Type)
	{
	case ERuneType::CritDamage:
		return 1.0f;
	case ERuneType::GoldFind:
	case ERuneType::ExpBoost:
		return 2.0f;
	case ERuneType::OfflineEff:
		return 0.5f;
	default:
		return 0.0f;
	}
}

int64 FRuneFormula::GetEnhanceEssenceCost(int32 CurrentLevel)
{
	return SquareCost(10, CurrentLevel);
}

int64 FRuneFormula::GetEnhanceGoldCost(int32 CurrentLevel)
{
	return SquareCost(1000, CurrentLevel);
}

int64 FRuneFormula::GetDisenchantEssence(EItemRarity Rarity, int32 EnhanceLevel)
{
	const FRuneRarityTuning Tuning = GetRarityTuning(Rarity);
	if (Tuning.DisenchantBase <= 0)
	{
		return 0;
	}
	return Tuning.DisenchantBase + static_cast<int64>(FMath::Max(0, EnhanceLevel)) * 2;
}

int64 FRuneFormula::GetRerollSetEssenceCost(EItemRarity Rarity)
{
	return RuneIsValidRarity(Rarity) ? RuneRerollSetEssenceByRarity[static_cast<int32>(Rarity)] : 0;
}

int64 FRuneFormula::GetRarityUpgradeEssenceCost(EItemRarity Rarity)
{
	return RuneIsValidRarity(Rarity) ? RuneRarityUpgradeEssenceByRarity[static_cast<int32>(Rarity)] : 0;
}

int64 FRuneFormula::GetRarityUpgradeGoldCost(EItemRarity Rarity)
{
	return RuneIsValidRarity(Rarity) ? RuneRarityUpgradeGoldByRarity[static_cast<int32>(Rarity)] : 0;
}

float FRuneFormula::GetRarityUpgradeChance(EItemRarity Rarity)
{
	// 서버 Math.fround parity: 상수 테이블이 이미 float 표현이므로 그대로 반환(C++ float = IEEE754 single).
	return RuneIsValidRarity(Rarity) ? RuneRarityUpgradeChanceByRarity[static_cast<int32>(Rarity)] : 0.0f;
}

int64 FRuneFormula::GetTransferEssenceCost(int32 SourceLevel)
{
	return RuneTransferEssenceBase + static_cast<int64>(FMath::Max(0, SourceLevel)) * RuneTransferEssenceStep;
}

bool FRuneFormula::RollRuneDrop(int32 MonsterLevel, bool bIsBoss, FRandomStream& Rng, FRuneInstance& OutRune)
{
	const float DropChance = bIsBoss ? 0.25f : 0.02f;
	if (Rng.FRand() >= DropChance)
	{
		OutRune = FRuneInstance();
		return false;
	}

	OutRune = RollShopRune(MonsterLevel, Rng);
	return true;
}

int64 FRuneFormula::GetShopRuneRollCost(int32 ProgressIndex)
{
	const double Cost = 5000.0 * (1.0 + static_cast<double>(FMath::Max(0, ProgressIndex)) * 0.1);
	return FMath::Max<int64>(0, FMath::RoundToInt64(Cost));
}

FRuneInstance FRuneFormula::RollShopRune(int32 ProgressIndex, FRandomStream& Rng)
{
	FRuneInstance Rune;
	Rune.RuneType = RollRuneType(Rng);
	Rune.Rarity = RollRuneRarity(FMath::Max(0, ProgressIndex), Rng);
	Rune.EnhanceLevel = 0;
	Rune.RuneId = MakeRuneId(Rune.RuneType, Rune.Rarity, ProgressIndex);
	Rune.RuneSet = FRuneSetFormula::RollRuneSet(Rune.Rarity, Rng);
	return Rune;
}
