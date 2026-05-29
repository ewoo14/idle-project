#include "Misc/AutomationTest.h"

#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CombatSystem/SkillComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "CharacterSystem/LevelFormulas.h"
#include "GameCore/DungeonFormula.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MasteryFormula.h"
#include "GameCore/MasteryService.h"
#include "GameCore/QuestService.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
struct FIdleGameInstanceWorldContextAccessor : UIdleGameInstance
{
	static void Attach(UIdleGameInstance* Instance, FWorldContext* Context)
	{
		static_cast<FIdleGameInstanceWorldContextAccessor*>(Instance)->WorldContext = Context;
	}
};

FWorldContext* AttachGameInstanceToTestWorld(UIdleGameInstance* GameInstance, UWorld* World)
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
}

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
	TestEqual(TEXT("Core multiplier level 1 parity anchor"), FMasteryFormula::CoreStatMultiplier(1, 1, 1), 1.0277259349822998f);
	TestEqual(TEXT("Core multiplier level 30 parity anchor"), FMasteryFormula::CoreStatMultiplier(30, 30, 30), 1.0902172327041626f);
	TestEqual(TEXT("Crit level 30 parity anchor"), FMasteryFormula::CritRateAdd(30), 0.034339871257543564f);
	TestEqual(TEXT("Gold find level 100 parity anchor"), FMasteryFormula::GoldFindPct(100), 0.09230241179466248f);
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

	const FMasteryLevelInfo RestoredCombatInfo = Restored->GetTrackLevelInfo(EMasteryTrack::Combat);
	int32 ExpectedLevel = 0;
	int64 ExpectedInto = 0;
	int64 ExpectedNeed = 0;
	FMasteryFormula::LevelFromTotalXp(Restored->GetTrackTotalXp(EMasteryTrack::Combat), ExpectedLevel, ExpectedInto, ExpectedNeed);
	TestEqual(TEXT("Imported combat cached level mirrors formula"), RestoredCombatInfo.Level, ExpectedLevel);
	TestEqual(TEXT("Imported combat cached xp into level mirrors formula"), RestoredCombatInfo.XpIntoLevel, ExpectedInto);
	TestEqual(TEXT("Imported combat cached xp to next mirrors formula"), RestoredCombatInfo.XpToNext, ExpectedNeed);
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

	GameInstance->InitializeStageServiceForTests();
	if (UStageService* StageService = GameInstance->GetStageService())
	{
		StageService->RestoreState(1, UStageService::StagesPerChapter, 0, false, 0);
		StageService->RecordKill(true);
	}
	TestEqual(TEXT("Chapter boss clear grants combat mastery xp"), Mastery ? Mastery->GetTrackTotalXp(EMasteryTrack::Combat) : -1, static_cast<int64>(21));

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
		static_cast<int64>(21));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryUtilityBonusExposureTest,
	"IdleProject.Mastery.UtilityBonusExposure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryUtilityBonusExposureTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
	Save->bHasSave = true;
	Save->SaveVersion = 13;

	FMasterySaveEntry Beast;
	Beast.Track = static_cast<uint8>(EMasteryTrack::Beast);
	Beast.TotalXp = FMasteryFormula::XpToNext(0);
	Save->Mastery.Add(Beast);

	TestTrue(TEXT("Seeded beast mastery save applies"), GameInstance->ApplyFromSave(Save));
	// EXP 마스터리는 AddExp 단일 경로 적용 — 처치 EXP getter는 마스터리를 제외해 이중 적용을 막는다.
	TestEqual(TEXT("Exp boost getter excludes mastery (no rune)"), GameInstance->GetRuneExpBoostBonus(), 0.0f);
	// 골드 마스터리는 처치 경로 getter에서 단일 적용된다.
	TestEqual(TEXT("Gold find getter includes beast mastery bonus"), GameInstance->GetRuneGoldFindBonus(), FMasteryFormula::GoldFindPct(1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryProgressionE2ETest,
	"IdleProject.Mastery.ProgressionE2E",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMasteryProgressionE2ETest::RunTest(const FString& Parameters)
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
	FWorldContext* WorldContext = AttachGameInstanceToTestWorld(GameInstance, World);
	TestNotNull(TEXT("Game instance has a world context"), WorldContext);
	if (!WorldContext)
	{
		World->DestroyWorld(false);
		return false;
	}
	GameInstance->InitializeQuestServiceForTests(TEXT("2026-05-29"));
	GameInstance->InitializeDungeonServiceForTests(TEXT("2026-05-29"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Character)
	{
		World->DestroyWorld(false);
		return false;
	}
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Player controller is spawned"), PlayerController);
	if (!PlayerController)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->AddController(PlayerController);
	PlayerController->Possess(Character);
	TestEqual(TEXT("Test world exposes possessed player character"), World->GetFirstPlayerController() ? Cast<AIdleCharacter>(World->GetFirstPlayerController()->GetPawn()) : nullptr, Character);
	Character->SetClassId(EClassId::Warrior);
	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	Character->RefreshDerivedStats();

	const FDerivedStats BaseDerived = Character->GetCurrentDerivedStats();
	const int64 BaseCombatPower = Character->GetCombatPower();

	for (int32 Index = 0; Index < 100; ++Index)
	{
		GameInstance->RecordMonsterKilled();
	}
	UMasteryService* Mastery = GameInstance->GetMasteryService();
	TestNotNull(TEXT("Mastery service exists after progression hook"), Mastery);
	if (!Mastery)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	TestEqual(TEXT("Monster kill hook grants combat mastery xp"), Mastery->GetTrackTotalXp(EMasteryTrack::Combat), static_cast<int64>(100));
	TestEqual(TEXT("Combat mastery reaches level 1"), Mastery->GetTrackLevel(EMasteryTrack::Combat), 1);

	for (int32 Index = 0; Index < 20; ++Index)
	{
		GameInstance->RecordGearEnhanced();
	}
	TestEqual(TEXT("Gear enhance hook grants equipment mastery xp"), Mastery->GetTrackTotalXp(EMasteryTrack::Equipment), static_cast<int64>(100));
	TestEqual(TEXT("Equipment mastery reaches level 1"), Mastery->GetTrackLevel(EMasteryTrack::Equipment), 1);

	const FDungeonRunResult DungeonResult = GameInstance->TryRunDungeon(EDungeonType::Gold);
	TestTrue(TEXT("Dungeon run succeeds for mastery hook"), DungeonResult.bSuccess);
	TestEqual(TEXT("Dungeon run hook grants abyss mastery xp"), Mastery->GetTrackTotalXp(EMasteryTrack::Abyss), static_cast<int64>(30));

	FQuestState MainQuest;
	TestTrue(TEXT("Main quest state exists"), GameInstance->GetQuestState(TEXT("main_ch1_001"), MainQuest));
	TestTrue(TEXT("Kill hook completes first main quest"), MainQuest.bCompleted);
	const FQuestClaimResult Claim = GameInstance->ClaimQuest(TEXT("main_ch1_001"));
	TestTrue(TEXT("Quest claim succeeds for mastery hook"), Claim.bSuccess);
	TestEqual(TEXT("Quest claim hook grants explore mastery xp"), Mastery->GetTrackTotalXp(EMasteryTrack::Explore), static_cast<int64>(20));

	const FMasteryGlobalBonus Bonus = Mastery->GetGlobalBonus();
	TestEqual(TEXT("World power reflects leveled combat and equipment tracks"), Bonus.WorldPower, static_cast<int64>(2));
	TestTrue(TEXT("Core mastery bonus is active"), Bonus.CoreStatMultiplier > 1.0f);
	TestTrue(TEXT("Derived HP increases through mastery core bonus"), Character->GetCurrentDerivedStats().Hp > BaseDerived.Hp);
	TestTrue(TEXT("Derived physical attack increases through mastery core bonus"), Character->GetCurrentDerivedStats().PhysAtk > BaseDerived.PhysAtk);
	TestTrue(TEXT("Combat power increases after mastery progression"), Character->GetCombatPower() > BaseCombatPower);
	TestEqual(TEXT("Combat power remains derived-stat formula"), Character->GetCombatPower(), FCombatPowerFormula::ComputeCombatPower(Character->GetCurrentDerivedStats()));

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
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
	FMasterySaveEntry Equipment;
	Equipment.Track = static_cast<uint8>(EMasteryTrack::Equipment);
	Equipment.TotalXp = 1000;
	Save->Mastery.Add(Equipment);
	FMasterySaveEntry Explore;
	Explore.Track = static_cast<uint8>(EMasteryTrack::Explore);
	Explore.TotalXp = 1000;
	Save->Mastery.Add(Explore);
	Save->CharacterLevel = 100;
	Save->bChapter1BossDefeated = true;
	TestTrue(TEXT("Seeded mastery save applies"), GameInstance->ApplyFromSave(Save));
	const float BonusBeforeRebirth = GameInstance->GetMasteryCoreStatMultiplier();

	TestTrue(TEXT("Rebirth succeeds"), GameInstance->Rebirth());
	TestEqual(
		TEXT("Rebirth keeps mastery xp"),
		GameInstance->GetMasteryService() ? GameInstance->GetMasteryService()->GetTrackTotalXp(EMasteryTrack::Combat) : -1,
		static_cast<int64>(1000));
	TestEqual(TEXT("Rebirth keeps mastery global bonus"), GameInstance->GetMasteryCoreStatMultiplier(), BonusBeforeRebirth);

	UIdleGameInstance* TranscendGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* TranscendSave = NewObject<UIdleSaveGame>();
	TranscendSave->bHasSave = true;
	TranscendSave->SaveVersion = 13;
	TranscendSave->RebirthCount = 5;
	TranscendSave->Mastery.Add(Combat);
	TranscendSave->Mastery.Add(Equipment);
	TranscendSave->Mastery.Add(Explore);
	TestTrue(TEXT("Seeded transcend mastery save applies"), TranscendGameInstance->ApplyFromSave(TranscendSave));
	const float BonusBeforeTranscend = TranscendGameInstance->GetMasteryCoreStatMultiplier();
	TestTrue(TEXT("Transcend succeeds"), TranscendGameInstance->Transcend());
	TestEqual(
		TEXT("Transcend keeps mastery xp"),
		TranscendGameInstance->GetMasteryService() ? TranscendGameInstance->GetMasteryService()->GetTrackTotalXp(EMasteryTrack::Combat) : -1,
		static_cast<int64>(1000));
	TestEqual(TEXT("Transcend keeps mastery global bonus"), TranscendGameInstance->GetMasteryCoreStatMultiplier(), BonusBeforeTranscend);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryZeroRegressionTest,
	"IdleProject.Mastery.ZeroRegression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMasteryZeroRegressionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 12;
	TestTrue(TEXT("Legacy v12 save applies without mastery payload"), GameInstance->ApplyFromSave(LegacySave));
	World->SetGameInstance(GameInstance);

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

	FDerivedStats ExpectedDerived = FStatFormulas::DeriveStats(FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1), 1);
	if (const USkillComponent* Skills = Character->FindComponentByClass<USkillComponent>())
	{
		Skills->ApplyPassivesToStats(ExpectedDerived);
	}

	const UMasteryService* Mastery = GameInstance->GetMasteryService();
	TestNotNull(TEXT("Mastery service is created for v12 migration"), Mastery);
	TestEqual(TEXT("Zero mastery world power"), Mastery ? Mastery->GetWorldPower() : -1, static_cast<int64>(0));
	TestEqual(TEXT("Zero mastery core multiplier"), GameInstance->GetMasteryCoreStatMultiplier(), 1.0f);
	TestEqual(TEXT("Zero mastery keeps HP unchanged"), Character->GetCurrentDerivedStats().Hp, ExpectedDerived.Hp);
	TestEqual(TEXT("Zero mastery keeps physical attack unchanged"), Character->GetCurrentDerivedStats().PhysAtk, ExpectedDerived.PhysAtk);
	TestEqual(TEXT("Zero mastery keeps crit unchanged"), Character->GetCurrentDerivedStats().CritRate, ExpectedDerived.CritRate);
	TestEqual(TEXT("Zero mastery keeps combat power formula unchanged"), Character->GetCombatPower(), FCombatPowerFormula::ComputeCombatPower(ExpectedDerived));

	World->DestroyWorld(false);
	return true;
}

#endif
