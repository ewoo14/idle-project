#include "GameCore/SeasonService.h"

void USeasonService::InitializeDefaultSeason()
{
	BuildDefaultTiers();
	SeasonTokens = 0;
	ClaimedTiers.Empty();
}

void USeasonService::AddSeasonTokens(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SeasonTokens += Amount;
}

FSeasonClaimResult USeasonService::ClaimSeasonReward(int32 Tier)
{
	FSeasonClaimResult Result;
	Result.Tier = Tier;

	const FSeasonTierDefinition* Definition = TierByNumber.Find(Tier);
	if (!Definition)
	{
		Result.Message = TEXT("season_tier_not_found");
		return Result;
	}

	if (ClaimedTiers.Contains(Tier))
	{
		Result.Message = TEXT("season_reward_already_claimed");
		return Result;
	}

	if (SeasonTokens < Definition->RequiredTokens)
	{
		Result.Message = TEXT("season_tier_not_reached");
		return Result;
	}

	ClaimedTiers.Add(Tier);
	Result.bSuccess = true;
	Result.Message = TEXT("claimed");
	Result.RewardType = Definition->RewardType;
	Result.RewardAmount = Definition->RewardAmount;
	return Result;
}

int32 USeasonService::GetReachedTier() const
{
	int32 ReachedTier = 0;
	for (const FSeasonTierDefinition& Tier : Tiers)
	{
		if (SeasonTokens >= Tier.RequiredTokens)
		{
			ReachedTier = Tier.Tier;
		}
	}
	return ReachedTier;
}

void USeasonService::CaptureState(int32& OutSeasonId, int32& OutTokens, TArray<int32>& OutClaimedTiers) const
{
	OutSeasonId = CurrentSeasonId;
	OutTokens = SeasonTokens;
	OutClaimedTiers = ClaimedTiers.Array();
	OutClaimedTiers.Sort();
}

void USeasonService::RestoreState(int32 InSeasonId, int32 InTokens, const TArray<int32>& InClaimedTiers)
{
	InitializeDefaultSeason();
	if (InSeasonId != CurrentSeasonId)
	{
		return;
	}

	SeasonTokens = FMath::Max(0, InTokens);
	for (const int32 Tier : InClaimedTiers)
	{
		const FSeasonTierDefinition* Definition = TierByNumber.Find(Tier);
		if (Definition && SeasonTokens >= Definition->RequiredTokens)
		{
			ClaimedTiers.Add(Tier);
		}
	}
}

void USeasonService::BuildDefaultTiers()
{
	if (!Tiers.IsEmpty())
	{
		return;
	}

	auto AddTier = [this](int32 Tier, int32 RequiredTokens, ESeasonRewardType RewardType, int64 RewardAmount)
	{
		FSeasonTierDefinition Definition;
		Definition.Tier = Tier;
		Definition.RequiredTokens = RequiredTokens;
		Definition.RewardType = RewardType;
		Definition.RewardAmount = RewardAmount;
		Tiers.Add(Definition);
		TierByNumber.Add(Tier, Definition);
	};

	AddTier(1, 10, ESeasonRewardType::Gold, 500);
	AddTier(2, 25, ESeasonRewardType::Gold, 1000);
	AddTier(3, 45, ESeasonRewardType::Exp, 300);
	AddTier(4, 70, ESeasonRewardType::Gold, 1800);
	AddTier(5, 100, ESeasonRewardType::Exp, 650);
	AddTier(6, 135, ESeasonRewardType::Gold, 3000);
	AddTier(7, 175, ESeasonRewardType::Exp, 1100);
	AddTier(8, 220, ESeasonRewardType::Gold, 4800);
	AddTier(9, 270, ESeasonRewardType::Exp, 1750);
	AddTier(10, 325, ESeasonRewardType::Gold, 7500);
}
