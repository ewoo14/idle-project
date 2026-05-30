#include "Misc/AutomationTest.h"

#include "GameCore/AttendanceService.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
// 익명 헬퍼 — Attendance~ prefix 로 jumbo ODR 충돌 방지.
UAttendanceService* AttendanceMakeService()
{
	return NewObject<UAttendanceService>();
}
}

// 서버 attendance.ts 1:1 parity(임계 f32 n1~8 / 보상 3주기 앵커).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceFormulaParityTest,
	"IdleProject.Attendance.FormulaParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceFormulaParityTest::RunTest(const FString& Parameters)
{
	// 임계: floor(BASE * fround(GROWTH^(n-1))) — 서버 getAttendanceMilestoneThreshold 와 동일(f32).
	TestEqual(TEXT("Threshold n0 is zero"), UAttendanceService::GetMilestoneThreshold(0), static_cast<int64>(0));
	TestEqual(TEXT("Threshold n1"), UAttendanceService::GetMilestoneThreshold(1), static_cast<int64>(2));
	TestEqual(TEXT("Threshold n2"), UAttendanceService::GetMilestoneThreshold(2), static_cast<int64>(3));
	TestEqual(TEXT("Threshold n3"), UAttendanceService::GetMilestoneThreshold(3), static_cast<int64>(5));
	TestEqual(TEXT("Threshold n4"), UAttendanceService::GetMilestoneThreshold(4), static_cast<int64>(8));
	TestEqual(TEXT("Threshold n5"), UAttendanceService::GetMilestoneThreshold(5), static_cast<int64>(13));
	TestEqual(TEXT("Threshold n6"), UAttendanceService::GetMilestoneThreshold(6), static_cast<int64>(20));
	TestEqual(TEXT("Threshold n7"), UAttendanceService::GetMilestoneThreshold(7), static_cast<int64>(33));
	TestEqual(TEXT("Threshold n8"), UAttendanceService::GetMilestoneThreshold(8), static_cast<int64>(53));

	// 임계 단조 증가.
	for (int32 N = 2; N <= 8; ++N)
	{
		TestTrue(TEXT("Threshold strictly increases"),
			UAttendanceService::GetMilestoneThreshold(N) > UAttendanceService::GetMilestoneThreshold(N - 1));
	}

	// 보상 3주기 앵커: 1=gold(기하), 2=essence(비례), 0=consumable(비례).
	EAttendanceReward Type = EAttendanceReward::Gold;
	int64 Value = 0;

	UAttendanceService::GetMilestoneReward(0, Type, Value);
	TestEqual(TEXT("Reward n0 gold zero"), Value, static_cast<int64>(0));

	UAttendanceService::GetMilestoneReward(1, Type, Value);
	TestEqual(TEXT("Reward n1 type gold"), static_cast<int32>(Type), static_cast<int32>(EAttendanceReward::Gold));
	TestEqual(TEXT("Reward n1 gold value"), Value, static_cast<int64>(10000));

	UAttendanceService::GetMilestoneReward(2, Type, Value);
	TestEqual(TEXT("Reward n2 type essence"), static_cast<int32>(Type), static_cast<int32>(EAttendanceReward::Essence));
	TestEqual(TEXT("Reward n2 essence value"), Value, static_cast<int64>(10));

	UAttendanceService::GetMilestoneReward(3, Type, Value);
	TestEqual(TEXT("Reward n3 type consumable"), static_cast<int32>(Type), static_cast<int32>(EAttendanceReward::Consumable));
	TestEqual(TEXT("Reward n3 consumable value"), Value, static_cast<int64>(3));

	UAttendanceService::GetMilestoneReward(4, Type, Value);
	TestEqual(TEXT("Reward n4 type gold"), static_cast<int32>(Type), static_cast<int32>(EAttendanceReward::Gold));
	TestEqual(TEXT("Reward n4 gold value"), Value, static_cast<int64>(33750));

	UAttendanceService::GetMilestoneReward(5, Type, Value);
	TestEqual(TEXT("Reward n5 essence value"), Value, static_cast<int64>(25));

	UAttendanceService::GetMilestoneReward(6, Type, Value);
	TestEqual(TEXT("Reward n6 consumable value"), Value, static_cast<int64>(6));

	UAttendanceService::GetMilestoneReward(7, Type, Value);
	TestEqual(TEXT("Reward n7 type gold"), static_cast<int32>(Type), static_cast<int32>(EAttendanceReward::Gold));
	TestEqual(TEXT("Reward n7 gold value"), Value, static_cast<int64>(113906));

	UAttendanceService::GetMilestoneReward(8, Type, Value);
	TestEqual(TEXT("Reward n8 essence value"), Value, static_cast<int64>(40));

	return true;
}

// 체크인 1일 1회·중복 false·날짜 변경 시 ++.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceCheckInTest,
	"IdleProject.Attendance.CheckIn",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceCheckInTest::RunTest(const FString& Parameters)
{
	UAttendanceService* Service = AttendanceMakeService();
	TestEqual(TEXT("Initial total is zero"), Service->GetTotalAttendance(), static_cast<int64>(0));

	TestTrue(TEXT("First check-in succeeds"), Service->CheckIn(TEXT("2026-05-30")));
	TestEqual(TEXT("Total increments to one"), Service->GetTotalAttendance(), static_cast<int64>(1));

	// 같은 날짜 중복 체크인은 false, 누적 불변.
	TestFalse(TEXT("Same-day check-in fails"), Service->CheckIn(TEXT("2026-05-30")));
	TestEqual(TEXT("Total unchanged on duplicate"), Service->GetTotalAttendance(), static_cast<int64>(1));

	// 빈 날짜는 무시.
	TestFalse(TEXT("Empty date check-in fails"), Service->CheckIn(TEXT("")));
	TestEqual(TEXT("Total unchanged on empty"), Service->GetTotalAttendance(), static_cast<int64>(1));

	// 날짜 변경 시 ++.
	TestTrue(TEXT("Next-day check-in succeeds"), Service->CheckIn(TEXT("2026-05-31")));
	TestEqual(TEXT("Total increments to two"), Service->GetTotalAttendance(), static_cast<int64>(2));
	TestEqual(TEXT("Last date updated"), Service->GetLastAttendanceDate(), FString(TEXT("2026-05-31")));

	return true;
}

// GetReachedMilestones 경계(threshold-1 / threshold), total=34 → 7.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceReachedMilestonesTest,
	"IdleProject.Attendance.ReachedMilestones",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceReachedMilestonesTest::RunTest(const FString& Parameters)
{
	UAttendanceService* Service = AttendanceMakeService();

	// total=0 → 0(첫 임계 2 미달).
	TestEqual(TEXT("Zero attendance reaches none"), Service->GetReachedMilestones(), 0);

	// threshold-1(=1) → 0, threshold(=2) → 1.
	Service->RestoreState(1, TEXT(""), TSet<int32>());
	TestEqual(TEXT("Below first threshold reaches none"), Service->GetReachedMilestones(), 0);

	Service->RestoreState(2, TEXT(""), TSet<int32>());
	TestEqual(TEXT("Exact first threshold reaches one"), Service->GetReachedMilestones(), 1);

	// n7 임계 33, n8 임계 53. total=34 → 7.
	Service->RestoreState(32, TEXT(""), TSet<int32>());
	TestEqual(TEXT("Just below n7 reaches six"), Service->GetReachedMilestones(), 6);

	Service->RestoreState(33, TEXT(""), TSet<int32>());
	TestEqual(TEXT("Exact n7 threshold reaches seven"), Service->GetReachedMilestones(), 7);

	Service->RestoreState(34, TEXT(""), TSet<int32>());
	TestEqual(TEXT("Total 34 reaches seven"), Service->GetReachedMilestones(), 7);

	return true;
}

// ClaimMilestone(도달 수령·미달 false·중복 false).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceClaimMilestoneTest,
	"IdleProject.Attendance.ClaimMilestone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceClaimMilestoneTest::RunTest(const FString& Parameters)
{
	UAttendanceService* Service = AttendanceMakeService();
	Service->RestoreState(13, TEXT(""), TSet<int32>()); // n5 임계 13 → reached 5.
	TestEqual(TEXT("Total 13 reaches five"), Service->GetReachedMilestones(), 5);

	// 미달 마일스톤 수령 거부.
	TestFalse(TEXT("Unreached milestone cannot be claimed"), Service->ClaimMilestone(6));
	// 무효 N 거부.
	TestFalse(TEXT("Zero milestone cannot be claimed"), Service->ClaimMilestone(0));

	// 도달 마일스톤 수령.
	TestTrue(TEXT("Reached milestone can be claimed"), Service->ClaimMilestone(3));
	TestTrue(TEXT("Claimed milestone is marked"), Service->IsMilestoneClaimed(3));
	// 중복 수령 거부.
	TestFalse(TEXT("Duplicate claim rejected"), Service->ClaimMilestone(3));

	// 비순차 수령 허용(단건 수령). 5 도달분 수령.
	TestTrue(TEXT("Higher reached milestone can be claimed"), Service->ClaimMilestone(5));
	TestTrue(TEXT("Both claims tracked"), Service->IsMilestoneClaimed(3) && Service->IsMilestoneClaimed(5));
	TestFalse(TEXT("Unclaimed mid milestone not marked"), Service->IsMilestoneClaimed(4));

	return true;
}

// GameInstance 보상 지급(gold/essence) + 미달/중복 거부.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceRewardGrantTest,
	"IdleProject.GameCore.Attendance.RewardGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceRewardGrantTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UAttendanceService* Service = GameInstance->GetAttendanceService();
	TestNotNull(TEXT("Attendance service available"), Service);
	if (!Service)
	{
		return false;
	}

	// 누적 13 → reached 5(n1 gold 10000, n2 essence 10).
	GameInstance->SeedAttendanceForTests(13);
	TestEqual(TEXT("Seeded reaches five"), Service->GetReachedMilestones(), 5);

	const int64 GoldBefore = GameInstance->GetGold();
	const int64 EssenceBefore = GameInstance->GetRuneEssence();

	// n1 gold 10000 단일 지급.
	TestTrue(TEXT("Gold milestone claim succeeds"), GameInstance->ClaimAttendanceMilestone(1));
	TestEqual(TEXT("Gold reward granted once"), GameInstance->GetGold(), GoldBefore + 10000);

	// 중복 수령 거부(추가 지급 없음).
	TestFalse(TEXT("Duplicate gold claim rejected"), GameInstance->ClaimAttendanceMilestone(1));
	TestEqual(TEXT("No double gold reward"), GameInstance->GetGold(), GoldBefore + 10000);

	// n2 essence 10 지급.
	TestTrue(TEXT("Essence milestone claim succeeds"), GameInstance->ClaimAttendanceMilestone(2));
	TestEqual(TEXT("Essence reward granted"), GameInstance->GetRuneEssence(), EssenceBefore + 10);

	// 미달 마일스톤(n6 임계 20 > 13) 수령 거부.
	TestFalse(TEXT("Unreached milestone rejected"), GameInstance->ClaimAttendanceMilestone(6));

	return true;
}

// 세이브 v23 라운드트립 + 레거시(<23) 회귀 안전.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttendanceSaveRoundTripTest,
	"IdleProject.GameCore.Attendance.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttendanceSaveRoundTripTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UAttendanceService* Service = GameInstance->GetAttendanceService();
	TestNotNull(TEXT("Attendance service available"), Service);
	if (!Service)
	{
		return false;
	}

	// 누적 13 + 마일스톤 3 수령.
	GameInstance->SeedAttendanceForTests(13);
	TestTrue(TEXT("Claim milestone three"), GameInstance->ClaimAttendanceMilestone(3));

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Capture writes V23"), SaveGame->SaveVersion, static_cast<int32>(23));
	// 세이브 시점 CheckIn 으로 누적이 14 로 증가(오늘 첫 출석).
	TestTrue(TEXT("Saved attendance total at least seeded"), SaveGame->AttendanceTotal >= static_cast<int64>(13));
	TestTrue(TEXT("Saved claimed contains milestone three"), SaveGame->AttendanceClaimedMilestones.Contains(3));

	const int64 CapturedTotal = SaveGame->AttendanceTotal;

	// 새 인스턴스 복원.
	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply succeeds"), Restored->ApplyFromSave(SaveGame));
	UAttendanceService* RestoredService = Restored->GetAttendanceService();
	TestNotNull(TEXT("Restored attendance service"), RestoredService);
	if (RestoredService)
	{
		// 복원 후 같은 날짜 CheckIn 은 중복이라 누적 불변.
		TestEqual(TEXT("Restored total persists"), RestoredService->GetTotalAttendance(), CapturedTotal);
		TestTrue(TEXT("Restored claim persists"), RestoredService->IsMilestoneClaimed(3));
	}

	// 레거시(<23) 세이브는 출석 빈 값으로 회귀 안전.
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 22;
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply legacy save succeeds"), LegacyInstance->ApplyFromSave(LegacySave));
	UAttendanceService* LegacyService = LegacyInstance->GetAttendanceService();
	TestNotNull(TEXT("Legacy attendance service"), LegacyService);
	if (LegacyService)
	{
		// 레거시는 누적 0 에서 시작하나, Apply 의 로그인 CheckIn 으로 1 이 될 수 있음(오늘 첫 출석).
		TestTrue(TEXT("Legacy total starts fresh"), LegacyService->GetTotalAttendance() <= static_cast<int64>(1));
		TestFalse(TEXT("Legacy claim empty"), LegacyService->IsMilestoneClaimed(3));
	}

	return true;
}

#endif
