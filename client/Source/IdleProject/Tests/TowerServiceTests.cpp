#include "Misc/AutomationTest.h"

#include "GameCore/IdleGameInstance.h"
#include "GameCore/TowerFormula.h"
#include "GameCore/TowerService.h"
#include "Tests/TowerEventTestReceiver.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTowerFormulaScalingTest,
	"IdleProject.GameCore.Tower.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTowerFormulaScalingTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Floor zero clamps to floor one requirement"), FTowerFormula::GetFloorRequiredPower(0), static_cast<int64>(100));
	TestEqual(TEXT("Floor one starts at 100 CP"), FTowerFormula::GetFloorRequiredPower(1), static_cast<int64>(100));
	TestEqual(TEXT("Floor two rounds 15 percent growth"), FTowerFormula::GetFloorRequiredPower(2), static_cast<int64>(115));
	TestEqual(TEXT("Floor three rounds compounded growth"), FTowerFormula::GetFloorRequiredPower(3), static_cast<int64>(132));

	TestTrue(TEXT("Exact CP clears required floor"), FTowerFormula::CanClearFloor(115, 2));
	TestFalse(TEXT("One CP below requirement cannot clear"), FTowerFormula::CanClearFloor(114, 2));
	TestEqual(TEXT("Floor zero reward clamps to first floor"), FTowerFormula::GetFloorReward(0), static_cast<int64>(50));
	TestEqual(TEXT("Floor three reward is linear gold"), FTowerFormula::GetFloorReward(3), static_cast<int64>(150));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTowerServiceClimbTest,
	"IdleProject.GameCore.Tower.ServiceClimb",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTowerServiceClimbTest::RunTest(const FString& Parameters)
{
	UTowerService* Tower = NewObject<UTowerService>();
	Tower->InitializeTower();
	UTowerEventTestReceiver* Receiver = NewObject<UTowerEventTestReceiver>();
	Tower->OnTowerClimbed.AddDynamic(Receiver, &UTowerEventTestReceiver::CaptureTowerClimbed);

	TestEqual(TEXT("Tower starts before floor one"), Tower->GetHighestFloor(), 0);
	TestEqual(TEXT("Next floor requirement starts at floor one"), Tower->GetNextFloorRequiredPower(), static_cast<int64>(100));

	TestEqual(TEXT("Insufficient CP returns no reward"), Tower->TryClimbTower(99), static_cast<int64>(0));
	TestEqual(TEXT("Insufficient CP leaves highest floor unchanged"), Tower->GetHighestFloor(), 0);
	TestEqual(TEXT("Insufficient CP does not broadcast climb"), Receiver->Count, 0);

	const int64 Reward = Tower->TryClimbTower(132);
	TestEqual(TEXT("CP 132 clears floors one through three"), Tower->GetHighestFloor(), 3);
	TestEqual(TEXT("Reward sums newly cleared floors"), Reward, static_cast<int64>(50 + 100 + 150));
	TestEqual(TEXT("Next floor requirement advances after climb"), Tower->GetNextFloorRequiredPower(), FTowerFormula::GetFloorRequiredPower(4));
	TestEqual(TEXT("Climb broadcasts once"), Receiver->Count, 1);
	TestEqual(TEXT("Climb broadcast reports new highest floor"), Receiver->LastHighestFloor, 3);
	TestEqual(TEXT("Climb broadcast reports total reward"), Receiver->LastTotalReward, Reward);

	TestEqual(TEXT("Same CP cannot claim already cleared rewards"), Tower->TryClimbTower(132), static_cast<int64>(0));
	TestEqual(TEXT("No-new-floor call does not rebroadcast"), Receiver->Count, 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTowerServiceMaxClimbCapTest,
	"IdleProject.GameCore.Tower.MaxClimbCap",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTowerServiceMaxClimbCapTest::RunTest(const FString& Parameters)
{
	UTowerService* Tower = NewObject<UTowerService>();
	Tower->InitializeTower();

	const int64 Reward = Tower->TryClimbTower(MAX_int64);

	TestEqual(TEXT("One call climbs at most the per-call cap"), Tower->GetHighestFloor(), UTowerService::MaxClimbPerCall);
	TestTrue(TEXT("Cap-limited climb still returns reward"), Reward > 0);

	const int32 PreviousFloor = Tower->GetHighestFloor();
	Tower->TryClimbTower(FTowerFormula::GetFloorRequiredPower(PreviousFloor + UTowerService::MaxClimbPerCall + 1));
	TestEqual(TEXT("Second call can progress by another capped batch"), Tower->GetHighestFloor(), PreviousFloor + UTowerService::MaxClimbPerCall);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceTowerHooksTest,
	"IdleProject.GameCore.IdleGameInstance.TowerHooks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceTowerHooksTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	GameInstance->InitializeTowerServiceForTests();

	UTowerService* Tower = GameInstance->GetTowerService();
	TestNotNull(TEXT("Game instance creates tower service for tests"), Tower);
	TestEqual(TEXT("Game instance tower starts before floor one"), Tower ? Tower->GetHighestFloor() : INDEX_NONE, 0);
	TestEqual(TEXT("Climb without a player character is safely ignored"), GameInstance->ClimbTower(), static_cast<int64>(0));
	TestEqual(TEXT("Ignored climb leaves gold unchanged"), GameInstance->GetGold(), static_cast<int64>(0));

	return true;
}

#endif
