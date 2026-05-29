#include "GameCore/WeeklyBossService.h"

#include "GameCore/QuestService.h"
#include "GameCore/WeeklyBossFormula.h"

namespace
{
FString NormalizeWeekId(const FString& CurrentWeek)
{
	return CurrentWeek.IsEmpty() ? UQuestService::GetCurrentUtcWeekString() : CurrentWeek;
}
}

void UWeeklyBossService::EnsureWeek(const FString& CurrentWeek)
{
	const FString NormalizedWeek = NormalizeWeekId(CurrentWeek);
	if (WeekId == NormalizedWeek)
	{
		return;
	}

	WeekId = NormalizedWeek;
	Damage = 0;
	ChallengesUsed = 0;
	ClaimedMilestones = 0;
}

FWeeklyBossChallengeResult UWeeklyBossService::Challenge(int64 CombatPower, const FString& CurrentWeek)
{
	EnsureWeek(CurrentWeek);

	FWeeklyBossChallengeResult Result;
	Result.WeekId = WeekId;
	Result.TotalDamage = Damage;
	Result.ChallengesUsed = ChallengesUsed;
	Result.RemainingChallenges = GetRemainingChallenges();
	Result.ReachedMilestones = GetReachedMilestones();

	if (GetRemainingChallenges() <= 0)
	{
		return Result;
	}

	const int64 ChallengeDamage = FWeeklyBossFormula::GetChallengeDamage(CombatPower);
	if (ChallengeDamage <= 0)
	{
		return Result;
	}

	Damage = FMath::Max<int64>(0, Damage + ChallengeDamage);
	ChallengesUsed = FMath::Clamp(ChallengesUsed + 1, 0, FWeeklyBossFormula::WeeklyChallengeLimit);

	Result.bSuccess = true;
	Result.DamageDealt = ChallengeDamage;
	Result.TotalDamage = Damage;
	Result.ChallengesUsed = ChallengesUsed;
	Result.RemainingChallenges = GetRemainingChallenges();
	Result.ReachedMilestones = GetReachedMilestones();
	return Result;
}

bool UWeeklyBossService::ClaimMilestone(int32 Milestone)
{
	if (Milestone <= ClaimedMilestones || Milestone > GetReachedMilestones())
	{
		return false;
	}

	ClaimedMilestones = Milestone;
	return true;
}

int32 UWeeklyBossService::GetRemainingChallenges() const
{
	return FMath::Clamp(FWeeklyBossFormula::WeeklyChallengeLimit - ChallengesUsed, 0, FWeeklyBossFormula::WeeklyChallengeLimit);
}

int32 UWeeklyBossService::GetReachedMilestones() const
{
	return FWeeklyBossFormula::GetReachedMilestones(Damage);
}

FWeeklyBossSaveState UWeeklyBossService::ExportSave() const
{
	FWeeklyBossSaveState State;
	State.WeekId = WeekId;
	State.Damage = FMath::Max<int64>(0, Damage);
	State.ChallengesUsed = FMath::Clamp(ChallengesUsed, 0, FWeeklyBossFormula::WeeklyChallengeLimit);
	State.ClaimedMilestones = FMath::Clamp(ClaimedMilestones, 0, GetReachedMilestones());
	return State;
}

void UWeeklyBossService::ImportSave(const FString& InWeekId, int64 InDamage, int32 InChallengesUsed, int32 InClaimedMilestones)
{
	WeekId = NormalizeWeekId(InWeekId);
	Damage = FMath::Max<int64>(0, InDamage);
	ChallengesUsed = FMath::Clamp(InChallengesUsed, 0, FWeeklyBossFormula::WeeklyChallengeLimit);
	ClaimedMilestones = FMath::Clamp(InClaimedMilestones, 0, GetReachedMilestones());
}
