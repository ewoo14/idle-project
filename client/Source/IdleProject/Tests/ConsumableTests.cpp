#include "Misc/AutomationTest.h"

#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameCore/BuffService.h"
#include "GameCore/ConsumableFormula.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameFramework/PlayerController.h"
#include "Internationalization/IdleLocalization.h"
#include "UI/IdleHUD.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
struct FConsumableTestWorldContextAccessor : UIdleGameInstance
{
	static void Attach(UIdleGameInstance* Instance, FWorldContext* Context)
	{
		static_cast<FConsumableTestWorldContextAccessor*>(Instance)->WorldContext = Context;
	}
};

FWorldContext* AttachConsumableGameInstanceToTestWorld(UIdleGameInstance* GameInstance, UWorld* World)
{
	if (!GEngine || !GameInstance || !World)
	{
		return nullptr;
	}

	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	Context.OwningGameInstance = GameInstance;
	FConsumableTestWorldContextAccessor::Attach(GameInstance, &Context);
	World->SetGameInstance(GameInstance);
	return &Context;
}

AIdleCharacter* SpawnConsumableCharacter(UWorld* World)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (PlayerController && Character)
	{
		World->AddController(PlayerController);
		PlayerController->Possess(Character);
	}
	return Character;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableFormulaAnchorsTest,
	"IdleProject.Consumable.FormulaAnchors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConsumableFormulaAnchorsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Attack tonic percent"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic), 0.30f);
	TestEqual(TEXT("Guard tonic percent"), FConsumableFormula::GetBuffPercent(EConsumableType::GuardTonic), 0.30f);
	TestEqual(TEXT("All stat elixir percent"), FConsumableFormula::GetBuffPercent(EConsumableType::AllStatElixir), 0.20f);
	TestEqual(TEXT("Fortune scroll drop add"), FConsumableFormula::GetBuffPercent(EConsumableType::FortuneScroll), 0.30f);
	TestEqual(TEXT("Gold feast percent"), FConsumableFormula::GetBuffPercent(EConsumableType::GoldFeast), 0.50f);
	TestEqual(TEXT("Wisdom booster percent"), FConsumableFormula::GetBuffPercent(EConsumableType::WisdomBooster), 0.50f);
	TestEqual(TEXT("All V1 consumables last 30 minutes"), FConsumableFormula::GetBuffDurationSec(EConsumableType::AttackTonic), static_cast<int64>(1800));
	TestEqual(TEXT("Unknown consumable percent is zero"), FConsumableFormula::GetBuffPercent(static_cast<EConsumableType>(99)), 0.0f);
	TestEqual(TEXT("Unknown consumable duration is zero"), FConsumableFormula::GetBuffDurationSec(static_cast<EConsumableType>(99)), static_cast<int64>(0));

	// 2-인자 시그니처는 Standard 등급과 동일해야 합니다(회귀).
	TestEqual(TEXT("Two-arg attack tonic equals standard grade"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic, EConsumableGrade::Standard));

	// 등급별 차등: Lesser = 0.5x, Greater = 2.0x.
	TestEqual(TEXT("Lesser attack tonic is half of standard"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic, EConsumableGrade::Lesser), 0.15f);
	TestEqual(TEXT("Standard attack tonic keeps base percent"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic, EConsumableGrade::Standard), 0.30f);
	TestEqual(TEXT("Greater attack tonic doubles standard"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic, EConsumableGrade::Greater), 0.60f);
	TestEqual(TEXT("Lesser gold feast is half of standard"), FConsumableFormula::GetBuffPercent(EConsumableType::GoldFeast, EConsumableGrade::Lesser), 0.25f);
	TestEqual(TEXT("Greater gold feast doubles standard"), FConsumableFormula::GetBuffPercent(EConsumableType::GoldFeast, EConsumableGrade::Greater), 1.00f);

	// 지속시간은 등급과 무관하게 고정입니다.
	TestEqual(TEXT("Lesser duration matches standard"), FConsumableFormula::GetBuffDurationSec(EConsumableType::AttackTonic, EConsumableGrade::Lesser), static_cast<int64>(1800));
	TestEqual(TEXT("Greater duration matches standard"), FConsumableFormula::GetBuffDurationSec(EConsumableType::AttackTonic, EConsumableGrade::Greater), static_cast<int64>(1800));

	// 잘못된 등급은 효과/지속 0.
	TestEqual(TEXT("Invalid grade percent is zero"), FConsumableFormula::GetBuffPercent(EConsumableType::AttackTonic, static_cast<EConsumableGrade>(99)), 0.0f);
	TestEqual(TEXT("Invalid grade duration is zero"), FConsumableFormula::GetBuffDurationSec(EConsumableType::AttackTonic, static_cast<EConsumableGrade>(99)), static_cast<int64>(0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableGradeBuffServiceTest,
	"IdleProject.Consumable.GradeBuffService",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConsumableGradeBuffServiceTest::RunTest(const FString& Parameters)
{
	UBuffService* Service = NewObject<UBuffService>();
	Service->Initialize();

	// 등급별 수량은 독립적으로 관리됩니다.
	Service->AddConsumable(EConsumableType::AttackTonic, EConsumableGrade::Lesser, 2);
	Service->AddConsumable(EConsumableType::AttackTonic, EConsumableGrade::Greater, 1);
	TestEqual(TEXT("Lesser stock is tracked"), Service->GetCount(EConsumableType::AttackTonic, EConsumableGrade::Lesser), 2);
	TestEqual(TEXT("Greater stock is tracked"), Service->GetCount(EConsumableType::AttackTonic, EConsumableGrade::Greater), 1);
	TestEqual(TEXT("Standard stock stays empty"), Service->GetCount(EConsumableType::AttackTonic, EConsumableGrade::Standard), 0);
	TestEqual(TEXT("Total count aggregates grades"), Service->GetTotalCount(EConsumableType::AttackTonic), 3);

	// Lesser 사용 시 활성 등급 % (0.5x).
	TestTrue(TEXT("Lesser use succeeds"), Service->UseConsumable(EConsumableType::AttackTonic, EConsumableGrade::Lesser, 1000));
	TestEqual(TEXT("Lesser use decrements lesser stock"), Service->GetCount(EConsumableType::AttackTonic, EConsumableGrade::Lesser), 1);
	TestEqual(TEXT("Active grade is Lesser"), static_cast<int32>(Service->GetActiveGrade(EConsumableType::AttackTonic, 1000)), static_cast<int32>(EConsumableGrade::Lesser));
	TestEqual(TEXT("Lesser multiplier reflects 0.5x percent"), Service->GetBuffStatMultiplier(EConsumableType::AttackTonic, 1000), 1.15f);

	// 같은 타입을 Greater 로 재사용 시 최신 등급으로 갱신됩니다(스택=최신).
	TestTrue(TEXT("Greater reuse succeeds"), Service->UseConsumable(EConsumableType::AttackTonic, EConsumableGrade::Greater, 1500));
	TestEqual(TEXT("Active grade updates to Greater"), static_cast<int32>(Service->GetActiveGrade(EConsumableType::AttackTonic, 1500)), static_cast<int32>(EConsumableGrade::Greater));
	TestEqual(TEXT("Greater multiplier reflects 2x percent"), Service->GetBuffStatMultiplier(EConsumableType::AttackTonic, 1500), 1.60f);
	TestEqual(TEXT("Reused buff end resets from latest use"), Service->GetBuffRemainingSec(EConsumableType::AttackTonic, 1500), static_cast<int64>(1800));

	// 경제 getter 도 활성 등급 % 를 사용합니다.
	Service->AddConsumable(EConsumableType::GoldFeast, EConsumableGrade::Greater, 1);
	TestTrue(TEXT("Greater gold feast use succeeds"), Service->UseConsumable(EConsumableType::GoldFeast, EConsumableGrade::Greater, 2000));
	TestEqual(TEXT("Greater gold buff exposes doubled percent"), Service->GetGoldBuffPct(2000), 1.00f);

	// 등급별 수량 + 활성 등급이 세이브를 통해 라운드트립됩니다.
	TArray<FConsumableSaveEntry> Saved = Service->ExportSave();
	UBuffService* Restored = NewObject<UBuffService>();
	Restored->Initialize();
	Restored->ImportSave(Saved);
	TestEqual(TEXT("Lesser stock round trips"), Restored->GetCount(EConsumableType::AttackTonic, EConsumableGrade::Lesser), 1);
	TestEqual(TEXT("Active Greater grade round trips"), static_cast<int32>(Restored->GetActiveGrade(EConsumableType::AttackTonic, 1500)), static_cast<int32>(EConsumableGrade::Greater));
	TestEqual(TEXT("Restored Greater multiplier round trips"), Restored->GetBuffStatMultiplier(EConsumableType::AttackTonic, 1500), 1.60f);
	TestEqual(TEXT("Restored gold buff round trips"), Restored->GetGoldBuffPct(2000), 1.00f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableBuffServiceLifecycleTest,
	"IdleProject.Consumable.BuffServiceLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConsumableBuffServiceLifecycleTest::RunTest(const FString& Parameters)
{
	UBuffService* Service = NewObject<UBuffService>();
	Service->Initialize();

	TestEqual(TEXT("Initial count is zero"), Service->GetCount(EConsumableType::AttackTonic), 0);
	TestFalse(TEXT("Use without stock fails"), Service->UseConsumable(EConsumableType::AttackTonic, 1000));
	Service->AddConsumable(EConsumableType::AttackTonic, 0);
	Service->AddConsumable(EConsumableType::AttackTonic, -3);
	Service->AddConsumable(static_cast<EConsumableType>(99), 5);
	TestEqual(TEXT("Zero, negative, and invalid adds are ignored"), Service->GetCount(EConsumableType::AttackTonic), 0);
	TestFalse(TEXT("Invalid use fails without side effects"), Service->UseConsumable(static_cast<EConsumableType>(99), 1000));

	Service->AddConsumable(EConsumableType::AttackTonic, 2);
	TestEqual(TEXT("AddConsumable increases count"), Service->GetCount(EConsumableType::AttackTonic), 2);
	TestTrue(TEXT("Use with stock succeeds"), Service->UseConsumable(EConsumableType::AttackTonic, 1000));
	TestEqual(TEXT("Use decrements count"), Service->GetCount(EConsumableType::AttackTonic), 1);
	TestTrue(TEXT("Buff is active before end"), Service->IsBuffActive(EConsumableType::AttackTonic, 2799));
	TestEqual(TEXT("Remaining seconds use absolute end time"), Service->GetBuffRemainingSec(EConsumableType::AttackTonic, 1000), static_cast<int64>(1800));
	TestEqual(TEXT("Active stat multiplier is one plus percent"), Service->GetBuffStatMultiplier(EConsumableType::AttackTonic, 1000), 1.30f);
	TestFalse(TEXT("Buff expires at exact end"), Service->IsBuffActive(EConsumableType::AttackTonic, 2800));
	TestEqual(TEXT("Expired stat multiplier is one"), Service->GetBuffStatMultiplier(EConsumableType::AttackTonic, 2800), 1.0f);

	TestTrue(TEXT("Second use extends from now"), Service->UseConsumable(EConsumableType::AttackTonic, 2000));
	TestEqual(TEXT("Reused buff end is now plus duration"), Service->GetBuffRemainingSec(EConsumableType::AttackTonic, 2000), static_cast<int64>(1800));

	Service->AddConsumable(EConsumableType::GuardTonic, 1);
	TestTrue(TEXT("Different buff type can run at the same time"), Service->UseConsumable(EConsumableType::GuardTonic, 2100));
	TestTrue(TEXT("Attack tonic remains active with guard tonic"), Service->IsBuffActive(EConsumableType::AttackTonic, 2100));
	TestTrue(TEXT("Guard tonic is active independently"), Service->IsBuffActive(EConsumableType::GuardTonic, 2100));
	TestEqual(TEXT("Guard multiplier is exposed independently"), Service->GetBuffStatMultiplier(EConsumableType::GuardTonic, 2100), 1.30f);

	Service->AddConsumable(EConsumableType::GoldFeast, 1);
	TestTrue(TEXT("Gold feast can be activated"), Service->UseConsumable(EConsumableType::GoldFeast, 3000));
	TestEqual(TEXT("Gold buff exposes percent while active"), Service->GetGoldBuffPct(3000), 0.50f);
	TestEqual(TEXT("Gold buff expires to zero"), Service->GetGoldBuffPct(4800), 0.0f);

	FConsumableSaveEntry NegativeEntry;
	NegativeEntry.Type = static_cast<uint8>(EConsumableType::WisdomBooster);
	NegativeEntry.Count = -5;
	NegativeEntry.BuffEndUnixSec = -10;
	FConsumableSaveEntry InvalidEntry;
	InvalidEntry.Type = 99;
	InvalidEntry.Count = 3;
	InvalidEntry.BuffEndUnixSec = 5000;
	TArray<FConsumableSaveEntry> ClampEntries;
	ClampEntries.Add(NegativeEntry);
	ClampEntries.Add(InvalidEntry);
	Service->ImportSave(ClampEntries);
	TestEqual(TEXT("Negative imported count clamps to zero"), Service->GetCount(EConsumableType::WisdomBooster), 0);
	TestFalse(TEXT("Negative imported end time is inactive"), Service->IsBuffActive(EConsumableType::WisdomBooster, 0));
	TestEqual(TEXT("Invalid imported type is ignored"), Service->GetCount(static_cast<EConsumableType>(99)), 0);

	Service->Initialize();
	Service->AddConsumable(EConsumableType::AttackTonic, 1);
	TestTrue(TEXT("Attack tonic reactivates after import clamp checks"), Service->UseConsumable(EConsumableType::AttackTonic, 2000));
	Service->AddConsumable(EConsumableType::GoldFeast, 1);
	TestTrue(TEXT("Gold feast reactivates after import clamp checks"), Service->UseConsumable(EConsumableType::GoldFeast, 3000));
	TArray<FConsumableSaveEntry> Saved = Service->ExportSave();
	UBuffService* Restored = NewObject<UBuffService>();
	Restored->Initialize();
	Restored->ImportSave(Saved);
	TestEqual(TEXT("Count round trips"), Restored->GetCount(EConsumableType::AttackTonic), 0);
	TestTrue(TEXT("Active end time round trips"), Restored->IsBuffActive(EConsumableType::AttackTonic, 3799));
	TestEqual(TEXT("Gold buff round trips"), Restored->GetGoldBuffPct(3000), 0.50f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableGameInstanceHooksTest,
	"IdleProject.Consumable.GameInstanceHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConsumableGameInstanceHooksTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world is created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	FWorldContext* Context = AttachConsumableGameInstanceToTestWorld(GameInstance, World);
	TestNotNull(TEXT("Game instance has a world context"), Context);
	AIdleCharacter* Character = SpawnConsumableCharacter(World);
	TestNotNull(TEXT("Idle character is spawned"), Character);
	if (!Context || !Character)
	{
		if (Context)
		{
			GEngine->DestroyWorldContext(World);
		}
		World->DestroyWorld(false);
		return false;
	}

	Character->SetClassId(EClassId::Warrior);
	const FDerivedStats BaseStats = Character->GetCurrentDerivedStats();
	const int64 BasePower = Character->GetCombatPower();

	GameInstance->AddConsumable(EConsumableType::AttackTonic, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("TryUseConsumable consumes stock"), GameInstance->TryUseConsumable(EConsumableType::AttackTonic));
	const FDerivedStats AttackBuffedStats = Character->GetCurrentDerivedStats();
	TestTrue(TEXT("Attack tonic increases physical attack"), AttackBuffedStats.PhysAtk > BaseStats.PhysAtk);
	TestTrue(TEXT("Attack tonic increases magic attack"), AttackBuffedStats.MagicAtk > BaseStats.MagicAtk);
	TestEqual(TEXT("Attack tonic does not buff HP"), AttackBuffedStats.Hp, BaseStats.Hp);
	TestTrue(TEXT("Combat power increases while attack tonic is active"), Character->GetCombatPower() > BasePower);

	GameInstance->AddConsumable(EConsumableType::GuardTonic, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("Guard tonic activates with attack tonic"), GameInstance->TryUseConsumable(EConsumableType::GuardTonic));
	const FDerivedStats GuardBuffedStats = Character->GetCurrentDerivedStats();
	TestTrue(TEXT("Guard tonic increases HP"), GuardBuffedStats.Hp > AttackBuffedStats.Hp);
	TestTrue(TEXT("Guard tonic increases physical defense"), GuardBuffedStats.PhysDef > AttackBuffedStats.PhysDef);
	TestEqual(TEXT("Guard tonic does not double-apply attack"), GuardBuffedStats.PhysAtk, AttackBuffedStats.PhysAtk);

	GameInstance->AddConsumable(EConsumableType::AllStatElixir, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("All stat elixir activates with existing stat buffs"), GameInstance->TryUseConsumable(EConsumableType::AllStatElixir));
	const FDerivedStats AllStatBuffedStats = Character->GetCurrentDerivedStats();
	TestTrue(TEXT("All stat elixir increases attack on top of attack tonic"), AllStatBuffedStats.PhysAtk > GuardBuffedStats.PhysAtk);
	TestTrue(TEXT("All stat elixir increases HP on top of guard tonic"), AllStatBuffedStats.Hp > GuardBuffedStats.Hp);

	GameInstance->AddConsumable(EConsumableType::GoldFeast, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("Gold feast activates"), GameInstance->TryUseConsumable(EConsumableType::GoldFeast));
	GameInstance->AddGold(100);
	TestEqual(TEXT("Gold feast applies once in AddGold"), GameInstance->GetGold(), static_cast<int64>(150));

	GameInstance->AddConsumable(EConsumableType::WisdomBooster, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("Wisdom booster activates"), GameInstance->TryUseConsumable(EConsumableType::WisdomBooster));
	GameInstance->AddExp(99);
	TestEqual(TEXT("Wisdom booster applies once in AddExp"), GameInstance->GetCurrentExp(), static_cast<int64>(149));

	const float BaseDropChance = 0.05f;
	TestEqual(TEXT("Drop chance has no consumable bonus before fortune scroll"), GameInstance->ApplyEquippedPetDropBonusChance(BaseDropChance), BaseDropChance);
	GameInstance->AddConsumable(EConsumableType::FortuneScroll, EConsumableGrade::Standard, 1);
	TestTrue(TEXT("Fortune scroll activates"), GameInstance->TryUseConsumable(EConsumableType::FortuneScroll));
	TestEqual(TEXT("Fortune scroll applies once in drop chance path"), GameInstance->ApplyEquippedPetDropBonusChance(BaseDropChance), BaseDropChance + 0.30f);

	UIdleSaveGame* ExpiredSave = NewObject<UIdleSaveGame>();
	ExpiredSave->bHasSave = true;
	ExpiredSave->SaveVersion = 14;
	FConsumableSaveEntry ExpiredEntry;
	ExpiredEntry.Type = static_cast<uint8>(EConsumableType::AttackTonic);
	ExpiredEntry.Count = 0;
	ExpiredEntry.BuffEndUnixSec = 1;
	ExpiredSave->Consumables.Add(ExpiredEntry);
	TestTrue(TEXT("Expired consumable save applies"), GameInstance->ApplyFromSave(ExpiredSave));
	Character->SetClassId(EClassId::Warrior);
	const FDerivedStats ExpiredStats = Character->GetCurrentDerivedStats();
	TestEqual(TEXT("Expired attack tonic restores physical attack baseline"), ExpiredStats.PhysAtk, BaseStats.PhysAtk);
	TestEqual(TEXT("Expired attack tonic restores combat power baseline"), Character->GetCombatPower(), BasePower);

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableResetPersistenceTest,
	"IdleProject.Consumable.ResetPersistence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FConsumableResetPersistenceTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* Save = NewObject<UIdleSaveGame>();
	Save->bHasSave = true;
	Save->SaveVersion = 14;
	Save->CharacterLevel = 100;
	Save->bChapter1BossDefeated = true;
	FConsumableSaveEntry Entry;
	Entry.Type = static_cast<uint8>(EConsumableType::GuardTonic);
	Entry.Count = 2;
	Entry.BuffEndUnixSec = 12345;
	Save->Consumables.Add(Entry);

	TestTrue(TEXT("Seeded consumable save applies"), GameInstance->ApplyFromSave(Save));
	TestEqual(TEXT("Guard tonic stock restored"), GameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 2);
	TestTrue(TEXT("Guard tonic active timestamp restored"), GameInstance->GetBuffService()->IsBuffActive(EConsumableType::GuardTonic, 1000));
	TestTrue(TEXT("Rebirth succeeds"), GameInstance->Rebirth());
	TestEqual(TEXT("Rebirth keeps consumable stock"), GameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 2);
	TestTrue(TEXT("Rebirth keeps active buff timestamp"), GameInstance->GetBuffService()->IsBuffActive(EConsumableType::GuardTonic, 1000));

	UIdleGameInstance* TranscendGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* TranscendSave = NewObject<UIdleSaveGame>();
	TranscendSave->bHasSave = true;
	TranscendSave->SaveVersion = 14;
	TranscendSave->RebirthCount = 5;
	TranscendSave->Consumables.Add(Entry);
	TestTrue(TEXT("Seeded transcend consumable save applies"), TranscendGameInstance->ApplyFromSave(TranscendSave));
	TestTrue(TEXT("Transcend succeeds"), TranscendGameInstance->Transcend());
	TestEqual(TEXT("Transcend keeps consumable stock"), TranscendGameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 2);
	TestTrue(TEXT("Transcend keeps active buff timestamp"), TranscendGameInstance->GetBuffService()->IsBuffActive(EConsumableType::GuardTonic, 1000));

	UIdleGameInstance* LegacyGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 13;
	LegacySave->Consumables.Add(Entry);
	TestTrue(TEXT("Legacy v13 save applies"), LegacyGameInstance->ApplyFromSave(LegacySave));
	TestEqual(TEXT("v13 consumable inventory migrates to zero"), LegacyGameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 0);
	TestFalse(TEXT("v13 active buff timestamp migrates to inactive"), LegacyGameInstance->GetBuffService()->IsBuffActive(EConsumableType::GuardTonic, 1000));

	UIdleSaveGame* RoundTripSave = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("v16 consumable save captures"), GameInstance->CaptureToSave(RoundTripSave));
	TestEqual(TEXT("Captured save version is v16"), RoundTripSave->SaveVersion, 16);
	TestEqual(TEXT("Captured consumable payload has guard entry"), RoundTripSave->Consumables.Num(), 1);
	TestEqual(TEXT("Captured consumable count persists"), RoundTripSave->Consumables[0].Count, 2);
	TestEqual(TEXT("Captured active end timestamp persists"), RoundTripSave->Consumables[0].BuffEndUnixSec, static_cast<int64>(12345));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FConsumableHudViewModelTest,
	"IdleProject.UI.HUD.ConsumablePanelViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConsumableHudViewModelTest::RunTest(const FString& Parameters)
{
	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	UBuffService* Service = NewObject<UBuffService>();
	Service->Initialize();
	// AttackTonic 은 소/대 두 등급을 보유합니다(등급별 행 노출 검증).
	Service->AddConsumable(EConsumableType::AttackTonic, EConsumableGrade::Lesser, 2);
	Service->AddConsumable(EConsumableType::AttackTonic, EConsumableGrade::Greater, 3);
	Service->AddConsumable(EConsumableType::GoldFeast, EConsumableGrade::Greater, 1);
	TestTrue(TEXT("Greater gold feast activates for HUD"), Service->UseConsumable(EConsumableType::GoldFeast, EConsumableGrade::Greater, 1000));

	const FIdleHUDConsumablePanelViewModel ViewModel = IdleProject::UI::BuildConsumablePanelViewModel(*Service, 1300);
	TestEqual(TEXT("Consumable panel title is localized"), ViewModel.Title.ToString(), FString(TEXT("Consumables")));
	// AttackTonic 2행(소/대) + 무재고 4종 Standard 플레이스홀더 4행 + GoldFeast(대) 1행 = 7행.
	TestEqual(TEXT("Consumable panel exposes a row per owned grade plus placeholders"), ViewModel.Rows.Num(), 7);
	TestEqual(TEXT("Active buff bar exposes one active row"), ViewModel.ActiveBuffRows.Num(), 1);

	const FIdleHUDConsumableRowViewModel& AttackLesserRow = ViewModel.Rows[0];
	TestEqual(TEXT("First row is attack tonic"), static_cast<int32>(AttackLesserRow.Type), static_cast<int32>(EConsumableType::AttackTonic));
	TestEqual(TEXT("First attack row is the Lesser grade"), static_cast<int32>(AttackLesserRow.Grade), static_cast<int32>(EConsumableGrade::Lesser));
	TestEqual(TEXT("Attack lesser name includes grade"), AttackLesserRow.NameLabel.ToString(), FString(TEXT("Attack Tonic (Lesser)")));
	TestEqual(TEXT("Attack lesser grade label is localized"), AttackLesserRow.GradeLabel.ToString(), FString(TEXT("Lesser")));
	TestEqual(TEXT("Attack lesser count is localized"), AttackLesserRow.CountLabel.ToString(), FString(TEXT("Owned 2")));
	TestTrue(TEXT("Attack lesser is usable with stock"), AttackLesserRow.bCanUse);
	TestEqual(TEXT("Attack lesser hitbox encodes type and grade"), AttackLesserRow.UseHitBoxName, FName(TEXT("ConsumableUse_0_0")));
	TestFalse(TEXT("Attack lesser is not active"), AttackLesserRow.bActive);

	const FIdleHUDConsumableRowViewModel& AttackGreaterRow = ViewModel.Rows[1];
	TestEqual(TEXT("Second attack row is the Greater grade"), static_cast<int32>(AttackGreaterRow.Grade), static_cast<int32>(EConsumableGrade::Greater));
	TestEqual(TEXT("Attack greater name includes grade"), AttackGreaterRow.NameLabel.ToString(), FString(TEXT("Attack Tonic (Greater)")));
	TestEqual(TEXT("Attack greater count is localized"), AttackGreaterRow.CountLabel.ToString(), FString(TEXT("Owned 3")));
	TestEqual(TEXT("Attack greater hitbox encodes type and grade"), AttackGreaterRow.UseHitBoxName, FName(TEXT("ConsumableUse_0_2")));

	// 무재고 타입은 Standard 플레이스홀더 1행만 노출하며 사용 불가입니다.
	const FIdleHUDConsumableRowViewModel& GuardRow = ViewModel.Rows[2];
	TestEqual(TEXT("Guard tonic placeholder follows attack rows"), static_cast<int32>(GuardRow.Type), static_cast<int32>(EConsumableType::GuardTonic));
	TestEqual(TEXT("Guard placeholder uses Standard grade"), static_cast<int32>(GuardRow.Grade), static_cast<int32>(EConsumableGrade::Standard));
	TestEqual(TEXT("Guard placeholder count is zero"), GuardRow.CountLabel.ToString(), FString(TEXT("Owned 0")));
	TestFalse(TEXT("Guard placeholder is not usable"), GuardRow.bCanUse);

	// 활성 버프는 사용한 등급(대) 행에만 활성 표시되며 등급을 이름에 표기합니다.
	const FIdleHUDConsumableRowViewModel& ActiveRow = ViewModel.ActiveBuffRows[0];
	TestEqual(TEXT("Active row is the Greater gold feast"), static_cast<int32>(ActiveRow.Type), static_cast<int32>(EConsumableType::GoldFeast));
	TestEqual(TEXT("Active row reflects the active grade"), static_cast<int32>(ActiveRow.Grade), static_cast<int32>(EConsumableGrade::Greater));
	TestEqual(TEXT("Active buff name shows the grade"), ActiveRow.NameLabel.ToString(), FString(TEXT("Gold Feast (Greater)")));
	TestEqual(TEXT("Active buff remaining time is formatted"), ActiveRow.RemainingLabel.ToString(), FString(TEXT("25:00")));
	TestEqual(TEXT("Active buff uses localized effect copy"), ActiveRow.EffectLabel.ToString(), FString(TEXT("Gold +50% for 30m")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

#endif
