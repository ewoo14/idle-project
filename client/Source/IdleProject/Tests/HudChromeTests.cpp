#include "Misc/AutomationTest.h"
#include "UI/UIThemeTokens.h"
#include "UI/HudChrome.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudChromeLogicTest, "IdleProject.UI.HUD.ChromeLogic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHudChromeLogicTest::RunTest(const FString& Parameters)
{
    using namespace IdleProject::UI;
    FNineSliceCell cells[9];
    ComputeNineSlice(FUiRect{0,0,100,80}, 10.0f, 64.0f, 16.0f, cells);
    TestEqual(TEXT("TL w"), cells[0].Dst.W, 10.0f);
    TestEqual(TEXT("TL h"), cells[0].Dst.H, 10.0f);
    TestEqual(TEXT("center w"), cells[4].Dst.W, 80.0f);
    TestEqual(TEXT("center h"), cells[4].Dst.H, 60.0f);
    TestTrue(TEXT("uv corner"), FMath::IsNearlyEqual(cells[0].UV.W, 0.25f));
    TestEqual(TEXT("cream tex"), PanelTextureName(EPanelStyle::Cream), FString(TEXT("T_GenshinPanel")));
    TestEqual(TEXT("slate tex"), PanelTextureName(EPanelStyle::Slate), FString(TEXT("T_GenshinPanelSlate")));
    TArray<FUiRect> stars = ComputeStarRects(10.0f, 5.0f, 5, 12.0f, 3.0f);
    TestEqual(TEXT("5 stars"), stars.Num(), 5);
    TestEqual(TEXT("star0 x"), stars[0].X, 10.0f);
    TestEqual(TEXT("star1 x"), stars[1].X, 25.0f);
    return true;
}

#endif
