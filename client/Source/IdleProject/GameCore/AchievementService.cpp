#include "GameCore/AchievementService.h"

void UAchievementService::InitializeDefaultAchievements()
{
	MetricValues.Reset();
	UnlockedTiers.Reset();
	UniqueItemIds.Reset();
}

void UAchievementService::RecordMetric(EAchievementMetric Metric, int64 AmountOrValue)
{
	if (AmountOrValue <= 0 || !IsKnownMetric(Metric))
	{
		return;
	}

	const int64 CurrentValue = MetricValues.FindRef(Metric);
	bool bUsesMaximum = false;
	for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
	{
		if (Definition.Metric == Metric && Definition.MetricMode == EAchievementMetricMode::Maximum)
		{
			bUsesMaximum = true;
			break;
		}
	}

	int64 NewValue = CurrentValue;
	if (bUsesMaximum)
	{
		NewValue = FMath::Max(CurrentValue, AmountOrValue);
	}
	else if (CurrentValue > MAX_int64 - AmountOrValue)
	{
		NewValue = MAX_int64;
	}
	else
	{
		NewValue = CurrentValue + AmountOrValue;
	}

	if (NewValue == CurrentValue)
	{
		return;
	}

	MetricValues.Add(Metric, NewValue);
	RecomputeUnlockedTiers(true);
	OnAchievementProgress.Broadcast(Metric, NewValue, GetTotalPoints());
}

void UAchievementService::RecordItemCollected(FName ItemId)
{
	if (ItemId.IsNone())
	{
		return;
	}

	RecordMetric(EAchievementMetric::ItemsCollected, 1);
	if (!UniqueItemIds.Contains(ItemId))
	{
		UniqueItemIds.Add(ItemId);
		RecordMetric(EAchievementMetric::UniqueItemsFound, 1);
	}
}

int64 UAchievementService::GetMetricValue(EAchievementMetric Metric) const
{
	return MetricValues.FindRef(Metric);
}

int32 UAchievementService::GetTotalPoints() const
{
	int32 Total = 0;
	for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
	{
		Total += FAchievementFormula::GetPointsForTiers(Definition, UnlockedTiers.FindRef(Definition.AchievementId));
	}
	return Total;
}

float UAchievementService::GetStatMultiplier() const
{
	return FAchievementFormula::GetStatMultiplier(GetTotalPoints());
}

TArray<FAchievementCategoryProgress> UAchievementService::GetCategoryProgress() const
{
	TMap<EAchievementCategory, FAchievementCategoryProgress> ProgressByCategory;
	for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
	{
		FAchievementCategoryProgress& Progress = ProgressByCategory.FindOrAdd(Definition.Category);
		Progress.Category = Definition.Category;
		const int32 Tier = UnlockedTiers.FindRef(Definition.AchievementId);
		Progress.HighestUnlockedTier = FMath::Max(Progress.HighestUnlockedTier, Tier);
		Progress.UnlockedPoints += FAchievementFormula::GetPointsForTiers(Definition, Tier);
		Progress.CurrentValue = FMath::Max<int64>(Progress.CurrentValue, MetricValues.FindRef(Definition.Metric));
	}

	TArray<FAchievementCategoryProgress> Result;
	ProgressByCategory.GenerateValueArray(Result);
	return Result;
}

void UAchievementService::CaptureState(TArray<FAchievementMetricSaveEntry>& OutMetrics, TArray<FAchievementSaveEntry>& OutAchievements, TArray<FName>* OutUniqueItemIds) const
{
	OutMetrics.Reset();
	for (const TPair<EAchievementMetric, int64>& Pair : MetricValues)
	{
		if (Pair.Value <= 0 || !IsKnownMetric(Pair.Key))
		{
			continue;
		}

		FAchievementMetricSaveEntry Entry;
		Entry.Metric = Pair.Key;
		Entry.Value = Pair.Value;
		OutMetrics.Add(Entry);
	}

	OutAchievements.Reset();
	for (const TPair<FString, int32>& Pair : UnlockedTiers)
	{
		if (Pair.Value <= 0)
		{
			continue;
		}

		FAchievementSaveEntry Entry;
		Entry.AchievementId = Pair.Key;
		Entry.Tier = Pair.Value;
		OutAchievements.Add(Entry);
	}

	if (OutUniqueItemIds)
	{
		OutUniqueItemIds->Reset();
		for (const FName& ItemId : UniqueItemIds)
		{
			if (!ItemId.IsNone())
			{
				OutUniqueItemIds->Add(ItemId);
			}
		}
	}
}

void UAchievementService::RestoreState(const TArray<FAchievementMetricSaveEntry>& InMetrics, const TArray<FAchievementSaveEntry>& InAchievements, const TArray<FName>* InUniqueItemIds)
{
	MetricValues.Reset();
	for (const FAchievementMetricSaveEntry& Entry : InMetrics)
	{
		if (Entry.Value > 0 && IsKnownMetric(Entry.Metric))
		{
			MetricValues.Add(Entry.Metric, Entry.Value);
		}
	}

	UnlockedTiers.Reset();
	for (const FAchievementSaveEntry& Entry : InAchievements)
	{
		if (!Entry.AchievementId.IsEmpty() && Entry.Tier > 0 && FindDefinitionById(Entry.AchievementId))
		{
			UnlockedTiers.Add(Entry.AchievementId, Entry.Tier);
		}
	}

	UniqueItemIds.Reset();
	if (InUniqueItemIds)
	{
		for (const FName& ItemId : *InUniqueItemIds)
		{
			if (!ItemId.IsNone())
			{
				UniqueItemIds.Add(ItemId);
			}
		}
	}

	RecomputeUnlockedTiers(false);
}

bool UAchievementService::IsKnownMetric(EAchievementMetric Metric)
{
	return FAchievementFormula::GetDefinitions().ContainsByPredicate([Metric](const FAchievementDefinition& Definition)
	{
		return Definition.Metric == Metric;
	});
}

const FAchievementDefinition* UAchievementService::FindDefinitionById(const FString& AchievementId)
{
	return FAchievementFormula::GetDefinitions().FindByPredicate([&AchievementId](const FAchievementDefinition& Definition)
	{
		return Definition.AchievementId == AchievementId;
	});
}

void UAchievementService::RecomputeUnlockedTiers(bool bBroadcastUnlocks)
{
	for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
	{
		const int32 OldTier = UnlockedTiers.FindRef(Definition.AchievementId);
		const int32 NewTier = FAchievementFormula::GetTierForValue(Definition, MetricValues.FindRef(Definition.Metric));
		if (NewTier <= OldTier)
		{
			continue;
		}

		UnlockedTiers.Add(Definition.AchievementId, NewTier);
		if (bBroadcastUnlocks)
		{
			for (int32 Tier = OldTier + 1; Tier <= NewTier; ++Tier)
			{
				OnAchievementUnlocked.Broadcast(Definition.AchievementId, Tier);
			}
		}
	}
}
