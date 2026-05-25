#include "Misc/AutomationTest.h"

#include "CharacterSystem/LevelFormulas.h"
#include "CharacterSystem/StatFormulas.h"
#include "GameCore/IdleGameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

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

	TestTrue(TEXT("Rebirth succeeds when gate is met"), GameInstance->Rebirth());
	TestEqual(TEXT("Rebirth count increments"), GameInstance->GetRebirthCount(), static_cast<int32>(1));
	TestEqual(TEXT("Rebirth bonus points increase by five"), GameInstance->GetRebirthBonusPoints(), static_cast<int32>(5));
	TestEqual(TEXT("Level resets to one"), GameInstance->GetCharacterLevel(), static_cast<int32>(1));
	TestEqual(TEXT("Exp resets to zero"), GameInstance->GetCurrentExp(), static_cast<int64>(0));
	TestEqual(TEXT("Gold keeps ten percent rounded down"), GameInstance->GetGold(), static_cast<int64>(123));
	TestFalse(TEXT("Chapter boss gate resets after rebirth"), GameInstance->HasDefeatedChapter1Boss());

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
