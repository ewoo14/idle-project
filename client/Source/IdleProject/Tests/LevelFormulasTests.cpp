#include "Misc/AutomationTest.h"
#include "CharacterSystem/LevelFormulas.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLevelFormulasExpTest,
	"IdleProject.Character.LevelFormulas.Exp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLevelFormulasExpTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("LEVEL_CAP"), FLevelFormulas::LEVEL_CAP, 200);
	TestEqual(TEXT("ExpToNext(1)"), FLevelFormulas::ExpToNext(1), 150);
	TestEqual(TEXT("ExpToNext(10)"), FLevelFormulas::ExpToNext(10), 3506);
	TestEqual(TEXT("ExpToNext(100)"), FLevelFormulas::ExpToNext(100), 135594);
	TestEqual(TEXT("ExpToNext(200) cap sentinel"), FLevelFormulas::ExpToNext(200), 0);

	const int64 ExpectedLevel4Total =
		FLevelFormulas::ExpToNext(1) +
		FLevelFormulas::ExpToNext(2) +
		FLevelFormulas::ExpToNext(3);
	TestEqual(TEXT("CumulativeExp(1)"), FLevelFormulas::CumulativeExp(1), 0);
	TestEqual(TEXT("CumulativeExp(4)"), FLevelFormulas::CumulativeExp(4), ExpectedLevel4Total);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLevelFormulasEnhanceTest,
	"IdleProject.Character.LevelFormulas.Enhance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLevelFormulasEnhanceTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("EnhanceSuccessRate(5)"), FLevelFormulas::EnhanceSuccessRate(5), 1.0f);
	TestEqual(TEXT("EnhanceSuccessRate(10)"), FLevelFormulas::EnhanceSuccessRate(10), 0.9f);
	TestEqual(TEXT("EnhanceSuccessRate(15)"), FLevelFormulas::EnhanceSuccessRate(15), 0.3f);

	return true;
}

#endif
