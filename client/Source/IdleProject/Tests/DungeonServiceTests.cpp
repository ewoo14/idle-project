#include "Misc/AutomationTest.h"

#include "IdleGameInstanceTestHelpers.h"
#include "GameCore/DungeonFormula.h"
#include "GameCore/DungeonService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MasteryFormula.h"
#include "Internationalization/IdleLocalization.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
FWorldContext* AttachGameInstanceToDungeonTestWorld(UIdleGameInstance* GameInstance, UWorld* World)
{
	if (!GEngine || !GameInstance || !World)
	{
		return nullptr;
	}

	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	Context.OwningGameInstance = GameInstance;
	FIdleGameInstanceWorldContextAccessor::Attach(GameInstance, &Context);
	World->SetGameInstance(GameInstance);
	return &Context;
}

FMasterySaveEntry MakeDungeonMasteryEntry(EMasteryTrack Track, int64 TotalXp = FMasteryFormula::XpToNext(0))
{
	FMasterySaveEntry Entry;
	Entry.Track = static_cast<uint8>(Track);
	Entry.TotalXp = TotalXp;
	return Entry;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonServiceRunLimitResetTest,
	"IdleProject.GameCore.Dungeon.ServiceRunLimitReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonServiceRunLimitResetTest::RunTest(const FString& Parameters)
{
	UDungeonService* Dungeons = NewObject<UDungeonService>();
	Dungeons->EnsureDailyReset(TEXT("2026-05-28"));

	TestEqual(TEXT("Gold starts with three entries"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 3);

	const FDungeonRunResult First = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	TestTrue(TEXT("Enough CP runs gold dungeon"), First.bSuccess);
	TestEqual(TEXT("First run grants gold"), First.GoldReward, static_cast<int64>(37417));
	TestEqual(TEXT("First run consumes one entry"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 2);

	Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	const FDungeonRunResult Fourth = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	TestFalse(TEXT("Fourth same-day run fails"), Fourth.bSuccess);
	TestEqual(TEXT("Fourth run consumes no entry"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 0);

	const FDungeonRunResult LowCp = Dungeons->TryRunDungeon(EDungeonType::Essence, 499, TEXT("2026-05-28"));
	TestFalse(TEXT("Below CP gate fails"), LowCp.bSuccess);
	TestEqual(TEXT("Failed CP gate consumes no essence entry"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-28")), 3);

	const FDungeonRunResult LockedTier = Dungeons->TryRunDungeon(EDungeonType::Essence, 999, TEXT("2026-05-28"), 2);
	TestFalse(TEXT("Locked tier fails"), LockedTier.bSuccess);
	TestEqual(TEXT("Locked tier consumes no essence entry"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-28")), 3);

	const FDungeonRunResult TierOne = Dungeons->TryRunDungeon(EDungeonType::Essence, 2000, TEXT("2026-05-28"), 1);
	TestTrue(TEXT("Tier one run succeeds"), TierOne.bSuccess);
	const FDungeonRunResult TierThree = Dungeons->TryRunDungeon(EDungeonType::Essence, 2000, TEXT("2026-05-28"), 3);
	TestTrue(TEXT("Tier three run succeeds"), TierThree.bSuccess);
	TestEqual(TEXT("Tier three triples tier one reward"), TierThree.EssenceReward, TierOne.EssenceReward * 3);
	TestEqual(TEXT("Dungeon entries are shared across tiers"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-28")), 1);

	Dungeons->EnsureDailyReset(TEXT("2026-05-29"));
	TestEqual(TEXT("Next UTC date resets gold entries"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-29")), 3);
	TestEqual(TEXT("Next UTC date keeps independent essence entries full"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-29")), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonServiceAbyssBonusEntriesTest,
	"IdleProject.GameCore.Dungeon.ServiceAbyssBonusEntries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonServiceAbyssBonusEntriesTest::RunTest(const FString& Parameters)
{
	UDungeonService* Dungeons = NewObject<UDungeonService>();
	const FString Today = TEXT("2026-05-28");
	Dungeons->EnsureDailyReset(Today);

	// 심연 마스터리 2종: 보너스 입장 +2 → 기본 3 + 2 = 5회 입장 가능.
	TestEqual(TEXT("Bonus entries extend remaining count"), Dungeons->GetRemainingEntries(EDungeonType::Gold, Today, 2), 5);

	for (int32 Index = 0; Index < 3; ++Index)
	{
		const FDungeonRunResult Run = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, Today, 1, 2);
		TestTrue(TEXT("Base entry run succeeds with bonus"), Run.bSuccess);
	}
	// 기본 3회 소진 후에도 보너스 입장 덕분에 4·5번째 입장 가능.
	TestEqual(TEXT("Two bonus entries remain after base limit"), Dungeons->GetRemainingEntries(EDungeonType::Gold, Today, 2), 2);
	const FDungeonRunResult FourthRun = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, Today, 1, 2);
	TestTrue(TEXT("Fourth run succeeds via bonus entry"), FourthRun.bSuccess);
	const FDungeonRunResult FifthRun = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, Today, 1, 2);
	TestTrue(TEXT("Fifth run succeeds via bonus entry"), FifthRun.bSuccess);
	const FDungeonRunResult SixthRun = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, Today, 1, 2);
	TestFalse(TEXT("Sixth run fails after extended limit"), SixthRun.bSuccess);
	TestEqual(TEXT("Extended limit fully consumed"), Dungeons->GetRemainingEntries(EDungeonType::Gold, Today, 2), 0);

	// 보너스 0이면 기존 동작과 동일(회귀).
	UDungeonService* Baseline = NewObject<UDungeonService>();
	Baseline->EnsureDailyReset(Today);
	TestEqual(TEXT("Zero bonus keeps base three entries"), Baseline->GetRemainingEntries(EDungeonType::Gold, Today, 0), 3);
	TestEqual(TEXT("Default argument keeps base three entries"), Baseline->GetRemainingEntries(EDungeonType::Gold, Today), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonServiceTierGateAndEntryTest,
	"IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonServiceTierGateAndEntryTest::RunTest(const FString& Parameters)
{
	UDungeonService* Dungeons = NewObject<UDungeonService>();
	const FString Today = TEXT("2026-05-29");
	Dungeons->EnsureDailyReset(Today);

	for (const EDungeonType Type : { EDungeonType::Gold, EDungeonType::Exp, EDungeonType::Essence })
	{
		const int64 MinCp = FDungeonFormula::GetMinimumCp(Type);
		TestEqual(TEXT("CP below minimum unlocks zero tiers"), Dungeons->GetMaxAccessibleTier(Type, MinCp - 1), 0);
		TestEqual(TEXT("CP at minimum unlocks tier one"), Dungeons->GetMaxAccessibleTier(Type, MinCp), 1);
		TestEqual(TEXT("CP at four times minimum unlocks tier three"), Dungeons->GetMaxAccessibleTier(Type, MinCp * 4), 3);
		TestEqual(TEXT("Tier one requirement equals minimum CP"), Dungeons->GetTierCpRequirement(Type, 1), MinCp);
		TestEqual(TEXT("Tier three requirement is four times minimum CP"), Dungeons->GetTierCpRequirement(Type, 3), MinCp * 4);
	}

	const int64 EssenceTierThreeRequirement = Dungeons->GetTierCpRequirement(EDungeonType::Essence, 3);
	const FDungeonRunResult OneBelow = Dungeons->TryRunDungeon(EDungeonType::Essence, EssenceTierThreeRequirement - 1, Today, 3);
	TestFalse(TEXT("One below tier requirement fails"), OneBelow.bSuccess);
	TestEqual(TEXT("One below tier requirement grants zero essence"), OneBelow.EssenceReward, static_cast<int64>(0));
	TestEqual(TEXT("Failed tier gate consumes no daily entry"), Dungeons->GetRemainingEntries(EDungeonType::Essence, Today), 3);

	const FDungeonRunResult AtRequirement = Dungeons->TryRunDungeon(EDungeonType::Essence, EssenceTierThreeRequirement, Today, 3);
	TestTrue(TEXT("At tier requirement succeeds"), AtRequirement.bSuccess);
	TestEqual(TEXT("At tier requirement consumes one shared entry"), Dungeons->GetRemainingEntries(EDungeonType::Essence, Today), 2);
	TestEqual(TEXT("Tier three reward is the formula tier multiplier"), AtRequirement.EssenceReward, FDungeonFormula::GetRewardForCp(EDungeonType::Essence, EssenceTierThreeRequirement, 1).EssenceReward * FDungeonFormula::GetTierRewardMultiplier(3));

	const FDungeonRunResult TierOneDefault = FDungeonFormula::GetRewardForCp(EDungeonType::Exp, 1000);
	const FDungeonRunResult TierOneExplicit = FDungeonFormula::GetRewardForCp(EDungeonType::Exp, 1000, 1);
	TestEqual(TEXT("Tier one remains compatible with the legacy default call"), TierOneExplicit.ExpReward, TierOneDefault.ExpReward);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonServiceCaptureRestoreTest,
	"IdleProject.GameCore.Dungeon.CaptureRestore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonServiceCaptureRestoreTest::RunTest(const FString& Parameters)
{
	UDungeonService* Source = NewObject<UDungeonService>();
	Source->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	Source->TryRunDungeon(EDungeonType::Essence, 1200, TEXT("2026-05-28"));

	TMap<EDungeonType, int32> CapturedEntries;
	FString CapturedResetDate;
	Source->CaptureState(CapturedEntries, CapturedResetDate);

	UDungeonService* Restored = NewObject<UDungeonService>();
	Restored->RestoreState(CapturedEntries, CapturedResetDate);

	TestEqual(TEXT("Captured reset date round trips"), CapturedResetDate, FString(TEXT("2026-05-28")));
	TestEqual(TEXT("Gold used count round trips"), Restored->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 2);
	TestEqual(TEXT("Exp unused count remains full"), Restored->GetRemainingEntries(EDungeonType::Exp, TEXT("2026-05-28")), 3);
	TestEqual(TEXT("Essence used count round trips"), Restored->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-28")), 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceDungeonAbyssTierBonusTest,
	"IdleProject.GameCore.Dungeon.AbyssTierBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceDungeonAbyssTierBonusTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient dungeon world exists"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
	Save->bHasSave = true;
	Save->SaveVersion = 14;
	Save->Mastery.Add(MakeDungeonMasteryEntry(EMasteryTrack::Abyss));
	TestTrue(TEXT("Abyss mastery save applies"), GameInstance->ApplyFromSave(Save));

	FWorldContext* WorldContext = AttachGameInstanceToDungeonTestWorld(GameInstance, World);
	TestNotNull(TEXT("Dungeon test world context exists"), WorldContext);
	if (!WorldContext)
	{
		World->DestroyWorld(false);
		return false;
	}

	GameInstance->InitializeDungeonServiceForTests(TEXT("2026-05-29"));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Dungeon bonus character exists"), Character);
	TestNotNull(TEXT("Dungeon bonus controller exists"), PlayerController);
	if (!Character || !PlayerController)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}

	World->AddController(PlayerController);
	PlayerController->Possess(Character);
	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	Character->RefreshDerivedStats();

	const int64 CombatPower = Character->GetCombatPower();
	const int32 Tier = 3;
	TestTrue(TEXT("Test character can enter tier three gold dungeon"), CombatPower >= FDungeonFormula::GetTierCpRequirement(EDungeonType::Gold, Tier));

	const FDungeonRunResult BaseTierReward = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, CombatPower, Tier);
	const float AbyssLocalBonus = FMasteryFormula::GetLocalBonus(EMasteryTrack::Abyss, 1);
	const FDungeonRunResult MasteryTierReward = GameInstance->TryRunDungeon(EDungeonType::Gold, Tier);
	TestTrue(TEXT("Abyss tier dungeon run succeeds"), MasteryTierReward.bSuccess);
	TestEqual(TEXT("Abyss local bonus applies once after tier multiplication"), MasteryTierReward.GoldReward, FMath::RoundToInt64(static_cast<double>(BaseTierReward.GoldReward) * (1.0 + static_cast<double>(AbyssLocalBonus))));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceDungeonSaveMigrationTest,
	"IdleProject.GameCore.Dungeon.SaveMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceDungeonSaveMigrationTest::RunTest(const FString& Parameters)
{
	UIdleSaveGame* Legacy = NewObject<UIdleSaveGame>();
	Legacy->SaveVersion = 9;
	Legacy->bHasSave = true;

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply accepts v9 save without dungeon payload"), GameInstance->ApplyFromSave(Legacy));

	const UDungeonService* Dungeons = GameInstance->GetDungeonService();
	TestNotNull(TEXT("Dungeon service is ensured after v9 migration"), Dungeons);
	TestEqual(TEXT("Migrated v9 save starts with full gold entries"), Dungeons ? Dungeons->GetRemainingEntries(EDungeonType::Gold, UQuestService::GetCurrentUtcDateString()) : INDEX_NONE, 3);

	UIdleSaveGame* Captured = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture after migration succeeds"), GameInstance->CaptureToSave(Captured));
	TestEqual(TEXT("Captured save writes V22"), Captured->SaveVersion, static_cast<int32>(22));
	TestEqual(TEXT("Captured dungeon entry array has three rows"), Captured->DungeonEntriesUsed.Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDungeonHudTierViewModelTest,
	"IdleProject.UI.HUD.DungeonTierPanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDungeonHudTierViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UDungeonService* Dungeons = NewObject<UDungeonService>();
	const FIdleHUDDungeonPanelViewModel ViewModel = IdleProject::UI::BuildDungeonPanelViewModel(*Dungeons, 400, TEXT("2026-05-29"));

	TestEqual(TEXT("Dungeon panel exposes three rows"), ViewModel.Rows.Num(), 3);
	if (ViewModel.Rows.Num() < 1)
	{
		return false;
	}

	const FIdleHUDDungeonRowViewModel& GoldRow = ViewModel.Rows[0];
	TestEqual(TEXT("Gold row selects the highest accessible tier"), GoldRow.SelectedTier, 3);
	TestEqual(TEXT("Gold row exposes max accessible tier"), GoldRow.MaxAccessibleTier, 3);
	TestEqual(TEXT("Selected tier CP requirement is exposed"), GoldRow.RequiredPower, static_cast<int64>(400));
	TestEqual(TEXT("Next locked tier CP requirement is exposed"), GoldRow.NextTierRequirement, static_cast<int64>(800));
	TestEqual(TEXT("Tier label is localized"), GoldRow.TierLabel.ToString(), FString(TEXT("Tier 3 / 3")));
	TestEqual(TEXT("Next tier label is localized"), GoldRow.NextTierLabel.ToString(), FString(TEXT("Next Tier CP 800")));
	TestEqual(TEXT("Reward preview uses selected tier multiplier"), GoldRow.RewardLabel.ToString(), FString(TEXT("Reward Gold +120,000")));
	TestEqual(TEXT("Enter hitbox carries selected tier"), GoldRow.EnterHitBoxName, FName(TEXT("DungeonEnter_1_3")));

	const FIdleHUDDungeonPanelViewModel LockedViewModel = IdleProject::UI::BuildDungeonPanelViewModel(*Dungeons, 99, TEXT("2026-05-29"));
	TestEqual(TEXT("Locked gold row keeps tier one selected"), LockedViewModel.Rows[0].SelectedTier, 1);
	TestEqual(TEXT("Locked gold row exposes zero max tier"), LockedViewModel.Rows[0].MaxAccessibleTier, 0);
	TestEqual(TEXT("Locked gold row shows tier one requirement"), LockedViewModel.Rows[0].RequiredPower, static_cast<int64>(100));
	TestTrue(TEXT("Locked gold row needs CP"), LockedViewModel.Rows[0].bNeedsPower);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

#endif
