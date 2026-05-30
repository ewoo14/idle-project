#include "Misc/AutomationTest.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGenshinTokenTest, "IdleProject.UI.HUD.GenshinTokens",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGenshinTokenTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	TestTrue(TEXT("cream bright"), Theme::PanelCream.R > 0.8f && Theme::PanelCream.A < 1.0f);
	TestTrue(TEXT("slate dark"), Theme::PanelSlate.R < 0.4f && Theme::PanelSlate.A < 1.0f);
	TestTrue(TEXT("gold warm"), Theme::FrameGold.R > Theme::FrameGold.B);
	TestTrue(TEXT("text slate dark"), Theme::TextSlate.R < 0.4f);
	TestEqual(TEXT("corner radius"), Theme::PanelCornerRadius, 14.0f);
	return true;
}

#endif
