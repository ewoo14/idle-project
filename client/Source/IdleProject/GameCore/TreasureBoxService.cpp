#include "GameCore/TreasureBoxService.h"

const TArray<FTreasurePoolEntry>& UTreasureBoxService::GetPool()
{
	// 서버 treasureBox.ts TREASURE_POOL 1:1 미러. 순서=누적 가중 구간 순서(weight 합 100).
	//   gold            weight 40 → roll  0~39 / 10000~50000
	//   essence         weight 25 → roll 40~64 / 3~10
	//   consumable      weight 15 → roll 65~79 / 1~2
	//   protectionScroll weight 10 → roll 80~89 / 1~3
	//   resetCube       weight  7 → roll 90~96 / 1~2
	//   rankCube        weight  3 → roll 97~99 / 1~1
	static const TArray<FTreasurePoolEntry> Pool = {
		{ ETreasureReward::Gold, 40, 10000, 50000 },
		{ ETreasureReward::Essence, 25, 3, 10 },
		{ ETreasureReward::Consumable, 15, 1, 2 },
		{ ETreasureReward::ProtectionScroll, 10, 1, 3 },
		{ ETreasureReward::ResetCube, 7, 1, 2 },
		{ ETreasureReward::RankCube, 3, 1, 1 },
	};
	return Pool;
}

int32 UTreasureBoxService::GetTotalWeight()
{
	int32 Total = 0;
	for (const FTreasurePoolEntry& Entry : GetPool())
	{
		Total += Entry.Weight;
	}
	return Total;
}

ETreasureReward UTreasureBoxService::PickReward(int32 Roll)
{
	const int32 Total = GetTotalWeight();
	// [0, total-1] 로 클램프(음수/초과 안전). 서버 pickTreasureReward 의 Math.min/max 와 1:1.
	const int32 Clamped = FMath::Min(FMath::Max(0, Roll), Total - 1);

	int32 Cumulative = 0;
	for (const FTreasurePoolEntry& Entry : GetPool())
	{
		Cumulative += Entry.Weight;
		if (Clamped < Cumulative)
		{
			return Entry.Reward;
		}
	}

	// 도달 불가(클램프로 인해) — 안전상 마지막 보상 반환.
	return GetPool().Last().Reward;
}

bool UTreasureBoxService::CanDrawToday(const FString& Date) const
{
	return !Date.IsEmpty() && Date != LastDrawDate;
}

FTreasureReward UTreasureBoxService::DrawTreasure(const FString& Date, FRandomStream& Rng)
{
	FTreasureReward Result;
	if (!CanDrawToday(Date))
	{
		return Result;
	}

	// RNG 클라 권위(#71): roll 로 보상 종류, 수량으로 보상 수치를 결정한다.
	const int32 Roll = Rng.RandRange(0, GetTotalWeight() - 1);
	const ETreasureReward Reward = PickReward(Roll);

	int64 MinAmount = 0;
	int64 MaxAmount = 0;
	for (const FTreasurePoolEntry& Entry : GetPool())
	{
		if (Entry.Reward == Reward)
		{
			MinAmount = Entry.MinAmount;
			MaxAmount = Entry.MaxAmount;
			break;
		}
	}

	const int64 Amount = Rng.RandRange(static_cast<int32>(MinAmount), static_cast<int32>(MaxAmount));

	Result.Reward = Reward;
	Result.Amount = Amount;

	LastDrawDate = Date;
	TotalDraws = FMath::Max<int64>(0, TotalDraws + 1);
	return Result;
}

void UTreasureBoxService::RestoreState(const FString& Date, int64 Total)
{
	LastDrawDate = Date;
	TotalDraws = FMath::Max<int64>(0, Total);
}
