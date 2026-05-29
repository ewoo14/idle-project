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

	Service->AddConsumable(EConsumableType::GoldFeast, 1);
	TestTrue(TEXT("Gold feast can be activated"), Service->UseConsumable(EConsumableType::GoldFeast, 3000));
	TestEqual(TEXT("Gold buff exposes percent while active"), Service->GetGoldBuffPct(3000), 0.50f);
	TestEqual(TEXT("Gold buff expires to zero"), Service->GetGoldBuffPct(4800), 0.0f);

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

	GameInstance->AddConsumable(EConsumableType::AttackTonic, 1);
	TestTrue(TEXT("TryUseConsumable consumes stock"), GameInstance->TryUseConsumable(EConsumableType::AttackTonic));
	const FDerivedStats AttackBuffedStats = Character->GetCurrentDerivedStats();
	TestTrue(TEXT("Attack tonic increases physical attack"), AttackBuffedStats.PhysAtk > BaseStats.PhysAtk);
	TestTrue(TEXT("Attack tonic increases magic attack"), AttackBuffedStats.MagicAtk > BaseStats.MagicAtk);
	TestEqual(TEXT("Attack tonic does not buff HP"), AttackBuffedStats.Hp, BaseStats.Hp);
	TestTrue(TEXT("Combat power increases while attack tonic is active"), Character->GetCombatPower() > BasePower);

	GameInstance->AddConsumable(EConsumableType::GoldFeast, 1);
	TestTrue(TEXT("Gold feast activates"), GameInstance->TryUseConsumable(EConsumableType::GoldFeast));
	GameInstance->AddGold(100);
	TestEqual(TEXT("Gold feast applies once in AddGold"), GameInstance->GetGold(), static_cast<int64>(150));

	GameInstance->AddConsumable(EConsumableType::WisdomBooster, 1);
	TestTrue(TEXT("Wisdom booster activates"), GameInstance->TryUseConsumable(EConsumableType::WisdomBooster));
	GameInstance->AddExp(99);
	TestEqual(TEXT("Wisdom booster applies once in AddExp"), GameInstance->GetCurrentExp(), static_cast<int64>(149));

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
	TestTrue(TEXT("Rebirth succeeds"), GameInstance->Rebirth());
	TestEqual(TEXT("Rebirth keeps consumable stock"), GameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 2);

	UIdleGameInstance* TranscendGameInstance = NewObject<UIdleGameInstance>();
	UIdleSaveGame* TranscendSave = NewObject<UIdleSaveGame>();
	TranscendSave->bHasSave = true;
	TranscendSave->SaveVersion = 14;
	TranscendSave->RebirthCount = 5;
	TranscendSave->Consumables.Add(Entry);
	TestTrue(TEXT("Seeded transcend consumable save applies"), TranscendGameInstance->ApplyFromSave(TranscendSave));
	TestTrue(TEXT("Transcend succeeds"), TranscendGameInstance->Transcend());
	TestEqual(TEXT("Transcend keeps consumable stock"), TranscendGameInstance->GetBuffService()->GetCount(EConsumableType::GuardTonic), 2);
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
	Service->AddConsumable(EConsumableType::AttackTonic, 2);
	Service->AddConsumable(EConsumableType::GoldFeast, 1);
	TestTrue(TEXT("Gold feast activates for HUD"), Service->UseConsumable(EConsumableType::GoldFeast, 1000));

	const FIdleHUDConsumablePanelViewModel ViewModel = IdleProject::UI::BuildConsumablePanelViewModel(*Service, 1300);
	TestEqual(TEXT("Consumable panel title is localized"), ViewModel.Title.ToString(), FString(TEXT("Consumables")));
	TestEqual(TEXT("Consumable panel exposes six rows"), ViewModel.Rows.Num(), 6);
	TestEqual(TEXT("Active buff bar exposes one active row"), ViewModel.ActiveBuffRows.Num(), 1);

	const FIdleHUDConsumableRowViewModel& AttackRow = ViewModel.Rows[0];
	TestEqual(TEXT("Attack tonic row keeps enum order"), static_cast<int32>(AttackRow.Type), static_cast<int32>(EConsumableType::AttackTonic));
	TestEqual(TEXT("Attack tonic name is localized"), AttackRow.NameLabel.ToString(), FString(TEXT("Attack Tonic")));
	TestEqual(TEXT("Attack tonic effect is localized"), AttackRow.EffectLabel.ToString(), FString(TEXT("PATK/MATK +30% for 30m")));
	TestEqual(TEXT("Attack tonic count is localized"), AttackRow.CountLabel.ToString(), FString(TEXT("Owned 2")));
	TestEqual(TEXT("Attack tonic use action is localized"), AttackRow.ActionLabel.ToString(), FString(TEXT("Use")));
	TestTrue(TEXT("Attack tonic is usable with stock"), AttackRow.bCanUse);
	TestEqual(TEXT("Attack tonic hitbox is deterministic"), AttackRow.UseHitBoxName, FName(TEXT("ConsumableUse_0")));

	const FIdleHUDConsumableRowViewModel& GoldRow = ViewModel.Rows[4];
	TestTrue(TEXT("Gold feast row shows active state"), GoldRow.bActive);
	TestEqual(TEXT("Gold feast remaining time is formatted"), GoldRow.RemainingLabel.ToString(), FString(TEXT("25:00")));
	TestEqual(TEXT("Active buff bar uses same remaining time"), ViewModel.ActiveBuffRows[0].RemainingLabel.ToString(), FString(TEXT("25:00")));
	TestEqual(TEXT("Active buff bar uses localized effect copy"), ViewModel.ActiveBuffRows[0].EffectLabel.ToString(), FString(TEXT("Gold +50% for 30m")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

#endif
