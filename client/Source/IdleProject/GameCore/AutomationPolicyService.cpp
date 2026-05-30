#include "GameCore/AutomationPolicyService.h"

FProgressionDecision UAutomationPolicyService::DecideOnClear(
	EProgressionMode Mode,
	int32 ClearedGlobalStage,
	int32 HighestClearedGlobalStage,
	int32 FarmLockStage,
	bool bAutoBossChallenge,
	bool bNextIsBoss)
{
	const int32 Cleared = FMath::Max(1, ClearedGlobalStage);
	const int32 Highest = FMath::Max(Cleared, HighestClearedGlobalStage);

	FProgressionDecision Out;
	if (Mode == EProgressionMode::FarmLock)
	{
		const int32 Upper = Highest + 1;
		Out.Action = EProgressionAction::Hold;
		Out.TargetGlobalStage = FMath::Clamp(FarmLockStage, 1, Upper);
		return Out;
	}

	if (bNextIsBoss && !bAutoBossChallenge)
	{
		Out.Action = EProgressionAction::Hold;
		Out.TargetGlobalStage = Cleared;
		return Out;
	}

	Out.Action = EProgressionAction::Advance;
	Out.TargetGlobalStage = Cleared + 1;
	return Out;
}

FProgressionDecision UAutomationPolicyService::DecideOnDeath(
	EProgressionMode Mode,
	int32 CurrentGlobalStage,
	int32 ConsecutiveDeaths,
	int32 DeathThreshold)
{
	const int32 Current = FMath::Max(1, CurrentGlobalStage);
	FProgressionDecision Out;
	Out.Action = EProgressionAction::Hold;
	Out.TargetGlobalStage = Current;

	if (Mode != EProgressionMode::AutoRetreat)
	{
		return Out;
	}
	const int32 Deaths = FMath::Max(0, ConsecutiveDeaths);
	const int32 Threshold = FMath::Max(1, DeathThreshold);
	if (Deaths >= Threshold && Current > 1)
	{
		Out.Action = EProgressionAction::Retreat;
		Out.TargetGlobalStage = Current - 1;
	}
	return Out;
}

bool UAutomationPolicyService::IsFeatureUnlocked(
	EAutomationFeature Feature,
	int32 HighestClearedChapter,
	int32 RebirthCount)
{
	int32 MinChapter = 0;
	int32 MinRebirth = 0;
	switch (Feature)
	{
	case EAutomationFeature::Progression:     MinChapter = 0; MinRebirth = 0; break;
	case EAutomationFeature::SkillTactics:    MinChapter = 3; MinRebirth = 0; break;
	case EAutomationFeature::AutoGear:        MinChapter = 5; MinRebirth = 0; break;
	case EAutomationFeature::AutoConsumable:  MinChapter = 0; MinRebirth = 1; break;
	}
	return HighestClearedChapter >= MinChapter && RebirthCount >= MinRebirth;
}

int64 UAutomationPolicyService::EfficiencyUpgradeCost(int64 Base, float Growth, int32 Level)
{
	const int32 Lv = FMath::Max(0, Level);
	// 서버 Math.round(base * growth^level) 미러. double 누적 후 반올림.
	const double Cost = static_cast<double>(Base) * FMath::Pow(static_cast<double>(Growth), static_cast<double>(Lv));
	return static_cast<int64>(FMath::RoundToDouble(Cost));
}

void UAutomationPolicyService::RestoreState(
	EProgressionMode InMode, int32 InFarmLockStage, bool bInAutoBoss, int32 InThreshold)
{
	Mode = InMode;
	FarmLockStage = FMath::Max(1, InFarmLockStage);
	bAutoBossChallenge = bInAutoBoss;
	PushDeathThreshold = FMath::Max(1, InThreshold);
}

bool UAutomationPolicyService::EvaluateSkillRule(
	ESkillAutoCondition Condition,
	float HpThresholdPct,
	float SelfHpPct,
	bool bIsBossElite,
	bool bBuffActive)
{
	switch (Condition)
	{
	case ESkillAutoCondition::BossEliteOnly:
		return bIsBossElite;
	case ESkillAutoCondition::HpBelow:
		return FMath::Clamp(SelfHpPct, 0.0f, 1.0f) <= FMath::Clamp(HpThresholdPct, 0.0f, 1.0f);
	case ESkillAutoCondition::MaintainBuff:
		return !bBuffActive;
	default:
		return true; // Always
	}
}

void UAutomationPolicyService::SetSkillRule(const FSkillAutoRule& Rule)
{
	for (FSkillAutoRule& Existing : SkillRules)
	{
		if (Existing.SkillId == Rule.SkillId)
		{
			Existing = Rule;
			return;
		}
	}
	SkillRules.Add(Rule);
}

void UAutomationPolicyService::ClearSkillRule(FName SkillId)
{
	SkillRules.RemoveAll([SkillId](const FSkillAutoRule& R) { return R.SkillId == SkillId; });
}

void UAutomationPolicyService::RestoreSkillRules(const TArray<FSkillAutoRule>& InRules)
{
	SkillRules = InRules;
}

void UAutomationPolicyService::RestoreGearPolicy(bool bInAutoEquip, bool bInAutoSell, EItemRarity InMaxRarity)
{
	bAutoEquipByPower = bInAutoEquip;
	bAutoSell = bInAutoSell;
	AutoSellMaxRarity = InMaxRarity;
}
