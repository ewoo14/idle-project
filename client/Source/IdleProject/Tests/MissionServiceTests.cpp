#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/IdleSaveGame.h"
#include "GameCore/MissionService.h"
#include "GameCore/MissionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// 익명 헬퍼는 jumbo ODR 충돌 방지를 위해 Mission~ prefix 를 사용한다.

	UMissionService* MissionMakeService()
	{
		UMissionService* Service = NewObject<UMissionService>();
		Service->InitializeDefaultMissions();
		// 테스트 기준 마커를 고정값으로 초기화(최초 호출이므로 리셋 없음).
		Service->EnsurePeriodFresh(TEXT("2026-05-30"), TEXT("2026-W22"));
		return Service;
	}

	const FMissionDefinition* MissionFindDefinition(const UMissionService* Service, const FString& Id)
	{
		if (!Service)
		{
			return nullptr;
		}
		for (const FMissionDefinition& Definition : Service->GetDefinitions())
		{
			if (Definition.Id == Id)
			{
				return &Definition;
			}
		}
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionCatalogParityTest,
	"IdleProject.GameCore.Mission.CatalogParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionCatalogParityTest::RunTest(const FString& Parameters)
{
	UMissionService* Service = MissionMakeService();
	TestNotNull(TEXT("Mission service created"), Service);
	if (!Service)
	{
		return false;
	}

	// 서버 mission.ts MISSION_CATALOG 와 1:1: 일일 6 + 주간 4 = 10종.
	TestEqual(TEXT("Catalog has 10 missions"), Service->GetDefinitions().Num(), 10);

	int32 DailyCount = 0;
	int32 WeeklyCount = 0;
	TSet<FString> SeenIds;
	for (const FMissionDefinition& Definition : Service->GetDefinitions())
	{
		TestFalse(FString::Printf(TEXT("Mission id %s is unique"), *Definition.Id), SeenIds.Contains(Definition.Id));
		SeenIds.Add(Definition.Id);
		TestTrue(FString::Printf(TEXT("Mission %s has a positive target"), *Definition.Id), Definition.Target > 0);
		TestTrue(FString::Printf(TEXT("Mission %s has a positive reward"), *Definition.Id), Definition.RewardValue > 0);
		Definition.Period == EMissionPeriod::Daily ? ++DailyCount : ++WeeklyCount;
	}
	TestEqual(TEXT("Six daily missions"), DailyCount, 6);
	TestEqual(TEXT("Four weekly missions"), WeeklyCount, 4);

	// 대표 앵커 — 서버 카탈로그 metric/target/reward 1:1.
	const FMissionDefinition* DailyKill = MissionFindDefinition(Service, TEXT("daily_kill_300"));
	TestNotNull(TEXT("daily_kill_300 exists"), DailyKill);
	if (DailyKill)
	{
		TestEqual(TEXT("daily_kill_300 period"), DailyKill->Period, EMissionPeriod::Daily);
		TestEqual(TEXT("daily_kill_300 metric"), DailyKill->Metric, EMissionMetric::MonstersKilled);
		TestEqual(TEXT("daily_kill_300 target"), DailyKill->Target, static_cast<int64>(300));
		TestEqual(TEXT("daily_kill_300 reward type"), DailyKill->RewardType, EMissionReward::Gold);
		TestEqual(TEXT("daily_kill_300 reward value"), DailyKill->RewardValue, static_cast<int64>(50000));
	}

	const FMissionDefinition* DailyDungeon = MissionFindDefinition(Service, TEXT("daily_dungeon_3"));
	TestNotNull(TEXT("daily_dungeon_3 exists"), DailyDungeon);
	if (DailyDungeon)
	{
		TestEqual(TEXT("daily_dungeon_3 metric"), DailyDungeon->Metric, EMissionMetric::DungeonRuns);
		TestEqual(TEXT("daily_dungeon_3 target"), DailyDungeon->Target, static_cast<int64>(3));
		TestEqual(TEXT("daily_dungeon_3 reward type"), DailyDungeon->RewardType, EMissionReward::Consumable);
		TestEqual(TEXT("daily_dungeon_3 reward value"), DailyDungeon->RewardValue, static_cast<int64>(1));
	}

	const FMissionDefinition* WeeklyBoss = MissionFindDefinition(Service, TEXT("weekly_boss_50"));
	TestNotNull(TEXT("weekly_boss_50 exists"), WeeklyBoss);
	if (WeeklyBoss)
	{
		TestEqual(TEXT("weekly_boss_50 period"), WeeklyBoss->Period, EMissionPeriod::Weekly);
		TestEqual(TEXT("weekly_boss_50 metric"), WeeklyBoss->Metric, EMissionMetric::BossesKilled);
		TestEqual(TEXT("weekly_boss_50 target"), WeeklyBoss->Target, static_cast<int64>(50));
		TestEqual(TEXT("weekly_boss_50 reward type"), WeeklyBoss->RewardType, EMissionReward::Essence);
		TestEqual(TEXT("weekly_boss_50 reward value"), WeeklyBoss->RewardValue, static_cast<int64>(30));
	}

	const FMissionDefinition* WeeklyDungeon = MissionFindDefinition(Service, TEXT("weekly_dungeon_15"));
	TestNotNull(TEXT("weekly_dungeon_15 exists"), WeeklyDungeon);
	if (WeeklyDungeon)
	{
		TestEqual(TEXT("weekly_dungeon_15 target"), WeeklyDungeon->Target, static_cast<int64>(15));
		TestEqual(TEXT("weekly_dungeon_15 reward type"), WeeklyDungeon->RewardType, EMissionReward::Gold);
		TestEqual(TEXT("weekly_dungeon_15 reward value"), WeeklyDungeon->RewardValue, static_cast<int64>(800000));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionProgressAndCompletionTest,
	"IdleProject.GameCore.Mission.ProgressCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionProgressAndCompletionTest::RunTest(const FString& Parameters)
{
	UMissionService* Service = MissionMakeService();
	if (!Service)
	{
		return false;
	}

	// daily_kill_300 / weekly_kill_5000 둘 다 MonstersKilled 를 공유 — 같은 델타로 함께 누적.
	Service->RecordProgress(EMissionMetric::MonstersKilled, 299);
	TestEqual(TEXT("Progress accumulates to 299"), Service->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(299));
	TestFalse(TEXT("daily_kill_300 not complete at target-1"), Service->IsComplete(TEXT("daily_kill_300")));
	TestEqual(TEXT("weekly shares MonstersKilled metric"), Service->GetProgress(TEXT("weekly_kill_5000")), static_cast<int64>(299));

	Service->RecordProgress(EMissionMetric::MonstersKilled, 1);
	TestTrue(TEXT("daily_kill_300 complete at target"), Service->IsComplete(TEXT("daily_kill_300")));
	TestFalse(TEXT("weekly_kill_5000 still incomplete"), Service->IsComplete(TEXT("weekly_kill_5000")));

	// 0/음수 델타는 무시(누적형 가드).
	Service->RecordProgress(EMissionMetric::MonstersKilled, 0);
	Service->RecordProgress(EMissionMetric::MonstersKilled, -100);
	TestEqual(TEXT("Non-positive delta ignored"), Service->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(300));

	// 다른 metric 미션은 영향 없음.
	TestEqual(TEXT("Dungeon mission untouched"), Service->GetProgress(TEXT("daily_dungeon_3")), static_cast<int64>(0));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionClaimTest,
	"IdleProject.GameCore.Mission.Claim",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionClaimTest::RunTest(const FString& Parameters)
{
	UMissionService* Service = MissionMakeService();
	if (!Service)
	{
		return false;
	}

	// 미달 상태에서 수령 거부.
	Service->RecordProgress(EMissionMetric::DungeonRuns, 2);
	TestFalse(TEXT("Claim rejected when incomplete"), Service->ClaimMission(TEXT("daily_dungeon_3")));
	TestFalse(TEXT("Not claimed after rejection"), Service->IsClaimed(TEXT("daily_dungeon_3")));

	// 완료 후 1회 수령 성공.
	Service->RecordProgress(EMissionMetric::DungeonRuns, 1);
	TestTrue(TEXT("Claim succeeds when complete"), Service->ClaimMission(TEXT("daily_dungeon_3")));
	TestTrue(TEXT("Marked claimed"), Service->IsClaimed(TEXT("daily_dungeon_3")));

	// 중복 수령 거부.
	TestFalse(TEXT("Duplicate claim rejected"), Service->ClaimMission(TEXT("daily_dungeon_3")));

	// 무효 id 거부.
	TestFalse(TEXT("Unknown id claim rejected"), Service->ClaimMission(TEXT("does_not_exist")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionDailyResetTest,
	"IdleProject.GameCore.Mission.DailyReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionDailyResetTest::RunTest(const FString& Parameters)
{
	UMissionService* Service = MissionMakeService();
	if (!Service)
	{
		return false;
	}

	// 일일/주간 모두 진행 + 일일 1건 수령.
	Service->RecordProgress(EMissionMetric::DungeonRuns, 3); // daily_dungeon_3 + weekly_dungeon_15
	Service->RecordProgress(EMissionMetric::BossesKilled, 5); // daily_boss_5 + weekly_boss_50
	TestTrue(TEXT("daily_dungeon_3 complete"), Service->ClaimMission(TEXT("daily_dungeon_3")));

	// 날짜 변경(주는 동일) → 일일만 리셋.
	Service->EnsurePeriodFresh(TEXT("2026-05-31"), TEXT("2026-W22"));

	TestEqual(TEXT("Daily dungeon progress reset"), Service->GetProgress(TEXT("daily_dungeon_3")), static_cast<int64>(0));
	TestFalse(TEXT("Daily dungeon claim cleared"), Service->IsClaimed(TEXT("daily_dungeon_3")));
	TestEqual(TEXT("Daily boss progress reset"), Service->GetProgress(TEXT("daily_boss_5")), static_cast<int64>(0));

	// 주간 진행은 유지.
	TestEqual(TEXT("Weekly dungeon progress kept"), Service->GetProgress(TEXT("weekly_dungeon_15")), static_cast<int64>(3));
	TestEqual(TEXT("Weekly boss progress kept"), Service->GetProgress(TEXT("weekly_boss_50")), static_cast<int64>(5));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionWeeklyResetTest,
	"IdleProject.GameCore.Mission.WeeklyReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionWeeklyResetTest::RunTest(const FString& Parameters)
{
	UMissionService* Service = MissionMakeService();
	if (!Service)
	{
		return false;
	}

	Service->RecordProgress(EMissionMetric::DungeonRuns, 4); // daily + weekly
	Service->RecordProgress(EMissionMetric::StagesCleared, 25); // daily_stage_20 (complete) + weekly_stage_150

	// 주 변경(날짜도 변경) → 주간 리셋. (날짜 변경으로 일일도 리셋되므로 주간 분리 검증.)
	Service->EnsurePeriodFresh(TEXT("2026-06-06"), TEXT("2026-W23"));

	TestEqual(TEXT("Weekly dungeon progress reset"), Service->GetProgress(TEXT("weekly_dungeon_15")), static_cast<int64>(0));
	TestEqual(TEXT("Weekly stage progress reset"), Service->GetProgress(TEXT("weekly_stage_150")), static_cast<int64>(0));

	// 같은 주 재호출은 추가 리셋 없음(멱등).
	Service->RecordProgress(EMissionMetric::DungeonRuns, 2);
	Service->EnsurePeriodFresh(TEXT("2026-06-06"), TEXT("2026-W23"));
	TestEqual(TEXT("Same week idempotent"), Service->GetProgress(TEXT("weekly_dungeon_15")), static_cast<int64>(2));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionGameInstanceHookTest,
	"IdleProject.GameCore.Mission.GameInstanceHook",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionGameInstanceHookTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	// 중앙 후크: RecordAchievementMetric 의 누적형 metric 이 미션 진행에 1회만 누적된다.
	GameInstance->RecordAchievementMetric(EAchievementMetric::MonstersKilled, 10);
	UMissionService* Mission = GameInstance->GetMissionService();
	TestNotNull(TEXT("Mission service available"), Mission);
	if (!Mission)
	{
		return false;
	}
	TestEqual(TEXT("MonstersKilled hooked once"), Mission->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(10));

	// Maximum 모드 등 미매핑 metric 은 미션 진행에 영향 없음.
	GameInstance->RecordAchievementMetric(EAchievementMetric::TowerHighestFloor, 50);
	TestEqual(TEXT("Unmapped metric does not affect missions"), Mission->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(10));

	// GoldEarned 후크(AddGold 경유는 별도이나 직접 호출로 매핑 확인).
	GameInstance->RecordAchievementMetric(EAchievementMetric::GoldEarned, 1000);
	TestEqual(TEXT("GoldEarned hooked"), Mission->GetProgress(TEXT("daily_gold_1m")), static_cast<int64>(1000));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionClaimRewardTest,
	"IdleProject.GameCore.Mission.ClaimReward",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionClaimRewardTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UMissionService* Mission = GameInstance->GetMissionService();
	TestNotNull(TEXT("Mission service available"), Mission);
	if (!Mission)
	{
		return false;
	}

	const int64 GoldBefore = GameInstance->GetGold();

	// 미달 상태에서 GameInstance 진입점 수령 거부(보상 미지급).
	GameInstance->RecordAchievementMetric(EAchievementMetric::GearEnhanced, 9);
	TestFalse(TEXT("Claim rejected when incomplete"), GameInstance->ClaimMission(TEXT("daily_enhance_10")));
	TestEqual(TEXT("Gold unchanged on rejected claim"), GameInstance->GetGold(), GoldBefore);

	// 완료 후 골드 보상(daily_enhance_10 = gold 80000) 단일 지급.
	GameInstance->RecordAchievementMetric(EAchievementMetric::GearEnhanced, 1);
	TestTrue(TEXT("Claim succeeds when complete"), GameInstance->ClaimMission(TEXT("daily_enhance_10")));
	TestEqual(TEXT("Gold reward granted once"), GameInstance->GetGold(), GoldBefore + 80000);

	// 중복 수령은 거부(추가 지급 없음).
	TestFalse(TEXT("Duplicate claim rejected"), GameInstance->ClaimMission(TEXT("daily_enhance_10")));
	TestEqual(TEXT("No double reward"), GameInstance->GetGold(), GoldBefore + 80000);

	// 정수 보상(daily_boss_5 = essence 3).
	const int64 EssenceBefore = GameInstance->GetRuneEssence();
	GameInstance->RecordAchievementMetric(EAchievementMetric::BossesKilled, 5);
	TestTrue(TEXT("Essence claim succeeds"), GameInstance->ClaimMission(TEXT("daily_boss_5")));
	TestEqual(TEXT("Essence reward granted"), GameInstance->GetRuneEssence(), EssenceBefore + 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMissionSaveRoundTripTest,
	"IdleProject.GameCore.Mission.SaveRoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMissionSaveRoundTripTest::RunTest(const FString& Parameters)
{
	// SaveVer 22 라운드트립: 진행/수령/리셋 마커가 capture/apply 를 통해 보존된다.
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	if (!GameInstance)
	{
		return false;
	}

	UMissionService* Mission = GameInstance->GetMissionService();
	TestNotNull(TEXT("Mission service available"), Mission);
	if (!Mission)
	{
		return false;
	}

	// 진행 누적 + 1건 완료/수령.
	GameInstance->RecordAchievementMetric(EAchievementMetric::MonstersKilled, 120);
	GameInstance->RecordAchievementMetric(EAchievementMetric::GearEnhanced, 10);
	TestTrue(TEXT("daily_enhance_10 claimed"), GameInstance->ClaimMission(TEXT("daily_enhance_10")));

	UIdleSaveGame* SaveGame = NewObject<UIdleSaveGame>();
	TestTrue(TEXT("Capture succeeds"), GameInstance->CaptureToSave(SaveGame));
	TestEqual(TEXT("Capture writes V27"), SaveGame->SaveVersion, static_cast<int32>(27));
	TestEqual(TEXT("Saved progress for daily_kill_300"), SaveGame->MissionProgress.FindRef(TEXT("daily_kill_300")), static_cast<int64>(120));
	TestTrue(TEXT("Saved claimed contains daily_enhance_10"), SaveGame->MissionClaimed.Contains(TEXT("daily_enhance_10")));

	// 새 인스턴스 복원.
	UIdleGameInstance* Restored = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply succeeds"), Restored->ApplyFromSave(SaveGame));
	UMissionService* RestoredMission = Restored->GetMissionService();
	TestNotNull(TEXT("Restored mission service"), RestoredMission);
	if (RestoredMission)
	{
		TestEqual(TEXT("Restored progress persists"), RestoredMission->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(120));
		TestTrue(TEXT("Restored claim persists"), RestoredMission->IsClaimed(TEXT("daily_enhance_10")));
		TestTrue(TEXT("Restored completion persists"), RestoredMission->IsComplete(TEXT("daily_enhance_10")));
	}

	// 레거시(<22) 세이브는 미션 빈 값으로 회귀 안전.
	UIdleSaveGame* LegacySave = NewObject<UIdleSaveGame>();
	LegacySave->bHasSave = true;
	LegacySave->SaveVersion = 21;
	UIdleGameInstance* LegacyInstance = NewObject<UIdleGameInstance>();
	TestTrue(TEXT("Apply legacy save succeeds"), LegacyInstance->ApplyFromSave(LegacySave));
	UMissionService* LegacyMission = LegacyInstance->GetMissionService();
	TestNotNull(TEXT("Legacy mission service"), LegacyMission);
	if (LegacyMission)
	{
		TestEqual(TEXT("Legacy progress empty"), LegacyMission->GetProgress(TEXT("daily_kill_300")), static_cast<int64>(0));
		TestFalse(TEXT("Legacy claim empty"), LegacyMission->IsClaimed(TEXT("daily_enhance_10")));
	}

	return true;
}

#endif
