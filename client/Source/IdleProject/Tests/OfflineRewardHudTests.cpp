#include "Misc/AutomationTest.h"

#include "GameCore/QuestService.h"
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

	const FIdleHUDQuestLogViewModel ViewModel = IdleProject::UI::BuildQuestLogViewModel(States);
	TestEqual(TEXT("Quest log uses approved title copy"), ViewModel.Title.ToString(), FString(TEXT("퀘스트")));
	TestEqual(TEXT("Quest log exposes all active quests"), ViewModel.Rows.Num(), 2);
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
	const FIdleHUDRebirthViewModel LockedViewModel = IdleProject::UI::BuildRebirthViewModel(false, false, 80, 2, 10);
	TestEqual(TEXT("Panel title uses approved Korean copy"), LockedViewModel.Title.ToString(), FString(TEXT("환생")));
	TestEqual(TEXT("Locked status explains rebirth is unavailable"), LockedViewModel.StatusLabel.ToString(), FString(TEXT("환생 조건 미달")));
	TestEqual(TEXT("Boss gate uses approved copy"), LockedViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 필요")));
	TestEqual(TEXT("Level gate shows current progress"), LockedViewModel.LevelLabel.ToString(), FString(TEXT("레벨 80 / 100")));
	TestFalse(TEXT("Locked state disables rebirth button"), LockedViewModel.bCanRebirth);

	const FIdleHUDRebirthViewModel ReadyViewModel = IdleProject::UI::BuildRebirthViewModel(true, true, 100, 2, 10);
	TestEqual(TEXT("Ready status uses approved Korean copy"), ReadyViewModel.StatusLabel.ToString(), FString(TEXT("환생 가능")));
	TestEqual(TEXT("Boss defeated state uses approved copy"), ReadyViewModel.BossLabel.ToString(), FString(TEXT("보스 격파 완료")));
	TestEqual(TEXT("Current count is exposed"), ReadyViewModel.CountLabel.ToString(), FString(TEXT("환생 2회")));
	TestEqual(TEXT("Permanent bonus uses approved copy"), ReadyViewModel.BonusLabel.ToString(), FString(TEXT("영구 보너스 10 포인트")));
	TestEqual(TEXT("Next bonus explains fixed reward"), ReadyViewModel.NextBonusLabel.ToString(), FString(TEXT("환생 진행 시 +5 포인트")));
	TestEqual(TEXT("Button uses approved Korean copy"), ReadyViewModel.ButtonLabel.ToString(), FString(TEXT("환생 진행")));
	TestTrue(TEXT("Ready state enables rebirth button"), ReadyViewModel.bCanRebirth);

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

	const FIdleHUDEnhancePanelViewModel ViewModel = IdleProject::UI::BuildEnhancePanelViewModel(*Inventory, 1000, FText::GetEmpty(), false);
	TestEqual(TEXT("Enhance panel title uses localized copy"), ViewModel.Title.ToString(), FString(TEXT("Enhancement")));
	TestEqual(TEXT("Enhance panel shows all equipment slots"), ViewModel.Rows.Num(), 8);
	TestEqual(TEXT("Panel gold label exposes current gold"), ViewModel.GoldLabel.ToString(), FString(TEXT("Gold 1,000")));

	const FIdleHUDEnhanceSlotViewModel& WeaponRow = ViewModel.Rows[0];
	TestEqual(TEXT("Weapon row targets weapon slot"), WeaponRow.Slot, EItemSlot::Weapon);
	TestTrue(TEXT("Equipped weapon row is visible as equipped"), WeaponRow.bEquipped);
	TestEqual(TEXT("Weapon row rarity label is localized"), WeaponRow.RarityLabel.ToString(), FString(TEXT("Rare")));
	TestEqual(TEXT("Weapon row rarity color uses rare theme"), WeaponRow.RarityColor, IdleProject::UI::Theme::RarityRare);
	TestEqual(TEXT("Weapon level label shows current level"), WeaponRow.LevelLabel.ToString(), FString(TEXT("+2 / +5")));
	TestEqual(TEXT("Weapon next cost follows formula"), WeaponRow.Cost, static_cast<int64>(900));
	TestEqual(TEXT("Weapon cost label is localized"), WeaponRow.CostLabel.ToString(), FString(TEXT("Cost 900")));
	TestEqual(TEXT("Weapon success rate follows formula"), WeaponRow.SuccessRate, 0.70f);
	TestEqual(TEXT("Weapon success label is localized"), WeaponRow.SuccessRateLabel.ToString(), FString(TEXT("Success 70%")));
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
		1000,
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
	TestEqual(TEXT("Epic row uses Korean rarity label"), KoreanViewModel.Rows[0].RarityLabel.ToString(), FString(TEXT("영웅")));
	TestEqual(TEXT("Legendary row uses Korean rarity label"), KoreanViewModel.Rows[1].RarityLabel.ToString(), FString(TEXT("전설")));

	return true;
}

#endif
