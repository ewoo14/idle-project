#include "Misc/AutomationTest.h"

#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MasteryFormula.h"
#include "GameCore/MasteryService.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryFormulaTest,
	"IdleProject.Mastery.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryFormulaTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Track count"), FMasteryFormula::TrackCount, 6);
	TestEqual(TEXT("XpToNext(0)"), FMasteryFormula::XpToNext(0), static_cast<int64>(100));
	TestTrue(TEXT("XpToNext is monotonic"), FMasteryFormula::XpToNext(10) > FMasteryFormula::XpToNext(9));

	int32 Level = -1;
	int64 Into = -1;
	int64 Need = -1;
	FMasteryFormula::LevelFromTotalXp(0, Level, Into, Need);
	TestEqual(TEXT("0 xp level"), Level, 0);
	TestEqual(TEXT("0 xp into level"), Into, static_cast<int64>(0));
	TestEqual(TEXT("0 xp next"), Need, static_cast<int64>(100));

	FMasteryFormula::LevelFromTotalXp(100, Level, Into, Need);
	TestEqual(TEXT("100 xp level"), Level, 1);
	TestEqual(TEXT("100 xp into level"), Into, static_cast<int64>(0));

	TestEqual(TEXT("Core multiplier starts at one"), FMasteryFormula::CoreStatMultiplier(0, 0, 0), 1.0f);
	TestTrue(TEXT("Core multiplier is monotonic"), FMasteryFormula::CoreStatMultiplier(10, 10, 10) > FMasteryFormula::CoreStatMultiplier(1, 1, 1));
	TestEqual(TEXT("Crit starts at zero"), FMasteryFormula::CritRateAdd(0), 0.0f);
	TestTrue(TEXT("Gold find is monotonic"), FMasteryFormula::GoldFindPct(50) > FMasteryFormula::GoldFindPct(5));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryServiceTest,
	"IdleProject.Mastery.Service",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryServiceTest::RunTest(const FString& Parameters)
{
	UMasteryService* Service = NewObject<UMasteryService>();
	Service->Initialize();

	TestEqual(TEXT("Initial combat level"), Service->GetTrackLevel(EMasteryTrack::Combat), 0);
	Service->AddXp(EMasteryTrack::Combat, 250);
	TestEqual(TEXT("250 combat xp reaches level 2"), Service->GetTrackLevel(EMasteryTrack::Combat), 2);
	TestEqual(TEXT("Combat total xp is stored"), Service->GetTrackTotalXp(EMasteryTrack::Combat), static_cast<int64>(250));
	const FMasteryLevelInfo CombatLevelInfo = Service->GetTrackLevelInfo(EMasteryTrack::Combat);
	TestEqual(TEXT("Level info exposes current level"), CombatLevelInfo.Level, 2);
	TestEqual(TEXT("Level info exposes xp into level"), CombatLevelInfo.XpIntoLevel, static_cast<int64>(36));
	TestEqual(TEXT("Level info exposes xp to next"), CombatLevelInfo.XpToNext, FMasteryFormula::XpToNext(2));

	Service->AddXp(EMasteryTrack::Beast, 1000);
	const FMasteryGlobalBonus Bonus = Service->GetGlobalBonus();
	TestTrue(TEXT("Gold bonus is positive"), Bonus.GoldFindPct > 0.0f);
	TestEqual(TEXT("World power mirrors level sum"), Bonus.WorldPower, Service->GetWorldPower());

	const TArray<FMasterySaveEntry> Saved = Service->ExportSave();
	UMasteryService* Restored = NewObject<UMasteryService>();
	Restored->Initialize();
	Restored->ImportSave(Saved);
	TestEqual(TEXT("Combat xp round trips"), Restored->GetTrackTotalXp(EMasteryTrack::Combat), static_cast<int64>(250));
	TestEqual(TEXT("Beast level round trips"), Restored->GetTrackLevel(EMasteryTrack::Beast), Service->GetTrackLevel(EMasteryTrack::Beast));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryCombatPowerPressureTest,
	"IdleProject.Mastery.CombatPowerPressure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryCombatPowerPressureTest::RunTest(const FString& Parameters)
{
	FDerivedStats BaseStats;
	BaseStats.Hp = 100.0f;
	BaseStats.PhysAtk = 10.0f;
	BaseStats.MagicAtk = 5.0f;
	BaseStats.PhysDef = 4.0f;
	BaseStats.MagicDef = 3.0f;
	BaseStats.CritRate = 0.05f;
	BaseStats.CritDmg = 0.5f;
	BaseStats.AtkSpeed = 1.0f;
	const int64 BasePower = FCombatPowerFormula::ComputeCombatPower(BaseStats);

	const float CoreMultiplier = FMasteryFormula::CoreStatMultiplier(10, 10, 10);
	FDerivedStats MasteryStats = BaseStats;
	MasteryStats.Hp *= CoreMultiplier;
	MasteryStats.PhysAtk *= CoreMultiplier;
	MasteryStats.MagicAtk *= CoreMultiplier;
	MasteryStats.PhysDef *= CoreMultiplier;
	MasteryStats.MagicDef *= CoreMultiplier;
	MasteryStats.CritRate += FMasteryFormula::CritRateAdd(10);

	TestTrue(TEXT("Mastery bonuses increase combat power"), FCombatPowerFormula::ComputeCombatPower(MasteryStats) > BasePower);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryGameInstanceHooksTest,
	"IdleProject.Mastery.GameInstanceHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryGameInstanceHooksTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* EmptySave = NewObject<UIdleSaveGame>();
	EmptySave->bHasSave = true;
	EmptySave->SaveVersion = 12;
	TestTrue(TEXT("Legacy v12 save applies"), GameInstance->ApplyFromSave(EmptySave));

	UMasteryService* Mastery = GameInstance->GetMasteryService();
	TestNotNull(TEXT("Mastery service exists after restore"), Mastery);
	TestEqual(TEXT("Legacy v12 combat xp starts at zero"), Mastery ? Mastery->GetTrackTotalXp(EMasteryTrack::Combat) : -1, static_cast<int64>(0));

	GameInstance->RecordMonsterKilled();
	TestEqual(TEXT("Monster kill grants combat mastery xp"), Mastery ? Mastery->GetTrackTotalXp(EMasteryTrack::Combat) : -1, static_cast<int64>(1));

	FRuneInstance Rune;
	Rune.RuneId = TEXT("test_rune");
	Rune.RuneType = ERuneType::PhysAtk;
	Rune.Rarity = EItemRarity::Common;
	GameInstance->AddRune(Rune);
	TestEqual(TEXT("Adding rune grants rune mastery xp"), Mastery ? Mastery->GetTrackTotalXp(EMasteryTrack::Rune) : -1, static_cast<int64>(5));

	UIdleSaveGame* CapturedSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(CapturedSave));
	TestEqual(TEXT("Capture writes v13"), CapturedSave->SaveVersion, 13);
	TestEqual(TEXT("Mastery save includes six tracks"), CapturedSave->Mastery.Num(), FMasteryFormula::TrackCount);

	UIdleGameInstance* RestoredGameInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Captured mastery save applies"), RestoredGameInstance->ApplyFromSave(CapturedSave));
	TestEqual(
		TEXT("Combat mastery survives save round trip"),
		RestoredGameInstance->GetMasteryService() ? RestoredGameInstance->GetMasteryService()->GetTrackTotalXp(EMasteryTrack::Combat) : -1,
		static_cast<int64>(1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryResetPersistenceTest,
	"IdleProject.Mastery.ResetPersistence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryResetPersistenceTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
	Save->bHasSave = true;
	Save->SaveVersion = 13;
	FMasterySaveEntry Combat;
	Combat.Track = static_cast<uint8>(EMasteryTrack::Combat);
	Combat.TotalXp = 1000;
	Save->Mastery.Add(Combat);
	Save->CharacterLevel = 100;
	Save->bChapter1BossDefeated = true;
	TestTrue(TEXT("Seeded mastery save applies"), GameInstance->ApplyFromSave(Save));

	TestTrue(TEXT("Rebirth succeeds"), GameInstance->Rebirth());
	TestEqual(
		TEXT("Rebirth keeps mastery xp"),
		GameInstance->GetMasteryService() ? GameInstance->GetMasteryService()->GetTrackTotalXp(EMasteryTrack::Combat) : -1,
		static_cast<int64>(1000));

	UIdleGameInstance* TranscendGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* TranscendSave = NewObject<UIdleSaveGame>();
	TranscendSave->bHasSave = true;
	TranscendSave->SaveVersion = 13;
	TranscendSave->RebirthCount = 5;
	TranscendSave->Mastery.Add(Combat);
	TestTrue(TEXT("Seeded transcend mastery save applies"), TranscendGameInstance->ApplyFromSave(TranscendSave));
	TestTrue(TEXT("Transcend succeeds"), TranscendGameInstance->Transcend());
	TestEqual(
		TEXT("Transcend keeps mastery xp"),
		TranscendGameInstance->GetMasteryService() ? TranscendGameInstance->GetMasteryService()->GetTrackTotalXp(EMasteryTrack::Combat) : -1,
		static_cast<int64>(1000));
	return true;
}

#endif
