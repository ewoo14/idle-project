#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/TreasureBoxService.h"
#include "GameCore/TreasureBoxTypes.h"
#include "Math/RandomStream.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
// 익명 헬퍼 — Treasure~ prefix 로 jumbo ODR 충돌 방지.
UTreasureBoxService* TreasureMakeService()
{
	return NewObject<UTreasureBoxService>();
}

// 풀에서 보상 종류의 수량 범위를 조회(테스트 검증용).
bool TreasureFindAmountRange(ETreasureReward Reward, int64& OutMin, int64& OutMax)
{
	for (const FTreasurePoolEntry& Entry : UTreasureBoxService::GetPool())
	{
		if (Entry.Reward == Reward)
		{
			OutMin = Entry.MinAmount;
			OutMax = Entry.MaxAmount;
			return true;
		}
	}
	return false;
}
}

// 서버 treasureBox.ts 1:1 parity(풀 6종 weight/min/max, 누적 가중 합 100).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasurePoolParityTest,
	"IdleProject.TreasureBox.PoolParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasurePoolParityTest::RunTest(const FString& Parameters)
{
	const TArray<FTreasurePoolEntry>& Pool = UTreasureBoxService::GetPool();
	TestEqual(TEXT("Pool has six entries"), Pool.Num(), 6);
	TestEqual(TEXT("Total weight is 100"), UTreasureBoxService::GetTotalWeight(), 100);

	// 순서=누적 가중 구간 순서. 서버 TREASURE_POOL 1:1.
	TestEqual(TEXT("Entry0 gold reward"), static_cast<int32>(Pool[0].Reward), static_cast<int32>(ETreasureReward::Gold));
	TestEqual(TEXT("Entry0 weight 40"), Pool[0].Weight, 40);
	TestEqual(TEXT("Entry0 min 10000"), Pool[0].MinAmount, static_cast<int64>(10000));
	TestEqual(TEXT("Entry0 max 50000"), Pool[0].MaxAmount, static_cast<int64>(50000));

	TestEqual(TEXT("Entry1 essence reward"), static_cast<int32>(Pool[1].Reward), static_cast<int32>(ETreasureReward::Essence));
	TestEqual(TEXT("Entry1 weight 25"), Pool[1].Weight, 25);
	TestEqual(TEXT("Entry1 min 3"), Pool[1].MinAmount, static_cast<int64>(3));
	TestEqual(TEXT("Entry1 max 10"), Pool[1].MaxAmount, static_cast<int64>(10));

	TestEqual(TEXT("Entry2 consumable reward"), static_cast<int32>(Pool[2].Reward), static_cast<int32>(ETreasureReward::Consumable));
	TestEqual(TEXT("Entry2 weight 15"), Pool[2].Weight, 15);
	TestEqual(TEXT("Entry2 min 1"), Pool[2].MinAmount, static_cast<int64>(1));
	TestEqual(TEXT("Entry2 max 2"), Pool[2].MaxAmount, static_cast<int64>(2));

	TestEqual(TEXT("Entry3 protectionScroll reward"), static_cast<int32>(Pool[3].Reward), static_cast<int32>(ETreasureReward::ProtectionScroll));
	TestEqual(TEXT("Entry3 weight 10"), Pool[3].Weight, 10);
	TestEqual(TEXT("Entry3 min 1"), Pool[3].MinAmount, static_cast<int64>(1));
	TestEqual(TEXT("Entry3 max 3"), Pool[3].MaxAmount, static_cast<int64>(3));

	TestEqual(TEXT("Entry4 resetCube reward"), static_cast<int32>(Pool[4].Reward), static_cast<int32>(ETreasureReward::ResetCube));
	TestEqual(TEXT("Entry4 weight 7"), Pool[4].Weight, 7);
	TestEqual(TEXT("Entry4 min 1"), Pool[4].MinAmount, static_cast<int64>(1));
	TestEqual(TEXT("Entry4 max 2"), Pool[4].MaxAmount, static_cast<int64>(2));

	TestEqual(TEXT("Entry5 rankCube reward"), static_cast<int32>(Pool[5].Reward), static_cast<int32>(ETreasureReward::RankCube));
	TestEqual(TEXT("Entry5 weight 3"), Pool[5].Weight, 3);
	TestEqual(TEXT("Entry5 min 1"), Pool[5].MinAmount, static_cast<int64>(1));
	TestEqual(TEXT("Entry5 max 1"), Pool[5].MaxAmount, static_cast<int64>(1));

	return true;
}

// PickReward 누적 가중 경계(서버 pickTreasureReward 1:1). roll 0/39/40/64/65/79/80/89/90/96/97/99 + 클램프.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasurePickRewardBoundaryTest,
	"IdleProject.TreasureBox.PickRewardBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasurePickRewardBoundaryTest::RunTest(const FString& Parameters)
{
	// gold: 0~39
	TestEqual(TEXT("roll 0 gold"), static_cast<int32>(UTreasureBoxService::PickReward(0)), static_cast<int32>(ETreasureReward::Gold));
	TestEqual(TEXT("roll 39 gold"), static_cast<int32>(UTreasureBoxService::PickReward(39)), static_cast<int32>(ETreasureReward::Gold));
	// essence: 40~64
	TestEqual(TEXT("roll 40 essence"), static_cast<int32>(UTreasureBoxService::PickReward(40)), static_cast<int32>(ETreasureReward::Essence));
	TestEqual(TEXT("roll 64 essence"), static_cast<int32>(UTreasureBoxService::PickReward(64)), static_cast<int32>(ETreasureReward::Essence));
	// consumable: 65~79
	TestEqual(TEXT("roll 65 consumable"), static_cast<int32>(UTreasureBoxService::PickReward(65)), static_cast<int32>(ETreasureReward::Consumable));
	TestEqual(TEXT("roll 79 consumable"), static_cast<int32>(UTreasureBoxService::PickReward(79)), static_cast<int32>(ETreasureReward::Consumable));
	// protectionScroll: 80~89
	TestEqual(TEXT("roll 80 protection"), static_cast<int32>(UTreasureBoxService::PickReward(80)), static_cast<int32>(ETreasureReward::ProtectionScroll));
	TestEqual(TEXT("roll 89 protection"), static_cast<int32>(UTreasureBoxService::PickReward(89)), static_cast<int32>(ETreasureReward::ProtectionScroll));
	// resetCube: 90~96
	TestEqual(TEXT("roll 90 resetCube"), static_cast<int32>(UTreasureBoxService::PickReward(90)), static_cast<int32>(ETreasureReward::ResetCube));
	TestEqual(TEXT("roll 96 resetCube"), static_cast<int32>(UTreasureBoxService::PickReward(96)), static_cast<int32>(ETreasureReward::ResetCube));
	// rankCube: 97~99
	TestEqual(TEXT("roll 97 rankCube"), static_cast<int32>(UTreasureBoxService::PickReward(97)), static_cast<int32>(ETreasureReward::RankCube));
	TestEqual(TEXT("roll 99 rankCube"), static_cast<int32>(UTreasureBoxService::PickReward(99)), static_cast<int32>(ETreasureReward::RankCube));

	// 클램프: 음수→gold(0), 초과→rankCube(마지막).
	TestEqual(TEXT("roll -5 clamps to gold"), static_cast<int32>(UTreasureBoxService::PickReward(-5)), static_cast<int32>(ETreasureReward::Gold));
	TestEqual(TEXT("roll 1000 clamps to rankCube"), static_cast<int32>(UTreasureBoxService::PickReward(1000)), static_cast<int32>(ETreasureReward::RankCube));

	// 전 roll(0~99) 커버 — None 미발생.
	for (int32 Roll = 0; Roll < UTreasureBoxService::GetTotalWeight(); ++Roll)
	{
		TestTrue(TEXT("Every roll maps to a real reward"), UTreasureBoxService::PickReward(Roll) != ETreasureReward::None);
	}

	return true;
}

// CanDrawToday 1일 1회·날짜 변경.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasureCanDrawTodayTest,
	"IdleProject.TreasureBox.CanDrawToday",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasureCanDrawTodayTest::RunTest(const FString& Parameters)
{
	UTreasureBoxService* Service = TreasureMakeService();

	// 초기 상태: 오늘 뽑기 가능, 빈 날짜는 불가.
	TestTrue(TEXT("Fresh service can draw today"), Service->CanDrawToday(TEXT("2026-05-30")));
	TestFalse(TEXT("Empty date cannot draw"), Service->CanDrawToday(TEXT("")));

	FRandomStream Rng(1234);
	const FTreasureReward First = Service->DrawTreasure(TEXT("2026-05-30"), Rng);
	TestTrue(TEXT("First draw yields a reward"), First.Reward != ETreasureReward::None);

	// 같은 날짜 재뽑기 불가.
	TestFalse(TEXT("Same day cannot draw again"), Service->CanDrawToday(TEXT("2026-05-30")));
	// 날짜 변경 시 가능.
	TestTrue(TEXT("Next day can draw"), Service->CanDrawToday(TEXT("2026-05-31")));

	return true;
}

// DrawTreasure(보상+수량 범위·TotalDraws++·날짜 갱신·중복 빈 보상). 시드 고정 결정적.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasureDrawTest,
	"IdleProject.TreasureBox.DrawTreasure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasureDrawTest::RunTest(const FString& Parameters)
{
	UTreasureBoxService* Service = TreasureMakeService();
	TestEqual(TEXT("Initial total draws zero"), Service->GetTotalDraws(), static_cast<int64>(0));

	FRandomStream Rng(98765);
	const FTreasureReward Result = Service->DrawTreasure(TEXT("2026-05-30"), Rng);
	TestTrue(TEXT("Draw yields a real reward"), Result.Reward != ETreasureReward::None);

	// 보상 수량이 풀 범위 내.
	int64 Min = 0;
	int64 Max = 0;
	TestTrue(TEXT("Reward range exists"), TreasureFindAmountRange(Result.Reward, Min, Max));
	TestTrue(TEXT("Amount within range"), Result.Amount >= Min && Result.Amount <= Max);

	// 누적 ++ / 날짜 갱신.
	TestEqual(TEXT("Total draws increments"), Service->GetTotalDraws(), static_cast<int64>(1));
	TestEqual(TEXT("Last draw date updated"), Service->GetLastDrawDate(), FString(TEXT("2026-05-30")));

	// 같은 날짜 재뽑기 → 빈 보상, 누적 불변.
	const FTreasureReward Duplicate = Service->DrawTreasure(TEXT("2026-05-30"), Rng);
	TestEqual(TEXT("Duplicate draw is empty"), static_cast<int32>(Duplicate.Reward), static_cast<int32>(ETreasureReward::None));
	TestEqual(TEXT("Total unchanged on duplicate"), Service->GetTotalDraws(), static_cast<int64>(1));

	// 다음 날 → 다시 뽑기 가능, 누적 ++.
	const FTreasureReward NextDay = Service->DrawTreasure(TEXT("2026-05-31"), Rng);
	TestTrue(TEXT("Next-day draw yields reward"), NextDay.Reward != ETreasureReward::None);
	TestEqual(TEXT("Total draws is two"), Service->GetTotalDraws(), static_cast<int64>(2));

	// 결정적: 동일 시드 + 동일 날짜 → 동일 결과.
	UTreasureBoxService* ServiceA = TreasureMakeService();
	UTreasureBoxService* ServiceB = TreasureMakeService();
	FRandomStream RngA(42);
	FRandomStream RngB(42);
	const FTreasureReward ResultA = ServiceA->DrawTreasure(TEXT("2026-06-01"), RngA);
	const FTreasureReward ResultB = ServiceB->DrawTreasure(TEXT("2026-06-01"), RngB);
	TestEqual(TEXT("Same seed same reward"), static_cast<int32>(ResultA.Reward), static_cast<int32>(ResultB.Reward));
	TestEqual(TEXT("Same seed same amount"), ResultA.Amount, ResultB.Amount);

	return true;
}

// GameInstance 보상 단일 지급(각 재화 반영, 시드 고정 결정적) + 중복 빈 보상.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasureRewardGrantTest,
	"IdleProject.GameCore.TreasureBox.RewardGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasureRewardGrantTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UTreasureBoxService* Service = GameInstance->GetTreasureBoxService();
	TestNotNull(TEXT("Treasure service available"), Service);
	if (!Service)
	{
		return false;
	}

	// 시드 고정 결정적 뽑기.
	GameInstance->SeedTreasureBoxRngForTests(7);

	const int64 GoldBefore = GameInstance->GetGold();
	const int64 EssenceBefore = GameInstance->GetRuneEssence();
	const int64 ProtectionBefore = GameInstance->GetProtectionScrolls();
	const int64 ResetBefore = GameInstance->GetResetCubes();
	const int64 RankBefore = GameInstance->GetRankCubes();

	const FTreasureReward Reward = GameInstance->DrawTreasureBox();
	TestTrue(TEXT("Draw yields a real reward"), Reward.Reward != ETreasureReward::None);
	TestTrue(TEXT("Total draws incremented"), Service->GetTotalDraws() >= static_cast<int64>(1));

	// 보상 단일 지급: 해당 재화만 정확히 Amount 만큼 증가, 나머지는 불변(이중 지급 금지).
	switch (Reward.Reward)
	{
	case ETreasureReward::Gold:
		TestEqual(TEXT("Gold granted once"), GameInstance->GetGold(), GoldBefore + Reward.Amount);
		break;
	case ETreasureReward::Essence:
		TestEqual(TEXT("Essence granted once"), GameInstance->GetRuneEssence(), EssenceBefore + Reward.Amount);
		break;
	case ETreasureReward::Consumable:
		// 소비는 BuffService 재고로 들어가므로 재화 카운터는 불변.
		TestEqual(TEXT("Gold unchanged for consumable"), GameInstance->GetGold(), GoldBefore);
		break;
	case ETreasureReward::ProtectionScroll:
		TestEqual(TEXT("Protection granted once"), GameInstance->GetProtectionScrolls(), ProtectionBefore + Reward.Amount);
		break;
	case ETreasureReward::ResetCube:
		TestEqual(TEXT("Reset cube granted once"), GameInstance->GetResetCubes(), ResetBefore + Reward.Amount);
		break;
	case ETreasureReward::RankCube:
		TestEqual(TEXT("Rank cube granted once"), GameInstance->GetRankCubes(), RankBefore + Reward.Amount);
		break;
	default:
		break;
	}

	// 같은 날 재뽑기 → 빈 보상(미지급).
	const int64 GoldAfter = GameInstance->GetGold();
	const FTreasureReward Duplicate = GameInstance->DrawTreasureBox();
	TestEqual(TEXT("Duplicate same-day draw is empty"), static_cast<int32>(Duplicate.Reward), static_cast<int32>(ETreasureReward::None));
	TestEqual(TEXT("No reward on duplicate draw"), GameInstance->GetGold(), GoldAfter);

	return true;
}

// 세이브 v29 라운드트립 + 레거시(<29) 회귀 안전.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTreasureSaveRoundTripTest,
	"IdleProject.GameCore.TreasureBox.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTreasureSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UTreasureBoxService* Service = GameInstance->GetTreasureBoxService();
	TestNotNull(TEXT("Treasure service available"), Service);
	if (!Service)
	{
		return false;
	}

	// 뽑기 1회 → 누적/날짜 세팅.
	GameInstance->SeedTreasureBoxRngForTests(3);
	const FTreasureReward Reward = GameInstance->DrawTreasureBox();
	TestTrue(TEXT("Draw succeeds"), Reward.Reward != ETreasureReward::None);

	const int64 SavedTotal = Service->GetTotalDraws();
	const FString SavedDate = Service->GetLastDrawDate();

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Capture writes V29"), SaveGame->SaveVersion, static_cast<int32>(29));
	TestEqual(TEXT("Saved total treasure draws"), SaveGame->TotalTreasureDraws, SavedTotal);
	TestEqual(TEXT("Saved last treasure draw date"), SaveGame->LastTreasureDrawDate, SavedDate);

	// 새 인스턴스 복원.
	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply succeeds"), Restored->ApplyFromSave(SaveGame));
	UTreasureBoxService* RestoredService = Restored->GetTreasureBoxService();
	TestNotNull(TEXT("Restored treasure service"), RestoredService);
	if (RestoredService)
	{
		TestEqual(TEXT("Restored total persists"), RestoredService->GetTotalDraws(), SavedTotal);
		TestEqual(TEXT("Restored date persists"), RestoredService->GetLastDrawDate(), SavedDate);
		// 복원 후 같은 날짜 재뽑기 거부(1일 1회 유지).
		TestFalse(TEXT("Restored cannot redraw same day"), RestoredService->CanDrawToday(SavedDate));
	}

	// 레거시(<25) 세이브는 보물 상자 빈 값으로 회귀 안전.
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 24;
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply legacy save succeeds"), LegacyInstance->ApplyFromSave(LegacySave));
	UTreasureBoxService* LegacyService = LegacyInstance->GetTreasureBoxService();
	TestNotNull(TEXT("Legacy treasure service"), LegacyService);
	if (LegacyService)
	{
		TestEqual(TEXT("Legacy total starts at zero"), LegacyService->GetTotalDraws(), static_cast<int64>(0));
		TestTrue(TEXT("Legacy last date empty"), LegacyService->GetLastDrawDate().IsEmpty());
	}

	return true;
}

#endif
