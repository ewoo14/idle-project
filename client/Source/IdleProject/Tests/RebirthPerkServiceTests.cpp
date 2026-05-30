#include "Misc/AutomationTest.h"

#include "CharacterSystem/LevelFormulas.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/RebirthPerkService.h"
#include "GameCore/RebirthPerkTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
// 익명 헬퍼 — RebirthPerk~ prefix 로 jumbo ODR 충돌 방지.
URebirthPerkService* RebirthPerkMakeService()
{
	return NewObject<URebirthPerkService>();
}

// 환생 1회 수행(레벨 100 + 챕터1 보스 격파 후 Rebirth). 실패 시 false.
bool RebirthPerkPerformRebirths(UIdleGameInstance& GameInstance, int32 Count)
{
	for (int32 Index = 0; Index < Count; ++Index)
	{
		GameInstance.AddExp(FLevelFormulas::CumulativeExp(100));
		GameInstance.MarkChapter1BossDefeated();
		if (!GameInstance.Rebirth())
		{
			return false;
		}
	}
	return true;
}
}

// 서버 rebirthPerk.ts 1:1 parity — PerkStep 6종 / getTotalRebirthPerkPoints / getPerkBonus(fround).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkParityTest,
	"IdleProject.RebirthPerk.FormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkParityTest::RunTest(const FString& Parameters)
{
	// PerkStep 6종 앵커(서버 PERK_STEP 와 동일).
	TestEqual(TEXT("GoldPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::GoldPct), 0.02f);
	TestEqual(TEXT("DropPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::DropPct), 0.02f);
	TestEqual(TEXT("CritDmgPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::CritDmgPct), 0.03f);
	TestEqual(TEXT("AllStatPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::AllStatPct), 0.01f);
	TestEqual(TEXT("ExpPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::ExpPct), 0.02f);
	TestEqual(TEXT("OfflineEffPct step"), URebirthPerkService::GetPerkStep(ERebirthPerk::OfflineEffPct), 0.03f);
	TestEqual(TEXT("None step is zero"), URebirthPerkService::GetPerkStep(ERebirthPerk::None), 0.0f);

	// getTotalRebirthPerkPoints: max(0, rebirthCount). 1/환생, 음수 0 가드.
	TestEqual(TEXT("Total at rebirth 0"), URebirthPerkService::GetTotalRebirthPerkPoints(0), 0);
	TestEqual(TEXT("Total at rebirth 7"), URebirthPerkService::GetTotalRebirthPerkPoints(7), 7);
	TestEqual(TEXT("Total negative clamps to zero"), URebirthPerkService::GetTotalRebirthPerkPoints(-3), 0);

	// getPerkBonus: fround(step * max(0, level)). level0=0, 선형, 음수 0 가드.
	// f32 곱셈 결과(서버 Math.fround 와 동일 차원). 직접 float 산술로 앵커.
	TestEqual(TEXT("Bonus level 0 is zero"), URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk::GoldPct, 0), 0.0f);
	TestEqual(TEXT("GoldPct level 3"), URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk::GoldPct, 3), 0.02f * 3.0f);
	TestEqual(TEXT("CritDmgPct level 5"), URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk::CritDmgPct, 5), 0.03f * 5.0f);
	TestEqual(TEXT("AllStatPct level 10"), URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk::AllStatPct, 10), 0.01f * 10.0f);
	TestEqual(TEXT("Negative level clamps to zero"), URebirthPerkService::GetPerkBonusForLevel(ERebirthPerk::ExpPct, -4), 0.0f);

	return true;
}

// 분배/해제/리셋 + 가용 한도 + None 거부 + 중복 분배.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkAllocationTest,
	"IdleProject.RebirthPerk.Allocation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkAllocationTest::RunTest(const FString& Parameters)
{
	URebirthPerkService* Service = RebirthPerkMakeService();
	TestNotNull(TEXT("Service available"), Service);
	if (!Service)
	{
		return false;
	}

	// 총 0 이면 어떤 분배도 실패(가용 0).
	TestEqual(TEXT("Spent zero initially"), Service->GetSpent(), 0);
	TestEqual(TEXT("Available zero initially"), Service->GetAvailable(), 0);
	TestFalse(TEXT("Allocate fails with no points"), Service->AllocatePerk(ERebirthPerk::GoldPct));

	Service->SetTotalPoints(3);
	TestEqual(TEXT("Total set to three"), Service->GetTotalPoints(), 3);
	TestEqual(TEXT("Available three"), Service->GetAvailable(), 3);

	// None 거부.
	TestFalse(TEXT("Allocate None rejected"), Service->AllocatePerk(ERebirthPerk::None));

	// 분배 + 중복.
	TestTrue(TEXT("Allocate GoldPct once"), Service->AllocatePerk(ERebirthPerk::GoldPct));
	TestTrue(TEXT("Allocate GoldPct twice"), Service->AllocatePerk(ERebirthPerk::GoldPct));
	TestTrue(TEXT("Allocate ExpPct once"), Service->AllocatePerk(ERebirthPerk::ExpPct));
	TestEqual(TEXT("GoldPct level two"), Service->GetPerkLevel(ERebirthPerk::GoldPct), 2);
	TestEqual(TEXT("ExpPct level one"), Service->GetPerkLevel(ERebirthPerk::ExpPct), 1);
	TestEqual(TEXT("Spent three"), Service->GetSpent(), 3);
	TestEqual(TEXT("Available zero"), Service->GetAvailable(), 0);

	// 가용 한도 초과 거부.
	TestFalse(TEXT("Allocate over available rejected"), Service->AllocatePerk(ERebirthPerk::DropPct));
	TestEqual(TEXT("Spent unchanged after rejected allocate"), Service->GetSpent(), 3);

	// 해제(-1) + None 거부 + 미보유 거부.
	TestFalse(TEXT("Deallocate None rejected"), Service->DeallocatePerk(ERebirthPerk::None));
	TestFalse(TEXT("Deallocate unallocated rejected"), Service->DeallocatePerk(ERebirthPerk::DropPct));
	TestTrue(TEXT("Deallocate GoldPct"), Service->DeallocatePerk(ERebirthPerk::GoldPct));
	TestEqual(TEXT("GoldPct level one after deallocate"), Service->GetPerkLevel(ERebirthPerk::GoldPct), 1);
	TestEqual(TEXT("Available one after deallocate"), Service->GetAvailable(), 1);

	// 레벨 0 도달 시 제거되어 재해제 거부.
	TestTrue(TEXT("Deallocate ExpPct to zero"), Service->DeallocatePerk(ERebirthPerk::ExpPct));
	TestEqual(TEXT("ExpPct level zero"), Service->GetPerkLevel(ERebirthPerk::ExpPct), 0);
	TestFalse(TEXT("Deallocate ExpPct again rejected"), Service->DeallocatePerk(ERebirthPerk::ExpPct));

	// 리셋 — 전부 회수.
	Service->ResetPerks();
	TestEqual(TEXT("Spent zero after reset"), Service->GetSpent(), 0);
	TestEqual(TEXT("Available restored to total after reset"), Service->GetAvailable(), 3);

	// 총 포인트 축소 시 가용은 0 클램프하되 분배는 유지(비파괴).
	TestTrue(TEXT("Re-allocate after reset"), Service->AllocatePerk(ERebirthPerk::CritDmgPct));
	TestTrue(TEXT("Re-allocate again"), Service->AllocatePerk(ERebirthPerk::CritDmgPct));
	Service->SetTotalPoints(1);
	TestEqual(TEXT("Spent persists when total shrinks"), Service->GetSpent(), 2);
	TestEqual(TEXT("Available clamps to zero when overspent"), Service->GetAvailable(), 0);

	return true;
}

// GameInstance: 환생 횟수 → 총 포인트 동기화, 분배 진입점.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkGameInstanceTotalTest,
	"IdleProject.RebirthPerk.GameInstanceTotal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkGameInstanceTotalTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	URebirthPerkService* Service = GameInstance->GetRebirthPerkService();
	TestNotNull(TEXT("Perk service lazy-ensured"), Service);
	if (!Service)
	{
		return false;
	}

	// 신규: 환생 0 → 총 0.
	TestEqual(TEXT("Total zero at rebirth zero"), Service->GetTotalPoints(), 0);
	TestFalse(TEXT("Allocate fails with no points"), GameInstance->AllocateRebirthPerk(ERebirthPerk::GoldPct));

	// 3회 환생 → 총 3.
	TestTrue(TEXT("Three rebirths"), RebirthPerkPerformRebirths(*GameInstance, 3));
	TestEqual(TEXT("Rebirth count three"), GameInstance->GetRebirthCount(), 3);
	TestEqual(TEXT("Total points sync to rebirth count"), Service->GetTotalPoints(), 3);
	TestEqual(TEXT("Available three after rebirths"), Service->GetAvailable(), 3);

	// GameInstance 진입점으로 분배.
	TestTrue(TEXT("Allocate via game instance"), GameInstance->AllocateRebirthPerk(ERebirthPerk::AllStatPct));
	TestEqual(TEXT("AllStat level one"), Service->GetPerkLevel(ERebirthPerk::AllStatPct), 1);
	TestTrue(TEXT("Deallocate via game instance"), GameInstance->DeallocateRebirthPerk(ERebirthPerk::AllStatPct));
	TestEqual(TEXT("AllStat level zero after deallocate"), Service->GetPerkLevel(ERebirthPerk::AllStatPct), 0);

	// 리셋 진입점.
	TestTrue(TEXT("Re-allocate before reset"), GameInstance->AllocateRebirthPerk(ERebirthPerk::ExpPct));
	GameInstance->ResetRebirthPerks();
	TestEqual(TEXT("Spent zero after reset entry"), Service->GetSpent(), 0);

	return true;
}

// 보너스 적용(GameInstance 경유) — Gold 골드 배수 반영.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkGoldBonusTest,
	"IdleProject.RebirthPerk.GoldBonusApplied",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkGoldBonusTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	// 환생 없이 분배 불가 → 기준 골드 배수 1.0.
	GameInstance->AddGold(1000);
	const int64 BaselineGold = GameInstance->GetGold();
	TestEqual(TEXT("Baseline gold no bonus"), BaselineGold, static_cast<int64>(1000));

	// 환생 5회 → 총 5 포인트. GoldPct 5 레벨 분배(+10%).
	TestTrue(TEXT("Five rebirths"), RebirthPerkPerformRebirths(*GameInstance, 5));
	URebirthPerkService* Service = GameInstance->GetRebirthPerkService();
	TestNotNull(TEXT("Perk service"), Service);
	if (!Service)
	{
		return false;
	}
	for (int32 Index = 0; Index < 5; ++Index)
	{
		TestTrue(TEXT("Allocate GoldPct"), GameInstance->AllocateRebirthPerk(ERebirthPerk::GoldPct));
	}
	TestEqual(TEXT("GoldPct level five"), Service->GetPerkLevel(ERebirthPerk::GoldPct), 5);

	// 환생은 골드를 10% 로 깎으므로 현재 보유분 리셋 후 신규 적립으로 배수 검증.
	const int64 GoldBefore = GameInstance->GetGold();
	GameInstance->AddGold(1000);
	const int64 Delta = GameInstance->GetGold() - GoldBefore;
	// +10% 배수 → 1100.
	TestEqual(TEXT("Gold gain scaled by rebirth perk +10 percent"), Delta, static_cast<int64>(1100));

	return true;
}

// 세이브 v24 라운드트립 + 레거시(<24) 회귀 안전.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkSaveRoundTripTest,
	"IdleProject.RebirthPerk.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestTrue(TEXT("Four rebirths"), RebirthPerkPerformRebirths(*GameInstance, 4));
	TestTrue(TEXT("Allocate DropPct"), GameInstance->AllocateRebirthPerk(ERebirthPerk::DropPct));
	TestTrue(TEXT("Allocate DropPct again"), GameInstance->AllocateRebirthPerk(ERebirthPerk::DropPct));
	TestTrue(TEXT("Allocate CritDmgPct"), GameInstance->AllocateRebirthPerk(ERebirthPerk::CritDmgPct));

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Capture writes V25"), SaveGame->SaveVersion, static_cast<int32>(25));
	TestTrue(TEXT("Saved allocations contain DropPct"), SaveGame->RebirthPerkAllocations.Contains(ERebirthPerk::DropPct));
	TestEqual(TEXT("Saved DropPct level"), SaveGame->RebirthPerkAllocations.FindRef(ERebirthPerk::DropPct), 2);

	// 새 인스턴스 복원.
	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply succeeds"), Restored->ApplyFromSave(SaveGame));
	URebirthPerkService* RestoredService = Restored->GetRebirthPerkService();
	TestNotNull(TEXT("Restored perk service"), RestoredService);
	if (RestoredService)
	{
		TestEqual(TEXT("Restored rebirth count four"), Restored->GetRebirthCount(), 4);
		TestEqual(TEXT("Restored total points sync"), RestoredService->GetTotalPoints(), 4);
		TestEqual(TEXT("Restored DropPct level"), RestoredService->GetPerkLevel(ERebirthPerk::DropPct), 2);
		TestEqual(TEXT("Restored CritDmgPct level"), RestoredService->GetPerkLevel(ERebirthPerk::CritDmgPct), 1);
		TestEqual(TEXT("Restored spent three"), RestoredService->GetSpent(), 3);
		TestEqual(TEXT("Restored available one"), RestoredService->GetAvailable(), 1);
	}

	// 레거시(<24) 세이브는 환생 특성 빈 분배로 회귀 안전.
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 23;
	LegacySave->RebirthCount = 6;
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply legacy save succeeds"), LegacyInstance->ApplyFromSave(LegacySave));
	URebirthPerkService* LegacyService = LegacyInstance->GetRebirthPerkService();
	TestNotNull(TEXT("Legacy perk service"), LegacyService);
	if (LegacyService)
	{
		// 분배는 비었지만 총 포인트는 환생 횟수에서 파생.
		TestEqual(TEXT("Legacy spent zero"), LegacyService->GetSpent(), 0);
		TestEqual(TEXT("Legacy total derived from rebirth count"), LegacyService->GetTotalPoints(), 6);
		TestEqual(TEXT("Legacy available six"), LegacyService->GetAvailable(), 6);
	}

	return true;
}

// 초월 시 환생 특성 분배·총 포인트 0 으로 리셋.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthPerkTranscendResetTest,
	"IdleProject.RebirthPerk.TranscendReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthPerkTranscendResetTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	// 5회 환생 → 초월 가능. 일부 분배 후 초월.
	TestTrue(TEXT("Five rebirths"), RebirthPerkPerformRebirths(*GameInstance, 5));
	TestTrue(TEXT("Allocate before transcend"), GameInstance->AllocateRebirthPerk(ERebirthPerk::GoldPct));
	TestTrue(TEXT("Can transcend"), GameInstance->CanTranscend());
	TestTrue(TEXT("Transcend"), GameInstance->Transcend());

	URebirthPerkService* Service = GameInstance->GetRebirthPerkService();
	TestNotNull(TEXT("Perk service"), Service);
	if (Service)
	{
		TestEqual(TEXT("Total zero after transcend"), Service->GetTotalPoints(), 0);
		TestEqual(TEXT("Spent zero after transcend"), Service->GetSpent(), 0);
		TestEqual(TEXT("GoldPct cleared after transcend"), Service->GetPerkLevel(ERebirthPerk::GoldPct), 0);
	}

	return true;
}

#endif
