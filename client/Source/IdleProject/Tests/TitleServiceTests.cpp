#include "Misc/AutomationTest.h"

#include "IdleGameInstanceTestHelpers.h"
#include "CharacterSystem/CombatPowerFormula.h"
#include "CharacterSystem/IdleCharacter.h"
#include "CharacterSystem/LevelFormulas.h"
#include "CharacterSystem/StatFormulas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameCore/AchievementFormula.h"
#include "GameCore/AchievementService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/TitleService.h"
#include "GameCore/TitleTypes.h"
#include "GameFramework/PlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// 익명 헬퍼는 jumbo ODR 충돌 방지를 위해 Title~ prefix 를 사용한다.

	UAchievementService* TitleMakeAchievementService()
	{
		UAchievementService* Service = NewObject<UAchievementService>();
		Service->InitializeDefaultAchievements();
		return Service;
	}

	UTitleService* TitleMakeService()
	{
		UTitleService* Service = NewObject<UTitleService>();
		Service->InitializeDefaultTitles();
		return Service;
	}

	const FTitleDefinition* TitleFindDefinition(const UTitleService* Service, const FString& TitleId)
	{
		if (!Service)
		{
			return nullptr;
		}
		for (const FTitleDefinition& Definition : Service->GetDefinitions())
		{
			if (Definition.TitleId == TitleId)
			{
				return &Definition;
			}
		}
		return nullptr;
	}

	FWorldContext* TitleAttachGameInstanceToTestWorld(UIdleGameInstance* GameInstance, UWorld* World)
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
	FTitleCatalogParityTest,
	"IdleProject.GameCore.Title.CatalogParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleCatalogParityTest::RunTest(const FString& Parameters)
{
	UTitleService* Service = TitleMakeService();
	TestNotNull(TEXT("Title service is created"), Service);
	if (!Service)
	{
		return false;
	}

	// 서버 title.ts TITLE_CATALOG 18종과 1:1.
	TestEqual(TEXT("Catalog has 18 titles"), Service->GetDefinitions().Num(), 18);

	// id 유니크 보장.
	TSet<FString> SeenIds;
	for (const FTitleDefinition& Definition : Service->GetDefinitions())
	{
		TestFalse(FString::Printf(TEXT("Title id %s is unique"), *Definition.TitleId), SeenIds.Contains(Definition.TitleId));
		SeenIds.Add(Definition.TitleId);
		TestTrue(FString::Printf(TEXT("Title %s has a positive threshold"), *Definition.TitleId), Definition.Threshold > 0);
		TestNotEqual(FString::Printf(TEXT("Title %s has a real bonus type"), *Definition.TitleId), Definition.BonusType, ETitleBonus::None);
		// bonusValue 는 비율(0.03~0.20). 퍼센트 정수가 아님.
		TestTrue(FString::Printf(TEXT("Title %s bonus value is a ratio"), *Definition.TitleId), Definition.BonusValue > 0.0f && Definition.BonusValue <= 0.25f);
	}

	// 대표 앵커 — 서버 카탈로그 임계/보너스값/메트릭/타입 1:1.
	const FTitleDefinition* MonsterHunter = TitleFindDefinition(Service, TEXT("monster_hunter"));
	TestNotNull(TEXT("monster_hunter exists"), MonsterHunter);
	if (MonsterHunter)
	{
		TestEqual(TEXT("monster_hunter metric"), MonsterHunter->Metric, EAchievementMetric::MonstersKilled);
		TestEqual(TEXT("monster_hunter threshold"), MonsterHunter->Threshold, static_cast<int64>(10000));
		TestEqual(TEXT("monster_hunter bonus type"), MonsterHunter->BonusType, ETitleBonus::AllStatPct);
		TestEqual(TEXT("monster_hunter bonus ratio"), MonsterHunter->BonusValue, 0.03f);
	}

	const FTitleDefinition* GoldKing = TitleFindDefinition(Service, TEXT("gold_king"));
	TestNotNull(TEXT("gold_king exists"), GoldKing);
	if (GoldKing)
	{
		TestEqual(TEXT("gold_king metric"), GoldKing->Metric, EAchievementMetric::GoldEarned);
		TestEqual(TEXT("gold_king threshold"), GoldKing->Threshold, static_cast<int64>(1000000000));
		TestEqual(TEXT("gold_king bonus type"), GoldKing->BonusType, ETitleBonus::GoldPct);
		TestEqual(TEXT("gold_king bonus ratio"), GoldKing->BonusValue, 0.2f);
	}

	const FTitleDefinition* BossExecutioner = TitleFindDefinition(Service, TEXT("boss_executioner"));
	TestNotNull(TEXT("boss_executioner exists"), BossExecutioner);
	if (BossExecutioner)
	{
		TestEqual(TEXT("boss_executioner metric"), BossExecutioner->Metric, EAchievementMetric::BossesKilled);
		TestEqual(TEXT("boss_executioner threshold"), BossExecutioner->Threshold, static_cast<int64>(5000));
		TestEqual(TEXT("boss_executioner bonus type"), BossExecutioner->BonusType, ETitleBonus::CritDmgPct);
		TestEqual(TEXT("boss_executioner bonus ratio"), BossExecutioner->BonusValue, 0.18f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTitleUnlockBoundaryTest,
	"IdleProject.GameCore.Title.UnlockBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleUnlockBoundaryTest::RunTest(const FString& Parameters)
{
	UTitleService* Service = TitleMakeService();
	UAchievementService* Achievements = TitleMakeAchievementService();
	TestNotNull(TEXT("Title service created"), Service);
	TestNotNull(TEXT("Achievement service created"), Achievements);
	if (!Service || !Achievements)
	{
		return false;
	}

	// monster_hunter threshold = 10000. threshold-1 에서 미해금.
	Achievements->RecordMetric(EAchievementMetric::MonstersKilled, 9999);
	Service->RecomputeUnlocked(Achievements);
	TestFalse(TEXT("monster_hunter locked at threshold-1"), Service->IsUnlocked(TEXT("monster_hunter")));

	// threshold 도달 시 해금.
	Achievements->RecordMetric(EAchievementMetric::MonstersKilled, 1);
	Service->RecomputeUnlocked(Achievements);
	TestTrue(TEXT("monster_hunter unlocked at threshold"), Service->IsUnlocked(TEXT("monster_hunter")));

	// 다른 메트릭 미달은 미해금 유지.
	TestFalse(TEXT("gold_king still locked"), Service->IsUnlocked(TEXT("gold_king")));

	// 해금은 영구(추가 only) — RecomputeUnlocked 재호출로 사라지지 않는다.
	Service->RecomputeUnlocked(Achievements);
	TestTrue(TEXT("monster_hunter remains unlocked after recompute"), Service->IsUnlocked(TEXT("monster_hunter")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTitleEquipRulesTest,
	"IdleProject.GameCore.Title.EquipRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleEquipRulesTest::RunTest(const FString& Parameters)
{
	UTitleService* Service = TitleMakeService();
	UAchievementService* Achievements = TitleMakeAchievementService();
	if (!Service || !Achievements)
	{
		return false;
	}

	// 미해금 칭호 장착 거부.
	TestFalse(TEXT("Cannot equip locked title"), Service->EquipTitle(TEXT("monster_hunter")));
	TestTrue(TEXT("Equipped id empty after failed equip"), Service->GetEquippedTitleId().IsEmpty());

	// 미정의 id 장착 거부.
	TestFalse(TEXT("Cannot equip unknown title"), Service->EquipTitle(TEXT("nonexistent_title")));

	// 해금 후 장착 성공.
	Achievements->RecordMetric(EAchievementMetric::MonstersKilled, 10000);
	Service->RecomputeUnlocked(Achievements);
	TestTrue(TEXT("Can equip unlocked title"), Service->EquipTitle(TEXT("monster_hunter")));
	TestEqual(TEXT("Equipped id reflects choice"), Service->GetEquippedTitleId(), FString(TEXT("monster_hunter")));

	// 장착은 1개 — 두 번째 칭호 해금 후 장착 시 교체(이전 칭호는 해제).
	Achievements->RecordMetric(EAchievementMetric::TowerHighestFloor, 100);
	Service->RecomputeUnlocked(Achievements);
	TestTrue(TEXT("Can equip second unlocked title"), Service->EquipTitle(TEXT("tower_conqueror")));
	TestEqual(TEXT("Only one title equipped"), Service->GetEquippedTitleId(), FString(TEXT("tower_conqueror")));

	// 해제.
	Service->UnequipTitle();
	TestTrue(TEXT("Unequip clears equipped id"), Service->GetEquippedTitleId().IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTitleEquippedBonusTest,
	"IdleProject.GameCore.Title.EquippedBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleEquippedBonusTest::RunTest(const FString& Parameters)
{
	UTitleService* Service = TitleMakeService();
	UAchievementService* Achievements = TitleMakeAchievementService();
	if (!Service || !Achievements)
	{
		return false;
	}

	// 미장착이면 None/0.
	const FTitleBonus NoBonus = Service->GetEquippedTitleBonus();
	TestEqual(TEXT("No bonus type when nothing equipped"), NoBonus.Type, ETitleBonus::None);
	TestEqual(TEXT("No bonus value when nothing equipped"), NoBonus.Value, 0.0f);

	// gold_king(GoldPct 0.2) 해금/장착 후 정확한 보너스.
	Achievements->RecordMetric(EAchievementMetric::GoldEarned, 1000000000);
	Service->RecomputeUnlocked(Achievements);
	TestTrue(TEXT("Equip gold_king"), Service->EquipTitle(TEXT("gold_king")));
	const FTitleBonus GoldBonus = Service->GetEquippedTitleBonus();
	TestEqual(TEXT("Gold king bonus type"), GoldBonus.Type, ETitleBonus::GoldPct);
	TestEqual(TEXT("Gold king bonus ratio"), GoldBonus.Value, 0.2f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTitleSaveRoundTripTest,
	"IdleProject.GameCore.Title.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleSaveRoundTripTest::RunTest(const FString& Parameters)
{
	// SaveVer 21 라운드트립: 해금 집합 + 장착 칭호가 capture/apply 를 통해 보존된다.
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	// 칭호 해금을 위한 메트릭 적립(업적 경로가 RecomputeUnlockedTitles 를 호출).
	GameInstance->RecordAchievementMetric(EAchievementMetric::TowerHighestFloor, 100);
	UTitleService* TitleService = GameInstance->GetTitleService();
	TestNotNull(TEXT("Title service available"), TitleService);
	if (!TitleService)
	{
		return false;
	}
	TestTrue(TEXT("tower_conqueror unlocked via metric"), TitleService->IsUnlocked(TEXT("tower_conqueror")));
	TestTrue(TEXT("Equip tower_conqueror"), GameInstance->EquipTitle(TEXT("tower_conqueror")));

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Capture writes V23"), SaveGame->SaveVersion, static_cast<int32>(23));
	TestTrue(TEXT("Saved unlocked set contains tower_conqueror"), SaveGame->UnlockedTitleIds.Contains(TEXT("tower_conqueror")));
	TestEqual(TEXT("Saved equipped title id"), SaveGame->EquippedTitleId, FString(TEXT("tower_conqueror")));

	// 새 인스턴스로 복원.
	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply succeeds"), Restored->ApplyFromSave(SaveGame));
	UTitleService* RestoredTitles = Restored->GetTitleService();
	TestNotNull(TEXT("Restored title service"), RestoredTitles);
	if (RestoredTitles)
	{
		TestTrue(TEXT("Restored unlock persists"), RestoredTitles->IsUnlocked(TEXT("tower_conqueror")));
		TestEqual(TEXT("Restored equipped title persists"), RestoredTitles->GetEquippedTitleId(), FString(TEXT("tower_conqueror")));
	}

	// 레거시(<21) 세이브는 칭호 빈 값으로 회귀 안전.
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 20;
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply legacy save succeeds"), LegacyInstance->ApplyFromSave(LegacySave));
	UTitleService* LegacyTitles = LegacyInstance->GetTitleService();
	TestNotNull(TEXT("Legacy title service"), LegacyTitles);
	if (LegacyTitles)
	{
		TestTrue(TEXT("Legacy save has no equipped title"), LegacyTitles->GetEquippedTitleId().IsEmpty());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTitleBonusApplicationE2ETest,
	"IdleProject.GameCore.Title.BonusApplicationE2E",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTitleBonusApplicationE2ETest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("Transient test world created"), World);
	if (!World)
	{
		return false;
	}

	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		World->DestroyWorld(false);
		return false;
	}
	FWorldContext* WorldContext = TitleAttachGameInstanceToTestWorld(GameInstance, World);
	TestNotNull(TEXT("Game instance has a world context"), WorldContext);
	if (!WorldContext)
	{
		World->DestroyWorld(false);
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AIdleCharacter* Character = World->SpawnActor<AIdleCharacter>(AIdleCharacter::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	APlayerController* PlayerController = World->SpawnActor<APlayerController>(APlayerController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	TestNotNull(TEXT("Idle character spawned"), Character);
	TestNotNull(TEXT("Player controller spawned"), PlayerController);
	if (!Character || !PlayerController)
	{
		World->DestroyWorld(false);
		return false;
	}
	World->AddController(PlayerController);
	PlayerController->Possess(Character);
	Character->SetClassId(EClassId::Warrior);
	GameInstance->AddExp(FLevelFormulas::CumulativeExp(50));
	Character->RefreshDerivedStats();

	// AllStatPct: monster_hunter(0.03) 해금/장착 → CP 증가.
	// 주의: MonstersKilled 메트릭 기록은 칭호뿐 아니라 업적(AchievementMultiplier)도 해금하므로,
	// 칭호 단독 효과 비교의 기준 CP는 반드시 메트릭 기록 후(업적 보너스 포함, 칭호 미장착)에 측정한다.
	GameInstance->RecordAchievementMetric(EAchievementMetric::MonstersKilled, 10000);
	Character->RefreshDerivedStats();
	const int64 BaseCombatPower = Character->GetCombatPower();

	TestTrue(TEXT("Equip monster_hunter (AllStatPct)"), GameInstance->EquipTitle(TEXT("monster_hunter")));
	const int64 BoostedCombatPower = Character->GetCombatPower();
	TestTrue(TEXT("AllStat title raises combat power"), BoostedCombatPower > BaseCombatPower);
	TestEqual(TEXT("Combat power stays derived-stat formula"), BoostedCombatPower, FCombatPowerFormula::ComputeCombatPower(Character->GetCurrentDerivedStats()));

	// 해제 시 원복(칭호 이중 적용 없음 — 업적 보너스만 남은 기준으로 복귀).
	GameInstance->UnequipTitle();
	TestEqual(TEXT("Unequip restores base combat power"), Character->GetCombatPower(), BaseCombatPower);

	// GoldPct: tower_conqueror(0.15) 해금/장착 → 골드 획득 1.15배.
	GameInstance->RecordAchievementMetric(EAchievementMetric::TowerHighestFloor, 100);
	TestTrue(TEXT("Equip tower_conqueror (GoldPct)"), GameInstance->EquipTitle(TEXT("tower_conqueror")));
	const int64 GoldBefore = GameInstance->GetGold();
	GameInstance->AddGold(1000);
	TestEqual(TEXT("Gold title applies +15% once"), GameInstance->GetGold() - GoldBefore, static_cast<int64>(1150));

	World->DestroyWorld(false);
	return true;
}

#endif
