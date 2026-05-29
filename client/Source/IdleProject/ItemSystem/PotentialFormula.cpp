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
	// 잠재 V2: Transcendent(인덱스 5).
	case 5:
		return EPotentialGrade::Transcendent;
	case 0:
	default:
		return EPotentialGrade::None;
	}
}

// 잠재 V2 신규 옵션 값 배수(전투 8종 = 1.0). 서버 NEW_OPTION_VALUE_SCALE parity
// (AllStatPercent 0.4 / GoldFindPercent 1.5 / DropRatePercent 1.5).
float PotentialOptionValueScale(EPotentialStat Stat)
{
	switch (Stat)
	{
	case EPotentialStat::AllStatPercent:
		return 0.4f;
	case EPotentialStat::GoldFindPercent:
	case EPotentialStat::DropRatePercent:
		return 1.5f;
	default:
		return 1.0f;
	}
}

// 서버 Math.round(x*1000)/1000 (3자리, fround 아님) parity.
float PotentialRound3(float Value)
{
	return FMath::RoundToFloat(Value * 1000.0f) / 1000.0f;
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
		return EPotentialGrade::Legendary;
	// 잠재 V2: 고레어도(Legendary/Transcendent/Mythic) 아이템은 Transcendent 잠재 허용(무한 chase). 서버 getMaxPotentialGrade parity.
	case EItemRarity::Legendary:
	case EItemRarity::Transcendent:
	case EItemRarity::Mythic:
		return EPotentialGrade::Transcendent;
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
	// 잠재 V2: Transcendent 4줄. 서버 getPotentialLineCount parity.
	case EPotentialGrade::Transcendent:
		return 4;
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
	// 잠재 V2: Transcendent = Legendary × 1.3 상향(0.13 ~ 0.195). 서버 getPotentialRollRange parity.
	case EPotentialGrade::Transcendent:
		OutMin = 0.13f;
		OutMax = 0.195f;
		break;
	case EPotentialGrade::None:
	default:
		OutMin = 0.0f;
		OutMax = 0.0f;
		break;
	}
}

void FPotentialFormula::GetPotentialStatRollRange(EPotentialGrade Grade, EPotentialStat Stat, float& OutMin, float& OutMax)
{
	float BaseMin = 0.0f;
	float BaseMax = 0.0f;
	GetPotentialRollRange(Grade, BaseMin, BaseMax);
	const float Scale = PotentialOptionValueScale(Stat);
	// 서버와 동일하게 범위 경계도 3자리 라운딩(Math.round(min*scale*1000)/1000).
	OutMin = PotentialRound3(BaseMin * Scale);
	OutMax = PotentialRound3(BaseMax * Scale);
}

TArray<FPotentialLine> FPotentialFormula::RollPotentialLines(EPotentialGrade Grade, FRandomStream& Rng)
{
	TArray<FPotentialLine> Lines;
	const int32 LineCount = GetPotentialLineCount(Grade);
	if (LineCount <= 0)
	{
		return Lines;
	}

	// 잠재 V2: 추첨 풀 8→11(신규 옵션 3종 포함). 서버 POTENTIAL_STATS 순서/Fisher-Yates parity.
	TArray<EPotentialStat> Stats{
		EPotentialStat::PhysAtkPercent,
		EPotentialStat::MagicAtkPercent,
		EPotentialStat::HpPercent,
		EPotentialStat::PhysDefPercent,
		EPotentialStat::MagicDefPercent,
		EPotentialStat::CritRatePercent,
		EPotentialStat::AtkSpeedPercent,
		EPotentialStat::CritDmgPercent,
		EPotentialStat::AllStatPercent,
		EPotentialStat::GoldFindPercent,
		EPotentialStat::DropRatePercent
	};
	for (int32 Index = Stats.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = Rng.RandRange(0, Index);
		Stats.Swap(Index, SwapIndex);
	}

	for (int32 Index = 0; Index < LineCount; ++Index)
	{
		FPotentialLine Line;
		Line.Stat = Stats[Index];
		// 잠재 V2: 신규 옵션은 스탯별 배수로 값 범위 조정(전투 8종은 1.0 그대로). 3자리 라운딩.
		float MinValue = 0.0f;
		float MaxValue = 0.0f;
		GetPotentialStatRollRange(Grade, Line.Stat, MinValue, MaxValue);
		Line.Value = PotentialRound3(Rng.FRandRange(MinValue, MaxValue));
		Lines.Add(Line);
	}
	return Lines;
}

EPotentialGrade FPotentialFormula::ApplyRankCube(EPotentialGrade CurrentGrade, EPotentialGrade MaxGrade, FRandomStream& Rng, TArray<FPotentialLine>& OutLines)
{
	EPotentialGrade NewGrade = CurrentGrade;
	const EPotentialGrade NextGrade = GradeFromIndex(GetGradeIndex(CurrentGrade) + 1);
	// 잠재 V2: Legendary→Transcendent 상승만 낮은 확률(0.05), 나머지는 기존 0.08. 서버 upgradeChanceTo parity.
	const float UpgradeChance = NextGrade == EPotentialGrade::Transcendent ? RankCubeTranscendentChance : RankCubeUpgradeChance;
	if (GetGradeIndex(CurrentGrade) > 0 && GetGradeIndex(CurrentGrade) < GetGradeIndex(MaxGrade) && Rng.GetFraction() < UpgradeChance)
	{
		NewGrade = NextGrade;
	}
	OutLines = RollPotentialLines(NewGrade, Rng);
	return NewGrade;
}
