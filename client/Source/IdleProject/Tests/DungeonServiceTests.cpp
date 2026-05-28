#include "Misc/AutomationTest.h"

#include "GameCore/DungeonFormula.h"
#include "GameCore/DungeonService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

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
	TestEqual(TEXT("First run grants gold"), First.GoldReward, static_cast<int64>(3500));
	TestEqual(TEXT("First run consumes one entry"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 2);

	Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	const FDungeonRunResult Fourth = Dungeons->TryRunDungeon(EDungeonType::Gold, 350, TEXT("2026-05-28"));
	TestFalse(TEXT("Fourth same-day run fails"), Fourth.bSuccess);
	TestEqual(TEXT("Fourth run consumes no entry"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-28")), 0);

	const FDungeonRunResult LowCp = Dungeons->TryRunDungeon(EDungeonType::Essence, 499, TEXT("2026-05-28"));
	TestFalse(TEXT("Below CP gate fails"), LowCp.bSuccess);
	TestEqual(TEXT("Failed CP gate consumes no essence entry"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-28")), 3);

	Dungeons->EnsureDailyReset(TEXT("2026-05-29"));
	TestEqual(TEXT("Next UTC date resets gold entries"), Dungeons->GetRemainingEntries(EDungeonType::Gold, TEXT("2026-05-29")), 3);
	TestEqual(TEXT("Next UTC date keeps independent essence entries full"), Dungeons->GetRemainingEntries(EDungeonType::Essence, TEXT("2026-05-29")), 3);

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
	TestEqual(TEXT("Captured save writes V10"), Captured->SaveVersion, static_cast<int32>(10));
	TestEqual(TEXT("Captured dungeon entry array has three rows"), Captured->DungeonEntriesUsed.Num(), 3);

	return true;
}

#endif
