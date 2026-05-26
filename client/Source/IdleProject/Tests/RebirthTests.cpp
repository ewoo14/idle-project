#include "Misc/AutomationTest.h"

#include "CharacterSystem/LevelFormulas.h"
#include "CharacterSystem/StatFormulas.h"
#include "CharacterSystem/StatPointFormula.h"
#include "GameCore/IdleGameInstance.h"
#include "GameCore/RebirthFormula.h"
#include "GameCore/TranscendFormula.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
bool PerformRebirths(UIdleGameInstance& GameInstance, int32 Count)
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthFormulaRewardTest,
	"IdleProject.GameCore.Rebirth.FormulaReward",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthFormulaRewardTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("First rebirth at level 100 keeps legacy five point reward"), FRebirthFormula::GetRebirthPointsReward(0, 100), static_cast<int32>(5));
	TestEqual(TEXT("Fifth rebirth at level 100 adds count scaling"), FRebirthFormula::GetRebirthPointsReward(4, 100), static_cast<int32>(13));
	TestEqual(TEXT("First rebirth at level 150 adds level scaling"), FRebirthFormula::GetRebirthPointsReward(0, 150), static_cast<int32>(10));
	TestEqual(TEXT("Fifth rebirth at level 150 combines count and level scaling"), FRebirthFormula::GetRebirthPointsReward(4, 150), static_cast<int32>(18));
	TestEqual(TEXT("Level 109 keeps the level 100 reward floor"), FRebirthFormula::GetRebirthPointsReward(0, 109), static_cast<int32>(5));
	TestEqual(TEXT("Level 110 adds the first level bonus point"), FRebirthFormula::GetRebirthPointsReward(0, 110), static_cast<int32>(6));
	TestEqual(TEXT("Negative inputs clamp to first level 100 reward"), FRebirthFormula::GetRebirthPointsReward(-1, 1), static_cast<int32>(5));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTranscendFormulaTest,
	"IdleProject.GameCore.Transcend.Formula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTranscendFormulaTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Transcend requires five rebirths"), FTranscendFormula::TranscendRebirthThreshold, static_cast<int32>(5));
	TestEqual(TEXT("Zero transcend count keeps neutral multiplier"), FTranscendFormula::GetTranscendStatMultiplier(0), 1.0f);
	TestEqual(TEXT("One transcend adds twenty five percent"), FTranscendFormula::GetTranscendStatMultiplier(1), 1.25f);
	TestEqual(TEXT("Four transcends double combat stats"), FTranscendFormula::GetTranscendStatMultiplier(4), 2.0f);
	TestEqual(TEXT("Ten transcends keep the infinite linear multiplier"), FTranscendFormula::GetTranscendStatMultiplier(10), 3.5f);
	TestEqual(TEXT("Negative transcend count clamps to neutral multiplier"), FTranscendFormula::GetTranscendStatMultiplier(-1), 1.0f);
	TestFalse(TEXT("Four rebirths cannot transcend"), FTranscendFormula::CanTranscend(4));
	TestTrue(TEXT("Five rebirths can transcend"), FTranscendFormula::CanTranscend(5));
	TestTrue(TEXT("Six rebirths can transcend"), FTranscendFormula::CanTranscend(6));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceStatAllocationTest,
	"IdleProject.GameCore.StatAllocation.AllocateAndReset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceStatAllocationTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestEqual(TEXT("New game starts with zero available stat points"), GameInstance->GetAvailableStatPoints(), 0);
	TestFalse(TEXT("Allocation fails with no available points"), GameInstance->AllocateStatPoint(EPrimaryStat::Str));

	GameInstance->GrantStatPoints(3);
	TestEqual(TEXT("GrantStatPoints adds available points"), GameInstance->GetAvailableStatPoints(), 3);
	TestTrue(TEXT("Allocation succeeds with available points"), GameInstance->AllocateStatPoint(EPrimaryStat::Str));

	const FPrimaryStats Allocated = GameInstance->GetAllocatedPrimaryStats();
	TestEqual(TEXT("Allocated STR increments by one"), Allocated.Str, 1.0f);
	TestEqual(TEXT("Available points decrement after allocation"), GameInstance->GetAvailableStatPoints(), 2);

	GameInstance->ResetStatPoints();
	const FPrimaryStats ResetAllocated = GameInstance->GetAllocatedPrimaryStats();
	TestEqual(TEXT("Reset returns spent points to available pool"), GameInstance->GetAvailableStatPoints(), 3);
	TestEqual(TEXT("Reset clears allocated STR"), ResetAllocated.Str, 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceLevelUpGrantsStatPointsTest,
	"IdleProject.GameCore.StatAllocation.LevelUpGrant",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceLevelUpGrantsStatPointsTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->LevelUp();

	TestEqual(TEXT("Level 2 grants formula stat points"), GameInstance->GetAvailableStatPoints(), FStatPointFormula::GetStatPointsForLevelUp(2));
	TestEqual(TEXT("Level 2 total stat points matches formula"), GameInstance->GetAvailableStatPoints(), FStatPointFormula::GetTotalStatPointsForLevel(GameInstance->GetCharacterLevel()));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceTranscendGateAndPreviewTest,
	"IdleProject.GameCore.Transcend.GateAndPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceTranscendGateAndPreviewTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestFalse(TEXT("Fresh game cannot transcend"), GameInstance->CanTranscend());
	TestEqual(TEXT("Fresh game has zero transcend count"), GameInstance->GetTranscendCount(), static_cast<int32>(0));
	TestEqual(TEXT("Fresh game has neutral transcend multiplier"), GameInstance->GetTranscendStatMultiplier(), 1.0f);
	TestEqual(TEXT("Preview shows the next transcend multiplier"), GameInstance->PreviewTranscendMultiplier(), 1.25f);
	TestFalse(TEXT("Transcend returns false before threshold"), GameInstance->Transcend());
	TestEqual(TEXT("Failed transcend keeps transcend count unchanged"), GameInstance->GetTranscendCount(), static_cast<int32>(0));
	TestEqual(TEXT("Failed transcend keeps level unchanged"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));
	TestEqual(TEXT("Failed transcend keeps gold unchanged"), GameInstance->GetGold(), static_cast<int64>(0));

	TestTrue(TEXT("Test setup performs four rebirths"), PerformRebirths(*GameInstance, 4));
	TestFalse(TEXT("Four rebirths still cannot transcend"), GameInstance->CanTranscend());

	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	GameInstance->MarkChapter1BossDefeated();
	TestTrue(TEXT("Fifth rebirth succeeds"), GameInstance->Rebirth());
	TestTrue(TEXT("Five rebirths can transcend"), GameInstance->CanTranscend());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceTranscendResetTest,
	"IdleProject.GameCore.Transcend.ResetAndCount",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceTranscendResetTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	TestTrue(TEXT("Test setup reaches transcend threshold"), PerformRebirths(*GameInstance, FTranscendFormula::TranscendRebirthThreshold));
	GameInstance->AddGold(1234);
	GameInstance->GrantStatPoints(2);
	TestTrue(TEXT("Allocated STR setup succeeds"), GameInstance->AllocateStatPoint(EPrimaryStat::Str));
	TestTrue(TEXT("Chapter boss setup is marked"), [&GameInstance]()
	{
		GameInstance->MarkChapter1BossDefeated();
		return GameInstance->HasDefeatedChapter1Boss();
	}());

	const int64 ExpectedNextExp = FLevelFormulas::ExpToNext(1);
	TestTrue(TEXT("Transcend succeeds at threshold"), GameInstance->Transcend());
	TestEqual(TEXT("Transcend count increments"), GameInstance->GetTranscendCount(), static_cast<int32>(1));
	TestEqual(TEXT("Rebirth count resets to zero"), GameInstance->GetRebirthCount(), static_cast<int32>(0));
	TestEqual(TEXT("Rebirth bonus points reset to zero"), GameInstance->GetRebirthBonusPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Level resets to one"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));
	TestEqual(TEXT("Exp resets to zero"), GameInstance->GetCurrentExp(), static_cast<int64>(0));
	TestEqual(TEXT("Next exp resets to level one curve"), GameInstance->GetNextExp(), ExpectedNextExp);
	TestEqual(TEXT("Available stat points reset to zero"), GameInstance->GetAvailableStatPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Allocated STR resets to zero"), GameInstance->GetAllocatedPrimaryStats().Str, 0.0f);
	TestEqual(TEXT("Transcend V1 resets gold to zero"), GameInstance->GetGold(), static_cast<int64>(0));
	TestFalse(TEXT("Chapter boss gate resets after transcend"), GameInstance->HasDefeatedChapter1Boss());
	TestEqual(TEXT("Current multiplier reflects new transcend count"), GameInstance->GetTranscendStatMultiplier(), 1.25f);
	TestEqual(TEXT("Preview advances from current count"), GameInstance->PreviewTranscendMultiplier(), 1.5f);
	TestFalse(TEXT("Cannot transcend again after rebirth count reset"), GameInstance->Transcend());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceRebirthGateTest,
	"IdleProject.GameCore.Rebirth.Gate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceRebirthGateTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100));
	TestEqual(TEXT("Test setup reaches level 100"), GameInstance->GetCharacterLevel(), static_cast<int32>(100));
	TestFalse(TEXT("Level 100 without chapter 1 boss clear cannot rebirth"), GameInstance->CanRebirth());

	UIdleGameInstance* BossClearedLowLevel = NewObject<UIdleGameInstance>();
	BossClearedLowLevel->MarkChapter1BossDefeated();
	TestFalse(TEXT("Boss clear below level 100 cannot rebirth"), BossClearedLowLevel->CanRebirth());

	GameInstance->MarkChapter1BossDefeated();
	TestTrue(TEXT("Boss clear and level 100 can rebirth"), GameInstance->CanRebirth());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceRebirthResetTest,
	"IdleProject.GameCore.Rebirth.ResetAndBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceRebirthResetTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	GameInstance->AddGold(1234);
	GameInstance->AddExp(FLevelFormulas::CumulativeExp(100) + 99);
	GameInstance->MarkChapter1BossDefeated();

	TestEqual(TEXT("Preview mirrors first rebirth reward before reset"), GameInstance->PreviewRebirthReward(), static_cast<int32>(5));
	TestTrue(TEXT("Rebirth succeeds when gate is met"), GameInstance->Rebirth());
	TestEqual(TEXT("Rebirth count increments"), GameInstance->GetRebirthCount(), static_cast<int32>(1));
	TestEqual(TEXT("Rebirth bonus points increase by five"), GameInstance->GetRebirthBonusPoints(), static_cast<int32>(5));
	TestEqual(TEXT("Rebirth clears available stat points"), GameInstance->GetAvailableStatPoints(), static_cast<int32>(0));
	TestEqual(TEXT("Rebirth clears allocated STR"), GameInstance->GetAllocatedPrimaryStats().Str, 0.0f);
	TestEqual(TEXT("Level resets to one"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));
	TestEqual(TEXT("Exp resets to zero"), GameInstance->GetCurrentExp(), static_cast<int64>(0));
	TestEqual(TEXT("Gold keeps ten percent rounded down"), GameInstance->GetGold(), static_cast<int64>(123));
	TestFalse(TEXT("Chapter boss gate resets after rebirth"), GameInstance->HasDefeatedChapter1Boss());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FIdleGameInstanceRebirthScalingRewardTest,
	"IdleProject.GameCore.Rebirth.ScalingReward",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FIdleGameInstanceRebirthScalingRewardTest::RunTest(const FString& Parameters)
{
	UIdleGameInstance* GameInstance = NewObject<UIdleGameInstance>();
	TestNotNull(TEXT("Game instance is created"), GameInstance);
	if (!GameInstance)
	{
		return false;
	}

	int32 ExpectedTotalBonusPoints = 0;
	const int32 RebirthLevels[] = { 100, 100, 100, 100, 150 };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(RebirthLevels); ++Index)
	{
		GameInstance->AddExp(FLevelFormulas::CumulativeExp(RebirthLevels[Index]));
		GameInstance->MarkChapter1BossDefeated();

		const int32 ExpectedReward = FRebirthFormula::GetRebirthPointsReward(Index, RebirthLevels[Index]);
		TestEqual(TEXT("Preview uses current count and level before rebirth"), GameInstance->PreviewRebirthReward(), ExpectedReward);

		TestTrue(TEXT("Rebirth succeeds for scaling setup"), GameInstance->Rebirth());
		ExpectedTotalBonusPoints += ExpectedReward;
		TestEqual(TEXT("Rebirth bonus points add scaled reward exactly once"), GameInstance->GetRebirthBonusPoints(), ExpectedTotalBonusPoints);
	}

	TestEqual(TEXT("Five rebirths are recorded"), GameInstance->GetRebirthCount(), static_cast<int32>(5));
	TestEqual(TEXT("Total bonus includes scaled fifth level 150 reward"), ExpectedTotalBonusPoints, static_cast<int32>(5 + 7 + 9 + 11 + 18));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRebirthStatBonusTest,
	"IdleProject.Character.StatFormulas.RebirthBonus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FRebirthStatBonusTest::RunTest(const FString& Parameters)
{
	const FPrimaryStats BasePrimary = FStatFormulas::DefaultPrimaryStats(EClassId::Warrior, 1);
	const FDerivedStats BaseDerived = FStatFormulas::DeriveStats(BasePrimary, 1);
	const FDerivedStats RebirthedDerived = FStatFormulas::DeriveStats(BasePrimary, 1, FDerivedStats(), 5);

	TestEqual(TEXT("Five rebirth points add ten physical attack"), RebirthedDerived.PhysAtk, BaseDerived.PhysAtk + 10.0f);
	TestEqual(TEXT("Five rebirth points add fifty max HP"), RebirthedDerived.Hp, BaseDerived.Hp + 50.0f);
	TestEqual(TEXT("Rebirth bonus does not alter attack speed"), RebirthedDerived.AtkSpeed, BaseDerived.AtkSpeed);

	return true;
}

#endif
