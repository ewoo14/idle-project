#include "GameCore/WeeklyBossFormula.h"

namespace
{
	constexpr double BaseMilestoneThreshold = 1000.0;
	constexpr double BaseMilestoneGoldReward = 5000.0;
	constexpr double MilestoneGrowth = 1.5;

	int64 FloorAndClampToInt64(double Value)
	{
		if (!FMath::IsFinite(Value) || Value >= static_cast<double>(MAX_int64))
		{
			return MAX_int64;
		}

		return FMath::Max<int64>(0, FMath::FloorToInt64(Value));
	}
}

int64 FWeeklyBossFormula::GetChallengeDamage(int64 CombatPower)
{
	return FMath::Max<int64>(0, CombatPower);
}

int64 FWeeklyBossFormula::MilestoneThreshold(int32 Milestone)
{
	if (Milestone < 1)
	{
		return 0;
	}

	const double Threshold = BaseMilestoneThreshold * FMath::Pow(MilestoneGrowth, static_cast<double>(Milestone - 1));
	return FloorAndClampToInt64(Threshold);
}

int32 FWeeklyBossFormula::GetReachedMilestones(int64 AccumDamage)
{
	const int64 Damage = FMath::Max<int64>(0, AccumDamage);
	int32 Reached = 0;
	for (int32 Next = 1; Next < MAX_int32; ++Next)
	{
		const int64 Threshold = MilestoneThreshold(Next);
		if (Threshold <= 0 || Damage < Threshold)
		{
			break;
		}
		Reached = Next;
	}
	return Reached;
}

int64 FWeeklyBossFormula::MilestoneGoldReward(int32 Milestone)
{
	if (Milestone < 1)
	{
		return 0;
	}

	const double Reward = BaseMilestoneGoldReward * FMath::Pow(MilestoneGrowth, static_cast<double>(Milestone - 1));
	return FloorAndClampToInt64(Reward);
}

int64 FWeeklyBossFormula::MilestoneEssenceReward(int32 Milestone)
{
	return FMath::Max<int64>(0, static_cast<int64>(FMath::FloorToInt(Milestone * 3.0f)));
}
