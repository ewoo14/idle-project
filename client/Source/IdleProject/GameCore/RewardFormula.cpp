#include "GameCore/RewardFormula.h"

#include "GameCore/StageFormula.h"

namespace
{
int64 ScaleKillReward(int64 BaseReward, int32 GlobalStageIndex, bool bIsBoss, bool bIsElite)
{
	const double StageMultiplier = static_cast<double>(FStageFormula::ComputeRewardMultiplier(GlobalStageIndex));
	const double StageTypeMultiplier = bIsBoss ? FRewardFormula::BossRewardBonus : (bIsElite ? FRewardFormula::EliteRewardBonus : 1.0);
	return FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseReward) * StageMultiplier * StageTypeMultiplier));
}
}

int64 FRewardFormula::ComputeKillExp(int64 BaseExp, int32 GlobalStageIndex, bool bIsBoss, bool bIsElite)
{
	return ScaleKillReward(BaseExp, GlobalStageIndex, bIsBoss, bIsElite);
}

int64 FRewardFormula::ComputeKillGold(int64 BaseGold, int32 GlobalStageIndex, bool bIsBoss, bool bIsElite)
{
	return ScaleKillReward(BaseGold, GlobalStageIndex, bIsBoss, bIsElite);
}

int32 FRewardFormula::GetMonsterLevelForStage(int32 GlobalStageIndex)
{
	return 1 + FMath::Max(0, GlobalStageIndex - 1);
}
