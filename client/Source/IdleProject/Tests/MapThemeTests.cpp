#include "Misc/AutomationTest.h"
#include "GameCore/MapThemeLibrary.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMapThemeLibraryTest,
	"IdleProject.MapTheme.Library",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMapThemeLibraryTest::RunTest(const FString& Parameters)
{
	// 1~8 모두 프롭 보유 + 유효 강도.
	for (int32 C = 1; C <= FMapThemeLibrary::ThemeCount; ++C)
	{
		const FMapTheme T = FMapThemeLibrary::GetTheme(C);
		TestTrue(FString::Printf(TEXT("chapter %d has props"), C), T.Props.Num() > 0);
		TestTrue(FString::Printf(TEXT("chapter %d sun intensity"), C), T.SunIntensity > 0.0f);
	}
	// 클램프: 0/9 → 1/8 동일.
	const FMapTheme Low = FMapThemeLibrary::GetTheme(0);
	const FMapTheme C1 = FMapThemeLibrary::GetTheme(1);
	TestEqual(TEXT("clamp low to ch1 intensity"), Low.SunIntensity, C1.SunIntensity);
	const FMapTheme High = FMapThemeLibrary::GetTheme(99);
	const FMapTheme C8 = FMapThemeLibrary::GetTheme(8);
	TestEqual(TEXT("clamp high to ch8 intensity"), High.SunIntensity, C8.SunIntensity);
	// 챕터별 차별화: ch1(녹) vs ch7(적) 태양색 상이.
	TestTrue(TEXT("ch1 vs ch7 distinct sun"), !FMapThemeLibrary::GetTheme(1).SunColor.Equals(FMapThemeLibrary::GetTheme(7).SunColor));
	return true;
}

#endif
