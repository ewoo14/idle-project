#include "ItemSystem/PotentialFormula.h"

namespace
{
int32 GetGradeIndex(EPotentialGrade Grade)
{
	return static_cast<int32>(Grade);
}

EPotentialGrade GradeFromIndex(int32 Index)
{
	switch (Index)
	{
	case 1:
		return EPotentialGrade::Rare;
	case 2:
		return EPotentialGrade::Epic;
	case 3:
		return EPotentialGrade::Unique;
	case 4:
		return EPotentialGrade::Legendary;
	case 0:
	default:
		return EPotentialGrade::None;
	}
}
}

EPotentialGrade FPotentialFormula::GetMaxPotentialGrade(EItemRarity Rarity)
{
	switch (Rarity)
	{
	case EItemRarity::Rare:
		return EPotentialGrade::Epic;
	case EItemRarity::Epic:
		return EPotentialGrade::Unique;
	case EItemRarity::Unique:
	case EItemRarity::Legendary:
	case EItemRarity::Transcendent:
	case EItemRarity::Mythic:
		return EPotentialGrade::Legendary;
	case EItemRarity::None:
	case EItemRarity::Common:
	default:
		return EPotentialGrade::None;
	}
}

int32 FPotentialFormula::GetPotentialLineCount(EPotentialGrade Grade)
{
	switch (Grade)
	{
	case EPotentialGrade::Rare:
		return 1;
	case EPotentialGrade::Epic:
		return 2;
	case EPotentialGrade::Unique:
	case EPotentialGrade::Legendary:
		return 3;
	case EPotentialGrade::None:
	default:
		return 0;
	}
}

void FPotentialFormula::GetPotentialRollRange(EPotentialGrade Grade, float& OutMin, float& OutMax)
{
	switch (Grade)
	{
	case EPotentialGrade::Rare:
		OutMin = 0.01f;
		OutMax = 0.03f;
		break;
	case EPotentialGrade::Epic:
		OutMin = 0.03f;
		OutMax = 0.06f;
		break;
	case EPotentialGrade::Unique:
		OutMin = 0.06f;
		OutMax = 0.10f;
		break;
	case EPotentialGrade::Legendary:
		OutMin = 0.10f;
		OutMax = 0.15f;
		break;
	case EPotentialGrade::None:
	default:
		OutMin = 0.0f;
		OutMax = 0.0f;
		break;
	}
}

TArray<FPotentialLine> FPotentialFormula::RollPotentialLines(EPotentialGrade Grade, FRandomStream& Rng)
{
	TArray<FPotentialLine> Lines;
	const int32 LineCount = GetPotentialLineCount(Grade);
	if (LineCount <= 0)
	{
		return Lines;
	}

	TArray<EPotentialStat> Stats{
		EPotentialStat::PhysAtkPercent,
		EPotentialStat::MagicAtkPercent,
		EPotentialStat::HpPercent,
		EPotentialStat::PhysDefPercent,
		EPotentialStat::MagicDefPercent,
		EPotentialStat::CritRatePercent,
		EPotentialStat::AtkSpeedPercent,
		EPotentialStat::CritDmgPercent
	};
	for (int32 Index = Stats.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Rng.RandRange(0, Index);
		Stats.Swap(Index, SwapIndex);
	}

	float MinValue = 0.0f;
	float MaxValue = 0.0f;
	GetPotentialRollRange(Grade, MinValue, MaxValue);
	for (int32 Index = 0; Index < LineCount; ++Index)
	{
		FPotentialLine Line;
		Line.Stat = Stats[Index];
		Line.Value = FMath::RoundToFloat(Rng.FRandRange(MinValue, MaxValue) * 1000.0f) / 1000.0f;
		Lines.Add(Line);
	}
	return Lines;
}

EPotentialGrade FPotentialFormula::ApplyRankCube(EPotentialGrade CurrentGrade, EPotentialGrade MaxGrade, FRandomStream& Rng, TArray<FPotentialLine>& OutLines)
{
	EPotentialGrade NewGrade = CurrentGrade;
	if (GetGradeIndex(CurrentGrade) > 0 && GetGradeIndex(CurrentGrade) < GetGradeIndex(MaxGrade) && Rng.GetFraction() < RankCubeUpgradeChance)
	{
		NewGrade = GradeFromIndex(GetGradeIndex(CurrentGrade) + 1);
	}
	OutLines = RollPotentialLines(NewGrade, Rng);
	return NewGrade;
}
