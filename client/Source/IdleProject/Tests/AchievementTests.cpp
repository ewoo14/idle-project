#include "Misc/AutomationTest.h"

#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/SkillComponent.h"
#include "Engine/World.h"
#include "GameCore/AchievementFormula.h"
#include "GameCore/AchievementService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "Internationalization/IdleLocalization.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
const FAchievementDefinition* FindAchievement(EAchievementMetric Metric)
{
	for (const FAchievementDefinition& Definition : FAchievementFormula::GetDefinitions())
	{
		if (Definition.Metric == Metric)
		{
			return &Definition;
		}
	}
	return nullptr;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAchievementFormulaCatalogTest,
	"IdleProject.GameCore.Achievement.FormulaCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAchievementFormulaCatalogTest::RunTest(const FString& Parameters)
{
	const TArray<FAchievementDefinition>& Definitions = FAchievementFormula::GetDefinitions();
	TestTrue(TEXT("Achievement catalog has substantial breadth"), Definitions.Num() >= 20);

	TSet<EAchievementCategory> Categories;
	for (const FAchievementDefinition& Definition : Definitions)
	{
		Categories.Add(Definition.Category);
		TestFalse(TEXT("Achievement id is populated"), Definition.AchievementId.IsEmpty());
		TestFalse(TEXT("Achievement display key is populated"), Definition.DisplayNameKey.IsEmpty());
		TestTrue(TEXT("Achievement base threshold is positive"), Definition.BaseThreshold > 0);
		TestTrue(TEXT("Achievement growth is above one"), Definition.Growth > 1.0f);
		TestTrue(TEXT("Achievement points per tier are positive"), Definition.PointsPerTier > 0);
	}
	TestTrue(TEXT("Catalog covers at least eight categories"), Categories.Num() >= 8);

	const FAchievementDefinition* MonsterKills = FindAchievement(EAchievementMetric::MonstersKilled);
	TestNotNull(TEXT("Monster kill achievement exists"), MonsterKills);
	if (!MonsterKills)
	{
		return false;
	}

	TestEqual(TEXT("Value below first threshold has no tier"), FAchievementFormula::GetTierForValue(*MonsterKills, 9), 0);
	TestEqual(TEXT("Base threshold unlocks tier one"), FAchievementFormula::GetTierForValue(*MonsterKills, 10), 1);
	TestEqual(TEXT("Second geometric threshold unlocks tier two"), FAchievementFormula::GetTierForValue(*MonsterKills, 20), 2);
	TestEqual(TEXT("Third geometric threshold unlocks tier three"), FAchievementFormula::GetTierForValue(*MonsterKills, 40), 3);
	TestEqual(TEXT("Three one-point tiers award three points"), FAchievementFormula::GetPointsForTiers(*MonsterKills, 3), 3);
	TestEqual(TEXT("Three achievement points add three percent"), FAchievementFormula::GetStatMultiplier(3), 1.03f);
	TestEqual(TEXT("Soft-cap start remains linear"), FAchievementFormula::GetStatMultiplier(100), 2.0f);
	TestEqual(TEXT("Soft-cap reduces points after one hundred"), FAchievementFormula::GetStatMultiplier(125), 2.1967347f);
	TestEqual(TEXT("Soft-cap approaches the achievement ceiling"), FAchievementFormula::GetStatMultiplier(500), 2.4998324f);
	TestEqual(TEXT("Zero achievement points stay neutral"), FAchievementFormula::GetStatMultiplier(0), 1.0f);
	TestEqual(TEXT("Negative achievement points stay neutral"), FAchievementFormula::GetStatMultiplier(-5), 1.0f);

	FAchievementDefinition UnsafeGrowth(TEXT("test_growth"), EAchievementCategory::Misc, EAchievementMetric::DaysPlayed, EAchievementMetricMode::Cumulative, 1, 1.0f, 1, TEXT("ACHIEVEMENT_NAME_MISC_DAYS_PLAYED"), TEXT("Daily Rhythm"));
	TestEqual(TEXT("Constructor clamps unsafe growth above one"), UnsafeGrowth.Growth, 1.01f);
	FAchievementDefinition HugeThreshold(TEXT("test_huge"), EAchievementCategory::Misc, EAchievementMetric::DaysPlayed, EAchievementMetricMode::Cumulative, MAX_int64, 2.0f, 1, TEXT("ACHIEVEMENT_NAME_MISC_DAYS_PLAYED"), TEXT("Daily Rhythm"));
	TestEqual(TEXT("Max int threshold unlocks once without overflow"), FAchievementFormula::GetTierForValue(HugeThreshold, MAX_int64), 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAchievementServiceProgressTest,
	"IdleProject.GameCore.Achievement.ServiceProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAchievementServiceProgressTest::RunTest(const FString& Parameters)
{
	UAchievementService* Service = NewObject<UAchievementService>();
	Service->InitializeDefaultAchievements();

	TestEqual(TEXT("Fresh service has neutral multiplier"), Service->GetStatMultiplier(), 1.0f);
	TestEqual(TEXT("Fresh service has no points"), Service->GetTotalPoints(), 0);

	Service->RecordMetric(EAchievementMetric::MonstersKilled, 10);
	TestEqual(TEXT("Cumulative metric stores kill count"), Service->GetMetricValue(EAchievementMetric::MonstersKilled), static_cast<int64>(10));
	TestEqual(TEXT("First monster tier grants one point"), Service->GetTotalPoints(), 1);
	TestEqual(TEXT("One point grants one percent"), Service->GetStatMultiplier(), 1.01f);

	Service->RecordMetric(EAchievementMetric::HighestLevelReached, 50);
	Service->RecordMetric(EAchievementMetric::HighestLevelReached, 10);
	TestEqual(TEXT("Max metric keeps the highest value"), Service->GetMetricValue(EAchievementMetric::HighestLevelReached), static_cast<int64>(50));

	Service->RecordMetric(EAchievementMetric::MonstersKilled, 30);
	TestEqual(TEXT("Cumulative metric adds later progress"), Service->GetMetricValue(EAchievementMetric::MonstersKilled), static_cast<int64>(40));
	TestTrue(TEXT("Additional tiers increase total points"), Service->GetTotalPoints() > 1);
	Service->RecordItemCollected(TEXT("sword_001"));
	Service->RecordItemCollected(TEXT("sword_001"));
	Service->RecordItemCollected(TEXT("axe_001"));
	TestEqual(TEXT("Item collection counts every collected item"), Service->GetMetricValue(EAchievementMetric::ItemsCollected), static_cast<int64>(3));
	TestEqual(TEXT("Unique item collection counts each id once"), Service->GetMetricValue(EAchievementMetric::UniqueItemsFound), static_cast<int64>(2));

	TArray<FAchievementMetricSaveEntry> SavedMetrics;
	TArray<FAchievementSaveEntry> SavedAchievements;
	TArray<FName> SavedUniqueItemIds;
	Service->CaptureState(SavedMetrics, SavedAchievements, &SavedUniqueItemIds);
	FAchievementSaveEntry UnknownAchievement;
	UnknownAchievement.AchievementId = TEXT("unknown_achievement");
	UnknownAchievement.Tier = 99;
	SavedAchievements.Add(UnknownAchievement);

	UAchievementService* Restored = NewObject<UAchievementService>();
	Restored->RestoreState(SavedMetrics, SavedAchievements, &SavedUniqueItemIds);
	Restored->RecordItemCollected(TEXT("sword_001"));

	TestEqual(TEXT("Restored kill metric round trips"), Restored->GetMetricValue(EAchievementMetric::MonstersKilled), Service->GetMetricValue(EAchievementMetric::MonstersKilled));
	TestEqual(TEXT("Restored max metric round trips"), Restored->GetMetricValue(EAchievementMetric::HighestLevelReached), Service->GetMetricValue(EAchievementMetric::HighestLevelReached));
	TestEqual(TEXT("Restored unique ids prevent duplicate unique credit"), Restored->GetMetricValue(EAchievementMetric::UniqueItemsFound), Service->GetMetricValue(EAchievementMetric::UniqueItemsFound));
	TestEqual(TEXT("Restored points round trip"), Restored->GetTotalPoints(), Service->GetTotalPoints());
	TestEqual(TEXT("Restored multiplier round trips"), Restored->GetStatMultiplier(), Service->GetStatMultiplier());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAchievementHudViewModelTest,
	"IdleProject.UI.HUD.AchievementPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAchievementHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UAchievementService* Service = NewObject<UAchievementService>();
	Service->InitializeDefaultAchievements();
	Service->RecordMetric(EAchievementMetric::MonstersKilled, 10);
	Service->RecordMetric(EAchievementMetric::HighestLevelReached, 25);

	const FIdleHUDAchievementViewModel ViewModel = IdleProject::UI::BuildAchievementViewModel(*Service);
	TestEqual(TEXT("Achievement panel title uses localized copy"), ViewModel.Title.ToString(), FString(TEXT("Achievements")));
	TestEqual(TEXT("Total points are exposed"), ViewModel.TotalPoints, 3);
	TestEqual(TEXT("Total points label is localized"), ViewModel.TotalPointsLabel.ToString(), FString(TEXT("Points 3")));
	TestEqual(TEXT("Stat multiplier is exposed"), ViewModel.StatMultiplier, 1.03f);
	TestEqual(TEXT("Stat multiplier label is localized"), ViewModel.StatMultiplierLabel.ToString(), FString(TEXT("Stat Bonus x1.03")));
	TestTrue(TEXT("Achievement categories are summarized"), ViewModel.Rows.Num() >= 8);

	const FIdleHUDAchievementRowViewModel* CombatRow = ViewModel.Rows.FindByPredicate([](const FIdleHUDAchievementRowViewModel& Row)
	{
		return Row.Category == EAchievementCategory::Combat;
	});
	TestNotNull(TEXT("Combat category row exists"), CombatRow);
	if (CombatRow)
	{
		TestEqual(TEXT("Combat row label is localized"), CombatRow->CategoryLabel.ToString(), FString(TEXT("Combat")));
		TestEqual(TEXT("Combat row shows unlocked points"), CombatRow->Points, 1);
		TestEqual(TEXT("Combat row shows current tier"), CombatRow->Tier, 1);
		TestEqual(TEXT("Combat row value label is localized"), CombatRow->ValueLabel.ToString(), FString(TEXT("Progress 10")));
	}

	TestEqual(TEXT("Unlock feedback includes achievement name and tier"),
		IdleProject::UI::BuildAchievementUnlockedFeedbackLabel(TEXT("combat_monster_slayer"), 2).ToString(),
		FString(TEXT("Achievement Unlocked: Monster Slayer (Tier 2)")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	TestEqual(TEXT("Unlock feedback uses Korean achievement name"),
		IdleProject::UI::BuildAchievementUnlockedFeedbackLabel(TEXT("combat_monster_slayer"), 2).ToString(),
		FString(TEXT("업적 달성: 몬스터 사냥꾼 (티어 2)")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceAchievementHooksTest,
	"IdleProject.GameCore.Achievement.GameInstanceHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceAchievementHooksTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestEqual(TEXT("Fresh achievement multiplier is neutral"), GameInstance->GetAchievementStatMultiplier(), 1.0f);

	for (int32 Index = 0; Index < 10; ++Index)
	{
		GameInstance->RecordMonsterKilled();
	}
	TestEqual(TEXT("Monster kill hook records achievement metric"), GameInstance->GetAchievementMetricValue(EAchievementMetric::MonstersKilled), static_cast<int64>(10));
	TestEqual(TEXT("Monster kill hook unlocks first point"), GameInstance->GetAchievementTotalPoints(), 1);
	TestEqual(TEXT("Game instance exposes achievement multiplier"), GameInstance->GetAchievementStatMultiplier(), 1.01f);

	GameInstance->RecordGearEnhanced();
	TestEqual(TEXT("Gear enhancement hook records achievement metric"), GameInstance->GetAchievementMetricValue(EAchievementMetric::GearEnhanced), static_cast<int64>(1));
	FItemInstance Item;
	Item.ItemId = TEXT("test_sword");
	Item.Slot = EItemSlot::Weapon;
	Item.Rarity = EItemRarity::Common;
	GameInstance->RecordAchievementItemCollected(Item);
	GameInstance->RecordAchievementItemCollected(Item);
	TestEqual(TEXT("Item collected hook records all collections"), GameInstance->GetAchievementMetricValue(EAchievementMetric::ItemsCollected), static_cast<int64>(2));
	TestEqual(TEXT("Item collected hook records unique item ids once"), GameInstance->GetAchievementMetricValue(EAchievementMetric::UniqueItemsFound), static_cast<int64>(1));

	UIdleSaveGame* Captured = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture includes achievement state"), GameInstance->CaptureToSave(Captured));
	TestTrue(TEXT("Captured achievement metrics are present"), Captured->AchievementMetrics.Num() > 0);
	TestTrue(TEXT("Captured achievement tiers are present"), Captured->Achievements.Num() > 0);

	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply restores achievement state"), Restored->ApplyFromSave(Captured));
	TestEqual(TEXT("Restored achievement points round trip"), Restored->GetAchievementTotalPoints(), GameInstance->GetAchievementTotalPoints());
	TestEqual(TEXT("Restored achievement multiplier round trips"), Restored->GetAchievementStatMultiplier(), GameInstance->GetAchievementStatMultiplier());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleCharacterAchievementDerivedStatsTest,
	"IdleProject.Character.Stats.AchievementMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleCharacterAchievementDerivedStatsTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GameInstance);

	for (int32 Index = 0; Index < 10; ++Index)
	{
		GameInstance->RecordMonsterKilled();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);

	const FPrimaryStats ExpectedPrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	FDerivedStats BaseDerived = FStatFormulas::DeriveStats(ExpectedPrimary, 1);
	const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>();
	if (Skills)
	{
		Skills->ApplyPassivesToStats(BaseDerived);
	}

	const float AchievementMultiplier = GameInstance->GetAchievementStatMultiplier();
	const FDerivedStats CurrentDerived = Character->GetCurrentDerivedStats();

	TestEqual(TEXT("Achievement multiplier affects HP"), CurrentDerived.Hp, BaseDerived.Hp * AchievementMultiplier);
	TestEqual(TEXT("Achievement multiplier affects physical attack"), CurrentDerived.PhysAtk, BaseDerived.PhysAtk * AchievementMultiplier);
	TestEqual(TEXT("Achievement multiplier affects magic attack"), CurrentDerived.MagicAtk, BaseDerived.MagicAtk * AchievementMultiplier);
	TestEqual(TEXT("Achievement multiplier affects physical defense"), CurrentDerived.PhysDef, BaseDerived.PhysDef * AchievementMultiplier);
	TestEqual(TEXT("Achievement multiplier affects magic defense"), CurrentDerived.MagicDef, BaseDerived.MagicDef * AchievementMultiplier);
	TestEqual(TEXT("Achievement multiplier does not alter attack speed"), CurrentDerived.AtkSpeed, BaseDerived.AtkSpeed);
	TestEqual(TEXT("Achievement multiplier does not alter crit rate"), CurrentDerived.CritRate, BaseDerived.CritRate);

	World->DestroyWorld(false);
	return true;
}

#endif
