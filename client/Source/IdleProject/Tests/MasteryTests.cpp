#include "Misc/AutomationTest.h"

#include "IdleGameInstanceTestHelpers.h"
#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/IdleMonster.h"
#include "CombatSystem/SkillComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "CharacterSystem/LevelFormulas.h"
#include "CharacterSystem/StatFormulas.h"
#include "CombatSystem/CombatComponent.h"
#include "GameCore/DungeonFormula.h"
#include "GameCore/DungeonService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MasteryFormula.h"
#include "GameCore/MasteryService.h"
#include "GameCore/OfflineRewardFormula.h"
#include "GameCore/PetLevelFormula.h"
#include "GameCore/PetService.h"
#include "GameCore/QuestService.h"
#include "GameCore/RewardFormula.h"
#include "GameFramework/PlayerController.h"
#include "ItemSystem/EnhanceFormula.h"
#include "ItemSystem/InventoryComponent.h"
#include "ItemSystem/GoldDrop.h"
#include "ItemSystem/ShopFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
struct FIdleMonsterDeathAccessor : AIdleMonster
{
	static void Trigger(AIdleMonster* Monster)
	{
		static_cast<FIdleMonsterDeathAccessor*>(Monster)->HandleDeath(Monster);
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

FMasterySaveEntry MakeMasteryEntry(EMasteryTrack Track, int64 TotalXp = FMasteryFormula::XpToNext(0))
{
	FMasterySaveEntry Entry;
	Entry.Track = static_cast<uint8>(Track);
	Entry.TotalXp = TotalXp;
	return Entry;
}

FItemInstance MakeMasteryEnhanceItem(FName ItemId, EItemSlot Slot, EItemRarity Rarity)
{
	FItemInstance Item;
	Item.ItemId = ItemId;
	Item.Slot = Slot;
	Item.Rarity = Rarity;
	Item.DisplayName = FText::FromName(ItemId);
	Item.BonusAtk = 10.0f;
	return Item;
}

int64 MasteryXpForLevel(int32 TargetLevel)
{
	int64 Total = 0;
	for (int32 Level = 0; Level < FMath::Max(0, TargetLevel); ++Level)
	{
		Total += FMasteryFormula::XpToNext(Level);
	}
	return Total;
}

TArray<EMasteryTrack> AllMasteryTracks()
{
	return {
		EMasteryTrack::Combat,
		EMasteryTrack::Equipment,
		EMasteryTrack::Abyss,
		EMasteryTrack::Rune,
		EMasteryTrack::Beast,
		EMasteryTrack::Explore,
	};
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

	for (const EMasteryTrack Track : AllMasteryTracks())
	{
		TestEqual(TEXT("Local bonus starts at zero"), FMasteryFormula::GetLocalBonus(Track, 0), 0.0f);
		TestEqual(TEXT("Negative local bonus starts at zero"), FMasteryFormula::GetLocalBonus(Track, -1), 0.0f);
		TestTrue(TEXT("Local bonus is monotonic"), FMasteryFormula::GetLocalBonus(Track, 30) > FMasteryFormula::GetLocalBonus(Track, 1));
		TestEqual(TEXT("Local bonus level 1 parity anchor"), FMasteryFormula::GetLocalBonus(Track, 1), 0.006931471638381481f);
		TestEqual(TEXT("Local bonus level 30 parity anchor"), FMasteryFormula::GetLocalBonus(Track, 30), 0.034339871257543564f);
		TestEqual(TEXT("Local bonus level 100 parity anchor"), FMasteryFormula::GetLocalBonus(Track, 100), 0.04615120589733124f);
	}
	const float EquipmentMaxIntBonus = FMasteryFormula::GetLocalBonus(EMasteryTrack::Equipment, MAX_int32);
	TestEqual(TEXT("Equipment local bonus uses fifty percent cap formula"), EquipmentMaxIntBonus, FMath::Min(0.5f, 0.01f * FMath::Loge(1.0f + static_cast<float>(MAX_int32))));
	TestTrue(TEXT("Equipment local bonus never exceeds fifty percent"), EquipmentMaxIntBonus <= 0.5f);

	// V2 로컬 보너스(2종) — 1종과 동일 ln 곡선·계수.
	for (const EMasteryTrack Track : AllMasteryTracks())
	{
		TestEqual(TEXT("Local bonus2 starts at zero"), FMasteryFormula::GetLocalBonus2(Track, 0), 0.0f);
		TestEqual(TEXT("Negative local bonus2 starts at zero"), FMasteryFormula::GetLocalBonus2(Track, -1), 0.0f);
		TestTrue(TEXT("Local bonus2 is monotonic"), FMasteryFormula::GetLocalBonus2(Track, 30) > FMasteryFormula::GetLocalBonus2(Track, 1));
	}
	// 비클램프 트랙(Combat/Abyss/Rune/Explore)은 1종과 동일 float 앵커.
	for (const EMasteryTrack Track : { EMasteryTrack::Combat, EMasteryTrack::Abyss, EMasteryTrack::Rune, EMasteryTrack::Explore })
	{
		TestEqual(TEXT("Local bonus2 level 1 parity anchor"), FMasteryFormula::GetLocalBonus2(Track, 1), 0.006931471638381481f);
		TestEqual(TEXT("Local bonus2 level 30 parity anchor"), FMasteryFormula::GetLocalBonus2(Track, 30), 0.034339871257543564f);
		TestEqual(TEXT("Local bonus2 level 100 parity anchor"), FMasteryFormula::GetLocalBonus2(Track, 100), 0.04615120589733124f);
	}
	// Equipment/Beast 2종은 비용 절감이라 0.5 상한 클램프.
	const float EquipmentMaxIntBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Equipment, MAX_int32);
	const float BeastMaxIntBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Beast, MAX_int32);
	TestTrue(TEXT("Equipment local bonus2 never exceeds fifty percent"), EquipmentMaxIntBonus2 <= 0.5f);
	TestTrue(TEXT("Beast local bonus2 never exceeds fifty percent"), BeastMaxIntBonus2 <= 0.5f);
	TestEqual(TEXT("Equipment local bonus2 caps at fifty percent"), EquipmentMaxIntBonus2, FMath::Min(0.5f, 0.01f * FMath::Loge(1.0f + static_cast<float>(MAX_int32))));
	TestEqual(TEXT("Beast local bonus2 caps at fifty percent"), BeastMaxIntBonus2, FMath::Min(0.5f, 0.01f * FMath::Loge(1.0f + static_cast<float>(MAX_int32))));

	// 심연 2종: 던전 일일 입장 정수 보너스 임계(floor(level/50), 상한 +3).
	TestEqual(TEXT("Abyss bonus entries are zero at level 0"), FMasteryFormula::GetAbyssBonusEntries(0), 0);
	TestEqual(TEXT("Abyss bonus entries are zero below first threshold"), FMasteryFormula::GetAbyssBonusEntries(49), 0);
	TestEqual(TEXT("Abyss bonus entries reach one at level 50"), FMasteryFormula::GetAbyssBonusEntries(50), 1);
	TestEqual(TEXT("Abyss bonus entries reach two at level 100"), FMasteryFormula::GetAbyssBonusEntries(100), 2);
	TestEqual(TEXT("Abyss bonus entries reach three at level 150"), FMasteryFormula::GetAbyssBonusEntries(150), 3);
	TestEqual(TEXT("Abyss bonus entries cap at three"), FMasteryFormula::GetAbyssBonusEntries(MAX_int32), 3);
	TestEqual(TEXT("Negative abyss bonus entries clamp to zero"), FMasteryFormula::GetAbyssBonusEntries(-100), 0);
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
	TestEqual(TEXT("Service exposes combat local bonus"), Service->GetLocalBonus(EMasteryTrack::Combat), FMasteryFormula::GetLocalBonus(EMasteryTrack::Combat, 2));
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
	TestEqual(TEXT("Capture writes v28"), CapturedSave->SaveVersion, 28);
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
	const float BeastExpBoost = FMasteryFormula::ExpBoostPct(1);
	const float BeastGoldFind = FMasteryFormula::GoldFindPct(1);
	GameInstance->AddExp(50);
	TestEqual(TEXT("AddExp applies beast global exp boost once"), GameInstance->GetCurrentExp(), FMath::RoundToInt64(50.0 * (1.0 + static_cast<double>(BeastExpBoost))));
	TestEqual(TEXT("Exp boost getter excludes mastery (no rune)"), GameInstance->GetRuneExpBoostBonus(), 0.0f);
	// 골드 마스터리는 처치 경로 getter에서 단일 적용된다.
	TestEqual(TEXT("Gold find getter includes beast global mastery"), GameInstance->GetRuneGoldFindBonus(), BeastGoldFind);
	GameInstance->InitializePetSeasonServicesForTests();
	TestTrue(TEXT("Dog equips for beast local/global separation"), GameInstance->EquipPet(TEXT("dog")));
	const float BeastLocalBonus = FMasteryFormula::GetLocalBonus(EMasteryTrack::Beast, 1);
	TestEqual(TEXT("Beast local bonus scales pet gold only"), GameInstance->GetEquippedPetGoldBonusPercent(), 20.0f * (1.0f + BeastLocalBonus));
	TestEqual(TEXT("Pet gold application excludes beast global gold find"), GameInstance->ApplyEquippedPetGoldBonus(1000), FMath::FloorToInt64(1000.0 * (1.0 + static_cast<double>(20.0f * (1.0f + BeastLocalBonus)) / 100.0)));

	UIdleGameInstance* AbyssGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* AbyssSave = NewObject<UIdleSaveGame>();
	AbyssSave->bHasSave = true;
	AbyssSave->SaveVersion = 13;
	AbyssSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Abyss));
	TestTrue(TEXT("Seeded abyss mastery save applies"), AbyssGameInstance->ApplyFromSave(AbyssSave));
	TestEqual(TEXT("Drop chance includes abyss global mastery add"), AbyssGameInstance->ApplyEquippedPetDropBonusChance(0.05f), 0.05f + FMasteryFormula::DropRateAdd(1));
	TestEqual(TEXT("Abyss local bonus is separate from global drop add"), AbyssGameInstance->GetMasteryService() ? AbyssGameInstance->GetMasteryService()->GetLocalBonus(EMasteryTrack::Abyss) : -1.0f, FMasteryFormula::GetLocalBonus(EMasteryTrack::Abyss, 1));
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
	FMasteryLocalBonusApplicationTest,
	"IdleProject.Mastery.LocalBonusApplication",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMasteryLocalBonusApplicationTest::RunTest(const FString& Parameters)
{
	UWorld* CombatWorld = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Combat local bonus world exists"), CombatWorld);
	if (!CombatWorld)
	{
		return false;
	}
	UIdleGameInstance* CombatGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* CombatSave = NewObject<UIdleSaveGame>();
	CombatSave->bHasSave = true;
	CombatSave->SaveVersion = 14;
	CombatSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Combat));
	TestTrue(TEXT("Combat mastery save applies"), CombatGameInstance->ApplyFromSave(CombatSave));
	FWorldContext* CombatWorldContext = AttachGameInstanceToTestWorld(CombatGameInstance, CombatWorld);
	TestNotNull(TEXT("Combat local bonus world context exists"), CombatWorldContext);
	if (!CombatWorldContext)
	{
		CombatWorld->DestroyWorld(false);
		return false;
	}
	FActorSpawnParameters CombatSpawnParams;
	CombatSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleMonster* Monster = CombatWorld->SpawnActor<AIdleMonster>(AIdleMonster::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, CombatSpawnParams);
	TestNotNull(TEXT("Combat local bonus monster exists"), Monster);
	if (!Monster || !Monster->GetCombat())
	{
		GEngine->DestroyWorldContext(CombatWorld);
		CombatWorld->DestroyWorld(false);
		return false;
	}
	Monster->SetStageGlobalIndex(1);
	FIdleMonsterDeathAccessor::Trigger(Monster);
	const int64 BaseKillExp = FRewardFormula::ComputeKillExp(12, 1, false, false);
	const int64 ExpectedCombatLocalExp = FMath::RoundToInt64(static_cast<double>(BaseKillExp) * (1.0 + static_cast<double>(FMasteryFormula::GetLocalBonus(EMasteryTrack::Combat, 1))));
	TestEqual(TEXT("Combat local bonus scales kill EXP once before AddExp"), CombatGameInstance->GetCurrentExp(), ExpectedCombatLocalExp);
	GEngine->DestroyWorldContext(CombatWorld);
	CombatWorld->DestroyWorld(false);

	UIdleGameInstance* EquipmentGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* EquipmentSave = NewObject<UIdleSaveGame>();
	EquipmentSave->bHasSave = true;
	EquipmentSave->SaveVersion = 14;
	EquipmentSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Equipment));
	TestTrue(TEXT("Equipment mastery save applies"), EquipmentGameInstance->ApplyFromSave(EquipmentSave));
	UInventoryComponent* Inventory = NewObject<UInventoryComponent>();
	Inventory->AddItem(MakeMasteryEnhanceItem(TEXT("rare_mastery_sword"), EItemSlot::Weapon, EItemRarity::Rare));
	const int64 RareBaseCost = FEnhanceFormula::GetEnhanceCost(0, EItemRarity::Rare);
	EquipmentGameInstance->AddGold(RareBaseCost);
	const FEnhanceAttemptResult Enhance = EquipmentGameInstance->TryEnhanceEquipped(EItemSlot::Weapon, Inventory);
	const int64 ExpectedReducedCost = FMath::RoundToInt64(static_cast<double>(RareBaseCost) * (1.0 - static_cast<double>(FMasteryFormula::GetLocalBonus(EMasteryTrack::Equipment, 1))));
	TestTrue(TEXT("Equipment local bonus still allows enhance attempt"), Enhance.bAttempted);
	TestEqual(TEXT("Equipment local bonus reduces enhance cost by formula"), Enhance.GoldSpent, ExpectedReducedCost);
	TestEqual(TEXT("Equipment local bonus leaves unspent reduced gold"), EquipmentGameInstance->GetGold(), RareBaseCost - Enhance.GoldSpent);

	UIdleGameInstance* BeastGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* BeastSave = NewObject<UIdleSaveGame>();
	BeastSave->bHasSave = true;
	BeastSave->SaveVersion = 14;
	BeastSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Beast));
	TestTrue(TEXT("Beast mastery save applies"), BeastGameInstance->ApplyFromSave(BeastSave));
	BeastGameInstance->InitializePetSeasonServicesForTests();
	TestTrue(TEXT("Dog remains equippable for beast local bonus"), BeastGameInstance->EquipPet(TEXT("dog")));
	TestTrue(TEXT("Beast local bonus scales pet gold percent"), BeastGameInstance->GetEquippedPetGoldBonusPercent() > 20.0f);
	TestTrue(TEXT("Beast local bonus scales applied pet gold"), BeastGameInstance->ApplyEquippedPetGoldBonus(1000) > 1200);

	UIdleGameInstance* ExploreGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* ExploreSave = NewObject<UIdleSaveGame>();
	ExploreSave->bHasSave = true;
	ExploreSave->SaveVersion = 14;
	ExploreSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Explore));
	TestTrue(TEXT("Explore mastery save applies"), ExploreGameInstance->ApplyFromSave(ExploreSave));
	ExploreGameInstance->InitializeQuestServiceForTests(TEXT("2026-05-29"));
	ExploreGameInstance->RecordQuestProgress(EQuestObjective::KillMonster, 5);
	const FQuestClaimResult Claim = ExploreGameInstance->ClaimQuest(TEXT("main_ch1_001"));
	TestTrue(TEXT("Explore local bonus quest claim succeeds"), Claim.bSuccess);
	TestEqual(TEXT("Explore local bonus scales quest gold by formula"), Claim.RewardGold, FMath::RoundToInt64(150.0 * (1.0 + static_cast<double>(FMasteryFormula::GetLocalBonus(EMasteryTrack::Explore, 1)))));
	TestEqual(TEXT("Explore local bonus scales quest exp by formula"), Claim.RewardExp, FMath::RoundToInt64(80.0 * (1.0 + static_cast<double>(FMasteryFormula::GetLocalBonus(EMasteryTrack::Explore, 1)))));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient world exists for dungeon and rune local bonus"), World);
	if (!World)
	{
		return false;
	}
	UIdleGameInstance* DungeonGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* DungeonSave = NewObject<UIdleSaveGame>();
	DungeonSave->bHasSave = true;
	DungeonSave->SaveVersion = 14;
	DungeonSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Abyss));
	TestTrue(TEXT("Abyss mastery save applies"), DungeonGameInstance->ApplyFromSave(DungeonSave));
	FWorldContext* WorldContext = AttachGameInstanceToTestWorld(DungeonGameInstance, World);
	TestNotNull(TEXT("Dungeon test world context exists"), WorldContext);
	if (!WorldContext)
	{
		World->DestroyWorld(false);
		return false;
	}
	DungeonGameInstance->InitializeDungeonServiceForTests(TEXT("2026-05-29"));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Dungeon local bonus character exists"), Character);
	TestNotNull(TEXT("Dungeon local bonus controller exists"), PlayerController);
	if (!Character || !PlayerController)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	World->AddController(PlayerController);
	PlayerController->Possess(Character);
	DungeonGameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	Character->RefreshDerivedStats();
	const FDungeonRunResult BaseDungeon = FDungeonFormula::GetRewardForCp(EDungeonType::Gold, Character->GetCombatPower());
	const FDungeonRunResult MasteryDungeon = DungeonGameInstance->TryRunDungeon(EDungeonType::Gold);
	TestTrue(TEXT("Dungeon local bonus run succeeds"), MasteryDungeon.bSuccess);
	TestEqual(TEXT("Abyss local bonus scales dungeon gold by formula"), MasteryDungeon.GoldReward, FMath::RoundToInt64(static_cast<double>(BaseDungeon.GoldReward) * (1.0 + static_cast<double>(FMasteryFormula::GetLocalBonus(EMasteryTrack::Abyss, 1)))));
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);

	UWorld* RuneWorld = UWorld::CreateWorld(EWorldType::Game, false);
	UIdleGameInstance* RuneGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* RuneSave = NewObject<UIdleSaveGame>();
	RuneSave->bHasSave = true;
	RuneSave->SaveVersion = 14;
	RuneSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Rune));
	TestTrue(TEXT("Rune mastery save applies"), RuneGameInstance->ApplyFromSave(RuneSave));
	AttachGameInstanceToTestWorld(RuneGameInstance, RuneWorld);
	RuneGameInstance->InitializeRuneServiceForTests();
	AIdleCharacter* RuneCharacter = RuneWorld->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Rune local bonus character exists"), RuneCharacter);
	if (!RuneCharacter)
	{
		GEngine->DestroyWorldContext(RuneWorld);
		RuneWorld->DestroyWorld(false);
		return false;
	}
	RuneCharacter->SetClassId(EClassId::Warrior);
	const FDerivedStats ExpectedZeroRune = FStatFormulas::DeriveStats(FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1), 1);
	TestTrue(TEXT("Rune local bonus increases physical attack through rune core path"), RuneCharacter->GetCurrentDerivedStats().PhysAtk > ExpectedZeroRune.PhysAtk);
	GEngine->DestroyWorldContext(RuneWorld);
	RuneWorld->DestroyWorld(false);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMasteryLocalBonus2ApplicationTest,
	"IdleProject.Mastery.LocalBonus2Application",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMasteryLocalBonus2ApplicationTest::RunTest(const FString& Parameters)
{
	// 전투/룬 2종 효과를 분명히 검증하기 위해 충분히 높은 레벨에서 비율을 키운다(로그 곡선이라 저레벨은 반올림에 묻힘).
	const int32 HighLevel = 100;
	const int64 HighLevelXp = MasteryXpForLevel(HighLevel);
	const float CombatBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Combat, HighLevel);
	const float EquipmentBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Equipment, 1);
	const float RuneBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Rune, HighLevel);
	const float BeastBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Beast, 1);
	const float ExploreBonus2 = FMasteryFormula::GetLocalBonus2(EMasteryTrack::Explore, 1);

	// 처치 골드 헬퍼: 동일 RNG 시드로 몬스터를 처치하고 GoldDrop->Amount 를 추출. 기준/마스터리 인스턴스 비교용.
	const auto CaptureKillGold = [](UIdleGameInstance* GameInstance, int32 Seed) -> int64
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		if (!World)
		{
			return -1;
		}
		FWorldContext* Context = AttachGameInstanceToTestWorld(GameInstance, World);
		if (!Context)
		{
			World->DestroyWorld(false);
			return -1;
		}
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AIdleMonster* Monster = World->SpawnActor<AIdleMonster>(AIdleMonster::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!Monster)
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
			return -1;
		}
		Monster->SetStageGlobalIndex(1);
		FMath::RandInit(Seed);
		FIdleMonsterDeathAccessor::Trigger(Monster);
		int64 Amount = -1;
		for (TActorIterator<AGoldDrop> It(World); It; ++It)
		{
			Amount = It->Amount;
			break;
		}
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return Amount;
	};

	// ── 전투 2종: 처치 골드 보상 ×(1 + GetLocalBonus2(Combat)). 기준(0레벨) 대비 마스터리 인스턴스 비교. ──
	{
		const int32 Seed = 424242;
		UIdleGameInstance* BaseGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* BaseSave = NewObject<UIdleSaveGame>();
		BaseSave->bHasSave = true;
		BaseSave->SaveVersion = 16;
		TestTrue(TEXT("Base combat gold save applies"), BaseGameInstance->ApplyFromSave(BaseSave));
		const int64 BaseGold = CaptureKillGold(BaseGameInstance, Seed);

		UIdleGameInstance* CombatGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Combat, HighLevelXp));
		TestTrue(TEXT("Combat gold2 mastery save applies"), CombatGameInstance->ApplyFromSave(Save));
		const int64 MasteryGold = CaptureKillGold(CombatGameInstance, Seed);

		TestTrue(TEXT("Combat gold2 captured base drop"), BaseGold > 0);
		TestEqual(TEXT("Combat local bonus2 scales kill gold once"), MasteryGold, FMath::RoundToInt64(static_cast<double>(BaseGold) * (1.0 + static_cast<double>(CombatBonus2))));
		TestTrue(TEXT("Combat local bonus2 increases gold above base"), MasteryGold > BaseGold);
	}

	// ── 장비 2종: 잠재 큐브(재설정/등급) 골드 가격 ×(1 - GetLocalBonus2(Equipment)). ──
	{
		UIdleGameInstance* EquipGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Equipment));
		TestTrue(TEXT("Equipment cube2 mastery save applies"), EquipGameInstance->ApplyFromSave(Save));
		const int64 BaseResetCost = FShopFormula::GetResetCubeCost(0);
		const int64 ExpectedResetCost = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseResetCost) * (1.0 - static_cast<double>(EquipmentBonus2))));
		EquipGameInstance->AddGold(BaseResetCost);
		TestTrue(TEXT("Reset cube purchase succeeds with mastery"), EquipGameInstance->TryBuyResetCube());
		TestTrue(TEXT("Equipment local bonus2 reduces reset cube cost"), ExpectedResetCost < BaseResetCost);
		TestEqual(TEXT("Equipment local bonus2 leaves reduced cube cost as remaining gold"), EquipGameInstance->GetGold(), BaseResetCost - ExpectedResetCost);

		const int64 BaseRankCost = FShopFormula::GetRankCubeCost(0);
		const int64 ExpectedRankCost = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseRankCost) * (1.0 - static_cast<double>(EquipmentBonus2))));
		UIdleGameInstance* RankGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* RankSave = NewObject<UIdleSaveGame>();
		RankSave->bHasSave = true;
		RankSave->SaveVersion = 16;
		RankSave->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Equipment));
		TestTrue(TEXT("Rank cube2 mastery save applies"), RankGameInstance->ApplyFromSave(RankSave));
		RankGameInstance->AddGold(BaseRankCost);
		TestTrue(TEXT("Rank cube purchase succeeds with mastery"), RankGameInstance->TryBuyRankCube());
		TestEqual(TEXT("Equipment local bonus2 reduces rank cube cost"), RankGameInstance->GetGold(), BaseRankCost - ExpectedRankCost);

		// 회귀: 마스터리 0레벨이면 큐브 가격 절감 없음.
		UIdleGameInstance* PlainGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* PlainSave = NewObject<UIdleSaveGame>();
		PlainSave->bHasSave = true;
		PlainSave->SaveVersion = 16;
		TestTrue(TEXT("Plain cube save applies"), PlainGameInstance->ApplyFromSave(PlainSave));
		PlainGameInstance->AddGold(BaseResetCost);
		TestTrue(TEXT("Plain reset cube purchase succeeds"), PlainGameInstance->TryBuyResetCube());
		TestEqual(TEXT("Zero equipment mastery keeps full cube cost"), PlainGameInstance->GetGold(), static_cast<int64>(0));
	}

	// ── 룬 2종: 분해 에센스 획득 ×(1 + GetLocalBonus2(Rune)). 기준 인스턴스와 비교. ──
	{
		UWorld* RuneWorld = UWorld::CreateWorld(EWorldType::Game, false);
		TestNotNull(TEXT("Rune essence2 world exists"), RuneWorld);
		if (!RuneWorld)
		{
			return false;
		}
		// 마스터리 적용 인스턴스(고레벨 — 로그 곡선상 저레벨은 반올림에 묻힘). Mythic 룬으로 기준 환급량을 키운다.
		UIdleGameInstance* RuneGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Rune, HighLevelXp));
		TestTrue(TEXT("Rune essence2 mastery save applies"), RuneGameInstance->ApplyFromSave(Save));
		AttachGameInstanceToTestWorld(RuneGameInstance, RuneWorld);
		RuneGameInstance->InitializeRuneServiceForTests();
		FRuneInstance RuneA;
		RuneA.RuneId = TEXT("essence_rune_a");
		RuneA.RuneType = ERuneType::PhysAtk;
		RuneA.Rarity = EItemRarity::Mythic;
		FRuneInstance RuneB = RuneA;
		RuneB.RuneId = TEXT("essence_rune_b");
		RuneGameInstance->AddRune(RuneA);
		RuneGameInstance->AddRune(RuneB);
		const int64 EssenceBefore = RuneGameInstance->GetRuneEssence();
		TestTrue(TEXT("Mastery rune disenchant succeeds"), RuneGameInstance->TryDisenchantRune(1));
		const int64 MasteryRefund = RuneGameInstance->GetRuneEssence() - EssenceBefore;

		// 기준(마스터리 0레벨) 인스턴스 — 동일 룬 분해 환급량.
		UIdleGameInstance* BaseRuneGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* BaseSave = NewObject<UIdleSaveGame>();
		BaseSave->bHasSave = true;
		BaseSave->SaveVersion = 16;
		TestTrue(TEXT("Base rune save applies"), BaseRuneGameInstance->ApplyFromSave(BaseSave));
		AttachGameInstanceToTestWorld(BaseRuneGameInstance, RuneWorld);
		BaseRuneGameInstance->InitializeRuneServiceForTests();
		BaseRuneGameInstance->AddRune(RuneA);
		BaseRuneGameInstance->AddRune(RuneB);
		const int64 BaseEssenceBefore = BaseRuneGameInstance->GetRuneEssence();
		TestTrue(TEXT("Base rune disenchant succeeds"), BaseRuneGameInstance->TryDisenchantRune(1));
		const int64 BaseRefund = BaseRuneGameInstance->GetRuneEssence() - BaseEssenceBefore;

		const int64 ExpectedRefund = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseRefund) * (1.0 + static_cast<double>(RuneBonus2))));
		TestEqual(TEXT("Rune local bonus2 scales disenchant essence by formula"), MasteryRefund, ExpectedRefund);
		TestTrue(TEXT("Rune local bonus2 increases essence above base refund"), MasteryRefund > BaseRefund);

		GEngine->DestroyWorldContext(RuneWorld);
		RuneWorld->DestroyWorld(false);
	}

	// ── 야성 2종: 펫 먹이 골드 비용 ×(1 - GetLocalBonus2(Beast)). ──
	{
		UIdleGameInstance* BeastGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Beast));
		TestTrue(TEXT("Beast feed2 mastery save applies"), BeastGameInstance->ApplyFromSave(Save));
		BeastGameInstance->InitializePetSeasonServicesForTests();
		TestTrue(TEXT("Dog equips for beast feed2"), BeastGameInstance->EquipPet(TEXT("dog")));
		const int64 BaseFeedCost = FPetLevelFormula::GetFeedCost(0);
		const int64 ExpectedFeedCost = FMath::Max<int64>(1, FMath::RoundToInt64(static_cast<double>(BaseFeedCost) * (1.0 - static_cast<double>(BeastBonus2))));
		BeastGameInstance->AddGold(BaseFeedCost);
		const FPetFeedResult Feed = BeastGameInstance->TryFeedPet(TEXT("dog"));
		TestTrue(TEXT("Beast feed2 succeeds at reduced cost"), Feed.bFed);
		TestTrue(TEXT("Beast local bonus2 reduces feed cost"), ExpectedFeedCost < BaseFeedCost);
		TestEqual(TEXT("Beast local bonus2 charges reduced feed cost"), Feed.GoldSpent, ExpectedFeedCost);
		TestEqual(TEXT("Beast local bonus2 leaves change after reduced feed"), BeastGameInstance->GetGold(), BaseFeedCost - ExpectedFeedCost);
	}

	// ── 탐험 2종: 오프라인 보상 ×(1 + GetLocalBonus2(Explore)). 기준 인스턴스와 비교. ──
	{
		const int64 LastSeen = 1000;
		const int64 Now = LastSeen + FOfflineRewardFormula::OFFLINE_CAP_SECONDS;

		UIdleGameInstance* ExploreGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->CharacterLevel = 100;
		Save->LastSeenUnixSec = LastSeen;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Explore));
		TestTrue(TEXT("Explore offline2 mastery save applies"), ExploreGameInstance->ApplyFromSave(Save));
		const FOfflineRewardResult MasteryReward = ExploreGameInstance->PreviewOfflineRewards(Now, 0);

		UIdleGameInstance* BaseGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* BaseSave = NewObject<UIdleSaveGame>();
		BaseSave->bHasSave = true;
		BaseSave->SaveVersion = 16;
		BaseSave->CharacterLevel = 100;
		BaseSave->LastSeenUnixSec = LastSeen;
		TestTrue(TEXT("Base offline save applies"), BaseGameInstance->ApplyFromSave(BaseSave));
		const FOfflineRewardResult BaseReward = BaseGameInstance->PreviewOfflineRewards(Now, 0);

		const int64 ExpectedGold = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseReward.Gold) * (1.0 + static_cast<double>(ExploreBonus2))));
		const int64 ExpectedExp = FMath::Max<int64>(0, FMath::RoundToInt64(static_cast<double>(BaseReward.Exp) * (1.0 + static_cast<double>(ExploreBonus2))));
		TestTrue(TEXT("Offline base reward is positive"), BaseReward.Gold > 0);
		TestEqual(TEXT("Explore local bonus2 scales offline gold by formula"), MasteryReward.Gold, ExpectedGold);
		TestEqual(TEXT("Explore local bonus2 scales offline exp by formula"), MasteryReward.Exp, ExpectedExp);
		TestTrue(TEXT("Explore local bonus2 increases offline gold above base"), MasteryReward.Gold > BaseReward.Gold);
	}

	// ── 심연 2종: 던전 일일 입장 +N(정수). 레벨 50 임계에서 +1 노출. ──
	{
		UIdleGameInstance* AbyssGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
		Save->bHasSave = true;
		Save->SaveVersion = 16;
		Save->Mastery.Add(MakeMasteryEntry(EMasteryTrack::Abyss, MasteryXpForLevel(50)));
		TestTrue(TEXT("Abyss entries2 mastery save applies"), AbyssGameInstance->ApplyFromSave(Save));
		UMasteryService* Mastery = AbyssGameInstance->GetMasteryService();
		TestNotNull(TEXT("Abyss entries2 mastery service exists"), Mastery);
		if (Mastery)
		{
			TestEqual(TEXT("Abyss mastery reaches level 50"), Mastery->GetTrackLevel(EMasteryTrack::Abyss), 50);
			TestEqual(TEXT("Abyss level 50 grants one bonus dungeon entry"), Mastery->GetAbyssBonusEntries(), 1);
		}

		// 0레벨 회귀: 보너스 입장 없음.
		UIdleGameInstance* PlainGameInstance = NewObject<UIdleGameInstance>();
		UIdleSaveGame* PlainSave = NewObject<UIdleSaveGame>();
		PlainSave->bHasSave = true;
		PlainSave->SaveVersion = 16;
		TestTrue(TEXT("Plain abyss save applies"), PlainGameInstance->ApplyFromSave(PlainSave));
		TestEqual(TEXT("Zero abyss mastery grants no bonus entry"), PlainGameInstance->GetMasteryService() ? PlainGameInstance->GetMasteryService()->GetAbyssBonusEntries() : -1, 0);
	}

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
	for (const EMasteryTrack Track : AllMasteryTracks())
	{
		TestEqual(TEXT("Zero mastery local bonus"), Mastery ? Mastery->GetLocalBonus(Track) : -1.0f, 0.0f);
	}
	TestEqual(TEXT("Zero mastery core multiplier"), GameInstance->GetMasteryCoreStatMultiplier(), 1.0f);
	TestEqual(TEXT("Zero mastery keeps HP unchanged"), Character->GetCurrentDerivedStats().Hp, ExpectedDerived.Hp);
	TestEqual(TEXT("Zero mastery keeps physical attack unchanged"), Character->GetCurrentDerivedStats().PhysAtk, ExpectedDerived.PhysAtk);
	TestEqual(TEXT("Zero mastery keeps crit unchanged"), Character->GetCurrentDerivedStats().CritRate, ExpectedDerived.CritRate);
	TestEqual(TEXT("Zero mastery keeps combat power formula unchanged"), Character->GetCombatPower(), FCombatPowerFormula::ComputeCombatPower(ExpectedDerived));

	World->DestroyWorld(false);
	return true;
}

#endif
