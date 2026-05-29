#include "GameCore/TitleService.h"

#include "GameCore/AchievementService.h"

void UTitleService::InitializeDefaultTitles()
{
	BuildDefaultDefinitions();
}

void UTitleService::BuildDefaultDefinitions()
{
	if (!Definitions.IsEmpty())
	{
		return;
	}

	auto AddDefinition = [this](const TCHAR* TitleId, EAchievementMetric Metric, int64 Threshold, ETitleBonus BonusType, float BonusValue)
	{
		FTitleDefinition Definition;
		Definition.TitleId = TitleId;
		// 한글 로컬라이즈는 UI 가 TitleId 기준으로 처리한다(여기선 id 미러를 Name 기본값으로 둔다).
		Definition.Name = FText::FromString(TitleId);
		Definition.Metric = Metric;
		Definition.Threshold = Threshold;
		Definition.BonusType = BonusType;
		Definition.BonusValue = BonusValue;
		Definitions.Add(Definition);
		DefinitionById.Add(Definition.TitleId, Definition);
	};

	// 서버 title.ts TITLE_CATALOG 18종 1:1 이식. bonusValue 는 비율(0.03 = +3%).
	// 전투(Combat)
	AddDefinition(TEXT("monster_hunter"), EAchievementMetric::MonstersKilled, 10000, ETitleBonus::AllStatPct, 0.03f);
	AddDefinition(TEXT("boss_slayer"), EAchievementMetric::BossesKilled, 500, ETitleBonus::CritDmgPct, 0.1f);
	AddDefinition(TEXT("monster_annihilator"), EAchievementMetric::MonstersKilled, 1000000, ETitleBonus::AllStatPct, 0.1f);
	AddDefinition(TEXT("boss_executioner"), EAchievementMetric::BossesKilled, 5000, ETitleBonus::CritDmgPct, 0.18f);
	// 진행(Progression)
	AddDefinition(TEXT("rebirth_master"), EAchievementMetric::RebirthCount, 10, ETitleBonus::AllStatPct, 0.05f);
	AddDefinition(TEXT("transcendent"), EAchievementMetric::TranscendCount, 5, ETitleBonus::AllStatPct, 0.08f);
	AddDefinition(TEXT("stage_conqueror"), EAchievementMetric::StagesCleared, 1000, ETitleBonus::ExpPct, 0.12f);
	AddDefinition(TEXT("level_legend"), EAchievementMetric::HighestLevelReached, 500, ETitleBonus::AllStatPct, 0.06f);
	// 탑(Tower)
	AddDefinition(TEXT("tower_conqueror"), EAchievementMetric::TowerHighestFloor, 100, ETitleBonus::GoldPct, 0.15f);
	AddDefinition(TEXT("tower_overlord"), EAchievementMetric::TowerHighestFloor, 300, ETitleBonus::CritDmgPct, 0.15f);
	// 수집(Collection)
	AddDefinition(TEXT("collector"), EAchievementMetric::ItemsCollected, 5000, ETitleBonus::GoldPct, 0.1f);
	AddDefinition(TEXT("unique_seeker"), EAchievementMetric::UniqueItemsFound, 100, ETitleBonus::AllStatPct, 0.07f);
	// 경제(Economy)
	AddDefinition(TEXT("gold_king"), EAchievementMetric::GoldEarned, 1000000000, ETitleBonus::GoldPct, 0.2f);
	// 펫(Pet)
	AddDefinition(TEXT("pet_whisperer"), EAchievementMetric::HighestPetLevel, 50, ETitleBonus::ExpPct, 0.08f);
	// 퀘스트(Quest)
	AddDefinition(TEXT("quest_champion"), EAchievementMetric::QuestsCompleted, 200, ETitleBonus::ExpPct, 0.1f);
	// 강화(Gear)
	AddDefinition(TEXT("enhance_artisan"), EAchievementMetric::HighestEnhanceLevel, 20, ETitleBonus::AllStatPct, 0.05f);
	AddDefinition(TEXT("enhance_grandmaster"), EAchievementMetric::HighestEnhanceLevel, 35, ETitleBonus::CritDmgPct, 0.12f);
	// 메타(Misc)
	AddDefinition(TEXT("veteran"), EAchievementMetric::DaysPlayed, 100, ETitleBonus::GoldPct, 0.1f);
}

void UTitleService::RecomputeUnlocked(const UAchievementService* Achievements)
{
	if (!Achievements)
	{
		return;
	}

	BuildDefaultDefinitions();
	for (const FTitleDefinition& Definition : Definitions)
	{
		// 해금은 영구(추가 only). 한 번 달성하면 메트릭이 줄어도 유지된다.
		if (!UnlockedTitleIds.Contains(Definition.TitleId)
			&& Achievements->GetMetricValue(Definition.Metric) >= Definition.Threshold)
		{
			UnlockedTitleIds.Add(Definition.TitleId);
		}
	}
}

bool UTitleService::EquipTitle(const FString& TitleId)
{
	if (!UnlockedTitleIds.Contains(TitleId) || !DefinitionById.Contains(TitleId))
	{
		return false;
	}

	EquippedTitleId = TitleId;
	return true;
}

void UTitleService::UnequipTitle()
{
	EquippedTitleId.Reset();
}

FTitleBonus UTitleService::GetEquippedTitleBonus() const
{
	FTitleBonus Bonus;
	const FTitleDefinition* Definition = GetEquippedDefinition();
	// 미장착 또는 (방어적으로) 미해금이면 None/0.
	if (!Definition || !UnlockedTitleIds.Contains(EquippedTitleId))
	{
		return Bonus;
	}

	Bonus.Type = Definition->BonusType;
	Bonus.Value = Definition->BonusValue;
	return Bonus;
}

void UTitleService::RestoreState(const TSet<FString>& Unlocked, const FString& Equipped)
{
	BuildDefaultDefinitions();
	UnlockedTitleIds.Reset();
	for (const FString& TitleId : Unlocked)
	{
		// 정의에 없는(구버전/잘못된) id 는 무시.
		if (DefinitionById.Contains(TitleId))
		{
			UnlockedTitleIds.Add(TitleId);
		}
	}

	// 장착은 해금된 경우에만 유지(미해금이면 미장착으로 회귀).
	EquippedTitleId.Reset();
	if (UnlockedTitleIds.Contains(Equipped) && DefinitionById.Contains(Equipped))
	{
		EquippedTitleId = Equipped;
	}
}

const FTitleDefinition* UTitleService::GetEquippedDefinition() const
{
	if (EquippedTitleId.IsEmpty())
	{
		return nullptr;
	}
	return DefinitionById.Find(EquippedTitleId);
}
