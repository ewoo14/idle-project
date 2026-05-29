#include "Misc/AutomationTest.h"

#include "GameCore/QuestService.h"
#include "GameCore/DungeonService.h"
#include "GameCore/MasteryFormula.h"
#include "GameCore/MasteryService.h"
#include "GameCore/OfflineRewardFormula.h"
#include "Internationalization/IdleLocalization.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/ItemTypes.h"
#include "UI/IdleHUD.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FItemInstance MakeHudTestItem(FName ItemId, EItemSlot Slot, EItemRarity Rarity, float Atk, float Def, float Hp, int32 EnhanceLevel)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = Atk;
	Item.BonusDef = Def;
	Item.BonusHp = Hp;
	Item.EnhanceLevel = EnhanceLevel;
	return Item;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryHudLocalBonusViewModelTest,
	"IdleProject.UI.HUD.MasteryPanelLocalBonusViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMasteryHudLocalBonusViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UMasteryService* MasteryService = NewObject<UMasteryService>();
	MasteryService->Initialize();
	MasteryService->AddXp(EMasteryTrack::Combat, FMasteryFormula::XpToNext(0));
	MasteryService->AddXp(EMasteryTrack::Equipment, FMasteryFormula::XpToNext(0));
	MasteryService->AddXp(EMasteryTrack::Abyss, FMasteryFormula::XpToNext(0));
	MasteryService->AddXp(EMasteryTrack::Rune, FMasteryFormula::XpToNext(0));
	MasteryService->AddXp(EMasteryTrack::Beast, FMasteryFormula::XpToNext(0));
	MasteryService->AddXp(EMasteryTrack::Explore, FMasteryFormula::XpToNext(0));

	const FIdleHUDMasteryPanelViewModel ViewModel = IdleProject::UI::BuildMasteryPanelViewModel(*MasteryService);
	TestEqual(TEXT("Mastery panel title uses approved English copy"), ViewModel.Title.ToString(), FString(TEXT("Mastery")));
	TestEqual(TEXT("Mastery panel exposes six track rows"), ViewModel.Rows.Num(), FMasteryFormula::TrackCount);
	TestEqual(TEXT("Combat local bonus label names kill rewards"), ViewModel.Rows[0].LocalBonusLabel.ToString(), FString(TEXT("Local Bonus: Kill Reward +1%")));
	TestEqual(TEXT("Equipment local bonus label names cost reduction"), ViewModel.Rows[1].LocalBonusLabel.ToString(), FString(TEXT("Local Bonus: Enhance Cost -1%")));
	TestEqual(TEXT("Rune local bonus label names rune core"), ViewModel.Rows[3].LocalBonusLabel.ToString(), FString(TEXT("Local Bonus: Rune Core +1%")));
	TestEqual(TEXT("Combat tooltip explains local effect"), ViewModel.Rows[0].TooltipLabel.ToString(), FString(TEXT("Increases gold and EXP from kills.")));
	TestEqual(TEXT("Equipment tooltip explains local effect"), ViewModel.Rows[1].TooltipLabel.ToString(), FString(TEXT("Reduces enhancement gold cost.")));
	TestEqual(TEXT("Combat local bonus comes from service getter"), ViewModel.Rows[0].LocalBonusPercent, FMath::RoundToInt(MasteryService->GetLocalBonus(EMasteryTrack::Combat) * 100.0f));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonHudViewModelTest,
	"IdleProject.UI.HUD.DungeonPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UDungeonService* Dungeons = NewObject<UDungeonService>();
	Dungeons->EnsureDailyReset(TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Exp, 750, TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Exp, 750, TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Exp, 750, TEXT("2026-05-28"));

	const FIdleHUDDungeonPanelViewModel ViewModel = IdleProject::UI::BuildDungeonPanelViewModel(*Dungeons, 350, TEXT("2026-05-28"));
	TestEqual(TEXT("Dungeon panel title uses localized copy"), ViewModel.Title.ToString(), FString(TEXT("Dungeons")));
	TestEqual(TEXT("Dungeon panel exposes the three dungeon rows"), ViewModel.Rows.Num(), 3);

	const FIdleHUDDungeonRowViewModel& GoldRow = ViewModel.Rows[0];
	TestEqual(TEXT("Gold dungeon row is first"), GoldRow.Type, EDungeonType::Gold);
	TestEqual(TEXT("Gold dungeon name is localized"), GoldRow.NameLabel.ToString(), FString(TEXT("Gold Dungeon")));
	TestEqual(TEXT("Gold remaining entries are formatted"), GoldRow.EntriesLabel.ToString(), FString(TEXT("Entries 2/3")));
	TestEqual(TEXT("Gold CP requirement is shown"), GoldRow.RequiredPowerLabel.ToString(), FString(TEXT("CP 350 / 100")));
	TestEqual(TEXT("Gold reward preview uses combat power formula"), GoldRow.RewardLabel.ToString(), FString(TEXT("Reward Gold +37,417")));
	TestEqual(TEXT("Gold ready row uses enter action copy"), GoldRow.ActionLabel.ToString(), FString(TEXT("Enter")));
	TestTrue(TEXT("Gold ready row enables hitbox"), GoldRow.bCanEnter);

	const FIdleHUDDungeonRowViewModel& ExpRow = ViewModel.Rows[1];
	TestEqual(TEXT("Sold out row keeps entry count at zero"), ExpRow.EntriesLabel.ToString(), FString(TEXT("Entries 0/3")));
	TestEqual(TEXT("Sold out row exposes sold out status"), ExpRow.StatusLabel.ToString(), FString(TEXT("No Entries")));
	TestFalse(TEXT("Sold out row disables hitbox"), ExpRow.bCanEnter);

	const FIdleHUDDungeonRowViewModel& EssenceRow = ViewModel.Rows[2];
	TestEqual(TEXT("Essence dungeon row is third"), EssenceRow.Type, EDungeonType::Essence);
	TestEqual(TEXT("Essence CP requirement is shown"), EssenceRow.RequiredPowerLabel.ToString(), FString(TEXT("CP 350 / 500")));
	TestEqual(TEXT("Low CP row exposes need CP status"), EssenceRow.StatusLabel.ToString(), FString(TEXT("Need CP")));
	TestFalse(TEXT("Low CP row disables hitbox"), EssenceRow.bCanEnter);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FOfflineRewardHudViewModelTest,
	"IdleProject.UI.HUD.OfflineRewardModalViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FOfflineRewardHudViewModelTest::RunTest(const FString& Parameters)
{
	FOfflineRewardResult Reward;
	Reward.CappedSeconds = 3661;
	Reward.Gold = 12345;
	Reward.Exp = 678;
	Reward.TimeBonusMultiplier = 1.02;

	const FIdleHUDOfflineRewardViewModel ViewModel = IdleProject::UI::BuildOfflineRewardViewModel(Reward);
	TestTrue(TEXT("Reward with gold or exp is visible"), ViewModel.bVisible);
	TestEqual(TEXT("Title uses approved Korean copy"), ViewModel.Title.ToString(), FString(TEXT("오프라인 보상")));
	TestEqual(TEXT("Elapsed time is formatted as H:MM"), ViewModel.ElapsedLabel.ToString(), FString(TEXT("경과 시간 1:01")));
	TestEqual(TEXT("Gold line is formatted"), ViewModel.GoldLabel.ToString(), FString(TEXT("골드 +12,345")));
	TestEqual(TEXT("EXP line is formatted"), ViewModel.ExpLabel.ToString(), FString(TEXT("EXP +678")));
	TestEqual(TEXT("Claim button uses approved Korean copy"), ViewModel.ClaimLabel.ToString(), FString(TEXT("수령")));

	FOfflineRewardResult EmptyReward;
	EmptyReward.CappedSeconds = 3600;
	const FIdleHUDOfflineRewardViewModel EmptyViewModel = IdleProject::UI::BuildOfflineRewardViewModel(EmptyReward);
	TestFalse(TEXT("Reward with no gold and no exp is hidden"), EmptyViewModel.bVisible);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuestLogHudViewModelTest,
	"IdleProject.UI.HUD.QuestLogViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FQuestLogHudViewModelTest::RunTest(const FString& Parameters)
{
	TArray<FQuestState> States;

	FQuestState MainQuest;
	MainQuest.QuestId = TEXT("main_ch1_001");
	MainQuest.Type = EQuestType::Main;
	MainQuest.Title = FText::FromString(TEXT("Find the Broken Gate"));
	MainQuest.TargetCount = 5;
	MainQuest.Progress = 3;
	MainQuest.RewardGold = 150;
	MainQuest.RewardExp = 80;
	States.Add(MainQuest);

	FQuestState DailyQuest;
	DailyQuest.QuestId = TEXT("daily_claim_offline");
	DailyQuest.Type = EQuestType::Daily;
	DailyQuest.Title = FText::FromString(TEXT("Claim Rest Rewards"));
	DailyQuest.TargetCount = 1;
	DailyQuest.Progress = 1;
	DailyQuest.bCompleted = true;
	DailyQuest.RewardGold = 300;
	DailyQuest.RewardExp = 180;
	States.Add(DailyQuest);

	FQuestState WeeklyQuest;
	WeeklyQuest.QuestId = TEXT("weekly_climb_tower");
	WeeklyQuest.Type = EQuestType::Weekly;
	WeeklyQuest.Title = FText::FromString(TEXT("Weekly Tower Push"));
	WeeklyQuest.TargetCount = 10;
	WeeklyQuest.Progress = 4;
	WeeklyQuest.RewardGold = 7000;
	WeeklyQuest.RewardExp = 3600;
	States.Add(WeeklyQuest);

	const FIdleHUDQuestLogViewModel ViewModel = IdleProject::UI::BuildQuestLogViewModel(States);
	TestEqual(TEXT("Quest log uses approved title copy"), ViewModel.Title.ToString(), FString(TEXT("퀘스트")));
	TestEqual(TEXT("Quest log exposes all active quests"), ViewModel.Rows.Num(), 3);
	TestEqual(TEXT("Quest log groups rows into main, daily, and weekly sections"), ViewModel.Sections.Num(), 3);
	TestEqual(TEXT("Main section contains main rows"), ViewModel.Sections[0].Rows.Num(), 1);
	TestEqual(TEXT("Daily section contains daily rows"), ViewModel.Sections[1].Rows.Num(), 1);
	TestEqual(TEXT("Weekly section contains weekly rows"), ViewModel.Sections[2].Rows.Num(), 1);
	TestEqual(TEXT("Weekly row remains available in flat row list"), ViewModel.Rows[2].QuestId, FString(TEXT("weekly_climb_tower")));
	TestEqual(TEXT("Main quest row uses approved Korean type label"), ViewModel.Rows[0].TypeLabel.ToString(), FString(TEXT("메인")));
	TestEqual(TEXT("Daily quest row uses approved Korean type label"), ViewModel.Rows[1].TypeLabel.ToString(), FString(TEXT("일일")));
	TestEqual(TEXT("Progress line shows current and target count"), ViewModel.Rows[0].ProgressLabel.ToString(), FString(TEXT("진행 3 / 5")));
	TestEqual(TEXT("Completed unclaimed quest is claimable"), ViewModel.Rows[1].ActionLabel.ToString(), FString(TEXT("수령")));
	TestTrue(TEXT("Completed unclaimed quest enables claim button"), ViewModel.Rows[1].bCanClaim);

	DailyQuest.bClaimed = true;
	States[1] = DailyQuest;
	const FIdleHUDQuestLogViewModel ClaimedViewModel = IdleProject::UI::BuildQuestLogViewModel(States);
	TestEqual(TEXT("Claimed quest action shows received copy"), ClaimedViewModel.Rows[1].ActionLabel.ToString(), FString(TEXT("완료")));
	TestFalse(TEXT("Claimed quest disables claim button"), ClaimedViewModel.Rows[1].bCanClaim);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthHudViewModelTest,
	"IdleProject.UI.HUD.RebirthPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthHudViewModelTest::RunTest(const FString& Parameters)
{
	const FIdleHUDRebirthViewModel LockedViewModel = IdleProject::UI::BuildRebirthViewModel(false, false, 80, 2, 10, 9);
	TestEqual(TEXT("Panel title uses approved Korean copy"), LockedViewModel.Title.ToString(), FString(TEXT("환생")));
	TestEqual(TEXT("Locked status explains rebirth is unavailable"), LockedViewModel.StatusLabel.ToString(), FString(TEXT("환생 조건 미달")));
	TestEqual(TEXT("Boss gate uses approved copy"), LockedViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 필요")));
	TestEqual(TEXT("Level gate shows current progress"), LockedViewModel.LevelLabel.ToString(), FString(TEXT("레벨 80 / 100")));
	TestFalse(TEXT("Locked state disables rebirth button"), LockedViewModel.bCanRebirth);

	const FIdleHUDRebirthViewModel ReadyViewModel = IdleProject::UI::BuildRebirthViewModel(true, true, 150, 4, 26, 18);
	TestEqual(TEXT("Ready status uses approved Korean copy"), ReadyViewModel.StatusLabel.ToString(), FString(TEXT("환생 가능")));
	TestEqual(TEXT("Boss defeated state uses approved copy"), ReadyViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 완료")));
	TestEqual(TEXT("Current count is exposed"), ReadyViewModel.CountLabel.ToString(), FString(TEXT("환생 4회")));
	TestEqual(TEXT("Permanent bonus uses approved copy"), ReadyViewModel.BonusLabel.ToString(), FString(TEXT("영구 보너스 26 포인트")));
	TestEqual(TEXT("Preview reward uses game instance reward"), ReadyViewModel.NextBonusLabel.ToString(), FString(TEXT("이번 환생 보상 +18 포인트")));
	TestEqual(TEXT("Button uses approved Korean copy"), ReadyViewModel.ButtonLabel.ToString(), FString(TEXT("환생 진행")));
	TestTrue(TEXT("Ready state enables rebirth button"), ReadyViewModel.bCanRebirth);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTranscendHudViewModelTest,
	"IdleProject.UI.HUD.TranscendPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTranscendHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FIdleHUDTranscendViewModel LockedViewModel = IdleProject::UI::BuildTranscendViewModel(false, 4, 5, 0, 1.0f, 1.25f);
	TestEqual(TEXT("Panel title uses approved English copy"), LockedViewModel.Title.ToString(), FString(TEXT("Transcendence")));
	TestEqual(TEXT("Locked status explains transcend is unavailable"), LockedViewModel.StatusLabel.ToString(), FString(TEXT("Transcend Locked")));
	TestEqual(TEXT("Requirement label exposes current rebirth progress"), LockedViewModel.RequirementLabel.ToString(), FString(TEXT("Rebirth 4 / 5")));
	TestEqual(TEXT("Current multiplier is formatted with two decimals"), LockedViewModel.CurrentMultiplierLabel.ToString(), FString(TEXT("Current multiplier x1.00")));
	TestEqual(TEXT("Preview multiplier advances from current count"), LockedViewModel.PreviewMultiplierLabel.ToString(), FString(TEXT("Next transcend x1.25")));
	TestEqual(TEXT("Transcend count is exposed"), LockedViewModel.CountLabel.ToString(), FString(TEXT("Transcend 0")));
	TestFalse(TEXT("Locked state disables transcend button"), LockedViewModel.bCanTranscend);

	const FIdleHUDTranscendViewModel ReadyViewModel = IdleProject::UI::BuildTranscendViewModel(true, 5, 5, 1, 1.25f, 1.5f);
	TestEqual(TEXT("Ready status uses approved English copy"), ReadyViewModel.StatusLabel.ToString(), FString(TEXT("Transcend Ready")));
	TestEqual(TEXT("Ready requirement still shows threshold progress"), ReadyViewModel.RequirementLabel.ToString(), FString(TEXT("Rebirth 5 / 5")));
	TestEqual(TEXT("Current multiplier reflects game instance multiplier"), ReadyViewModel.CurrentMultiplierLabel.ToString(), FString(TEXT("Current multiplier x1.25")));
	TestEqual(TEXT("Preview multiplier reflects next transcend"), ReadyViewModel.PreviewMultiplierLabel.ToString(), FString(TEXT("Next transcend x1.50")));
	TestEqual(TEXT("Button uses approved English copy"), ReadyViewModel.ButtonLabel.ToString(), FString(TEXT("Transcend")));
	TestTrue(TEXT("Ready state enables transcend button"), ReadyViewModel.bCanTranscend);

	const FIdleHUDTranscendViewModel OverflowViewModel = IdleProject::UI::BuildTranscendViewModel(true, 7, 5, 2, 1.5f, 1.75f);
	TestEqual(TEXT("Requirement label keeps current rebirth count above threshold"), OverflowViewModel.RequirementLabel.ToString(), FString(TEXT("Rebirth 7 / 5")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSaveProgressHudFeedbackLabelTest,
	"IdleProject.UI.HUD.SaveProgressFeedbackLabel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSaveProgressHudFeedbackLabelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));
	TestEqual(TEXT("Saved feedback uses approved English copy"),
		IdleProject::UI::BuildProgressSavedFeedbackLabel().ToString(),
		FString(TEXT("Saved")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	TestEqual(TEXT("Saved feedback uses approved Korean copy"),
		IdleProject::UI::BuildProgressSavedFeedbackLabel().ToString(),
		FString(TEXT("저장됨")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCloudSyncHudViewModelTest,
	"IdleProject.UI.HUD.CloudSyncViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCloudSyncHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FIdleHUDCloudSyncViewModel IdleViewModel = IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState::Idle);
	TestFalse(TEXT("Idle cloud sync state is hidden"), IdleViewModel.bVisible);

	const FIdleHUDCloudSyncViewModel SyncingViewModel = IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState::Syncing);
	TestTrue(TEXT("Syncing cloud sync state is visible"), SyncingViewModel.bVisible);
	TestEqual(TEXT("Syncing cloud sync state uses approved copy"), SyncingViewModel.Label.ToString(), FString(TEXT("Syncing...")));
	TestFalse(TEXT("Syncing state is not an error"), SyncingViewModel.bError);

	const FIdleHUDCloudSyncViewModel SyncedViewModel = IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState::Synced);
	TestTrue(TEXT("Synced cloud sync state is visible for transient feedback"), SyncedViewModel.bVisible);
	TestEqual(TEXT("Synced cloud sync state uses approved copy"), SyncedViewModel.Label.ToString(), FString(TEXT("Cloud Saved")));
	TestTrue(TEXT("Synced state may fade out"), SyncedViewModel.bTransient);

	const FIdleHUDCloudSyncViewModel OfflineViewModel = IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState::Offline);
	TestTrue(TEXT("Offline cloud sync state stays visible"), OfflineViewModel.bVisible);
	TestEqual(TEXT("Offline cloud sync state uses approved copy"), OfflineViewModel.Label.ToString(), FString(TEXT("Offline")));
	TestTrue(TEXT("Offline state is an error"), OfflineViewModel.bError);
	TestFalse(TEXT("Offline state is persistent"), OfflineViewModel.bTransient);

	const FIdleHUDCloudSyncViewModel ConflictViewModel = IdleProject::UI::BuildCloudSyncViewModel(ECloudSyncState::Conflict);
	TestTrue(TEXT("Conflict cloud sync state stays visible"), ConflictViewModel.bVisible);
	TestEqual(TEXT("Conflict cloud sync state uses approved copy"), ConflictViewModel.Label.ToString(), FString(TEXT("Sync Conflict")));
	TestTrue(TEXT("Conflict state is an error"), ConflictViewModel.bError);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceHudViewModelTest,
	"IdleProject.UI.HUD.EnhancePanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeHudTestItem(TEXT("iron_sword"), EItemSlot::Weapon, EItemRarity::Rare, 12.0f, 0.0f, 0.0f, 2));
	Inventory->AddItem(MakeHudTestItem(TEXT("training_helmet"), EItemSlot::Helmet, EItemRarity::Common, 0.0f, 3.0f, 15.0f, FEnhanceFormula::MaxEnhanceLevel));

	const FIdleHUDEnhancePanelViewModel ViewModel = IdleProject::UI::BuildEnhancePanelViewModel(*Inventory, 4000, FText::GetEmpty(), false);
	TestEqual(TEXT("Enhance panel title uses localized copy"), ViewModel.Title.ToString(), FString(TEXT("Enhancement")));
	TestEqual(TEXT("Enhance panel shows all equipment slots"), ViewModel.Rows.Num(), 8);
	TestEqual(TEXT("Panel gold label exposes current gold"), ViewModel.GoldLabel.ToString(), FString(TEXT("Gold 4,000")));

	const FIdleHUDEnhanceSlotViewModel& WeaponRow = ViewModel.Rows[0];
	TestEqual(TEXT("Weapon row targets weapon slot"), WeaponRow.Slot, EItemSlot::Weapon);
	TestTrue(TEXT("Equipped weapon row is visible as equipped"), WeaponRow.bEquipped);
	TestEqual(TEXT("Weapon row rarity label is localized"), WeaponRow.RarityLabel.ToString(), FString(TEXT("Rare")));
	TestEqual(TEXT("Weapon row rarity color uses rare theme"), WeaponRow.RarityColor, IdleProject::UI::Theme::RarityRare);
	TestEqual(TEXT("Weapon level label shows current level"), WeaponRow.LevelLabel.ToString(), FString(TEXT("+2 / 50")));
	TestEqual(TEXT("Weapon next cost follows rarity-scaled formula"), WeaponRow.Cost, static_cast<int64>(1800));
	TestEqual(TEXT("Weapon cost label is localized"), WeaponRow.CostLabel.ToString(), FString(TEXT("Cost 1,800")));
	TestEqual(TEXT("Weapon success rate follows formula"), WeaponRow.SuccessRate, FEnhanceFormula::GetEnhanceSuccessRate(2));
	TestEqual(TEXT("Weapon success label is localized"), WeaponRow.SuccessRateLabel.ToString(), FString(TEXT("Success 91%")));
	TestTrue(TEXT("Enough gold and below max enables enhance"), WeaponRow.bCanEnhance);
	TestEqual(TEXT("Enabled button uses enhance copy"), WeaponRow.ButtonLabel.ToString(), FString(TEXT("Enhance")));

	const FIdleHUDEnhanceSlotViewModel& HelmetRow = ViewModel.Rows[1];
	TestTrue(TEXT("Max level row is equipped"), HelmetRow.bEquipped);
	TestFalse(TEXT("Max level row disables enhance"), HelmetRow.bCanEnhance);
	TestTrue(TEXT("Max level row is marked max"), HelmetRow.bMaxLevel);
	TestEqual(TEXT("Max level row status is localized"), HelmetRow.StatusLabel.ToString(), FString(TEXT("Max")));

	const FIdleHUDEnhanceSlotViewModel& TopRow = ViewModel.Rows[2];
	TestFalse(TEXT("Empty slot row is not equipped"), TopRow.bEquipped);
	TestFalse(TEXT("Empty slot cannot enhance"), TopRow.bCanEnhance);
	TestEqual(TEXT("Empty slot row status is localized"), TopRow.StatusLabel.ToString(), FString(TEXT("Empty")));

	const FIdleHUDEnhancePanelViewModel PoorViewModel = IdleProject::UI::BuildEnhancePanelViewModel(*Inventory, 100, FText::GetEmpty(), false);
	TestFalse(TEXT("Gold shortage disables weapon enhance"), PoorViewModel.Rows[0].bCanEnhance);
	TestFalse(TEXT("Gold shortage is exposed"), PoorViewModel.Rows[0].bGoldEnough);
	TestEqual(TEXT("Gold shortage row status is localized"), PoorViewModel.Rows[0].StatusLabel.ToString(), FString(TEXT("Need Gold")));

	const FIdleHUDEnhancePanelViewModel SuccessViewModel = IdleProject::UI::BuildEnhancePanelViewModel(
		*Inventory,
		4000,
		FText::FromString(TEXT("Success +3")),
		true);
	TestEqual(TEXT("Feedback label is carried into view model"), SuccessViewModel.FeedbackLabel.ToString(), FString(TEXT("Success +3")));
	TestTrue(TEXT("Feedback success flag is carried into view model"), SuccessViewModel.bFeedbackSuccess);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnhanceHudRarityViewModelTest,
	"IdleProject.UI.HUD.EnhancePanelRarityViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnhanceHudRarityViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeHudTestItem(TEXT("epic_sword"), EItemSlot::Weapon, EItemRarity::Epic, 20.0f, 0.0f, 0.0f, 0));
	Inventory->AddItem(MakeHudTestItem(TEXT("legendary_helmet"), EItemSlot::Helmet, EItemRarity::Legendary, 0.0f, 8.0f, 40.0f, 0));

	const FIdleHUDEnhancePanelViewModel EnglishViewModel = IdleProject::UI::BuildEnhancePanelViewModel(*Inventory, 1000, FText::GetEmpty(), false);
	TestEqual(TEXT("Epic row uses English rarity label"), EnglishViewModel.Rows[0].RarityLabel.ToString(), FString(TEXT("Epic")));
	TestEqual(TEXT("Epic row uses epic theme color"), EnglishViewModel.Rows[0].RarityColor, IdleProject::UI::Theme::RarityEpic);
	TestEqual(TEXT("Legendary row uses English rarity label"), EnglishViewModel.Rows[1].RarityLabel.ToString(), FString(TEXT("Legendary")));
	TestEqual(TEXT("Legendary row uses legendary theme color"), EnglishViewModel.Rows[1].RarityColor, IdleProject::UI::Theme::RarityLegendary);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	const FIdleHUDEnhancePanelViewModel KoreanViewModel = IdleProject::UI::BuildEnhancePanelViewModel(*Inventory, 1000, FText::GetEmpty(), false);
	TestEqual(TEXT("Epic row uses Korean rarity label"), KoreanViewModel.Rows[0].RarityLabel.ToString(), FString(TEXT("에픽")));
	TestEqual(TEXT("Legendary row uses Korean rarity label"), KoreanViewModel.Rows[1].RarityLabel.ToString(), FString(TEXT("전설")));

	return true;
}

#endif
