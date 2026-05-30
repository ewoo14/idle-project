#include "Misc/AutomationTest.h"
#include "UI/HudNavigation.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace IdleProject::UI;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavCategoryRegistryTest,
    "IdleProject.UI.HUD.NavCategoryRegistry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavCategoryRegistryTest::RunTest(const FString& Parameters)
{
    const TArray<EHudCategory>& Cats = AllHudCategories();
    TestEqual(TEXT("7 categories"), Cats.Num(), 7);
    TestEqual(TEXT("first is Combat"), static_cast<int32>(Cats[0]), static_cast<int32>(EHudCategory::Combat));
    TestEqual(TEXT("last is Social"), static_cast<int32>(Cats[6]), static_cast<int32>(EHudCategory::Social));

    TestEqual(TEXT("Combat has 3"), PanelsForCategory(EHudCategory::Combat).Num(), 3);
    TestEqual(TEXT("Growth has 3"), PanelsForCategory(EHudCategory::Growth).Num(), 3);
    TestEqual(TEXT("Rebirth has 3"), PanelsForCategory(EHudCategory::Rebirth).Num(), 3);
    TestEqual(TEXT("Gear has 5"), PanelsForCategory(EHudCategory::Gear).Num(), 5);
    TestEqual(TEXT("Collection has 4"), PanelsForCategory(EHudCategory::Collection).Num(), 4);
    TestEqual(TEXT("Daily has 5"), PanelsForCategory(EHudCategory::Daily).Num(), 5);
    TestEqual(TEXT("Social has 2"), PanelsForCategory(EHudCategory::Social).Num(), 2);

    for (const EHudCategory Cat : Cats)
    {
        for (const EHudPanel Panel : PanelsForCategory(Cat))
        {
            TestEqual(TEXT("panel maps back to its category"), static_cast<int32>(CategoryForPanel(Panel)), static_cast<int32>(Cat));
        }
    }
    int32 Total = 0;
    for (const EHudCategory Cat : Cats) { Total += PanelsForCategory(Cat).Num(); }
    TestEqual(TEXT("25 panels total"), Total, 25);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavLayoutModeTest,
    "IdleProject.UI.HUD.NavLayoutMode",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavLayoutModeTest::RunTest(const FString& Parameters)
{
    // 명확히 가로 → Desktop.
    TestEqual(TEXT("wide -> Desktop"), static_cast<int32>(ResolveLayoutMode(1.78f, EHudLayoutMode::Mobile)), static_cast<int32>(EHudLayoutMode::Desktop));
    // 명확히 세로 → Mobile.
    TestEqual(TEXT("tall -> Mobile"), static_cast<int32>(ResolveLayoutMode(0.56f, EHudLayoutMode::Desktop)), static_cast<int32>(EHudLayoutMode::Mobile));
    // 히스테리시스 데드존(1.25~1.35): 현재 모드 유지.
    TestEqual(TEXT("deadzone keeps Desktop"), static_cast<int32>(ResolveLayoutMode(1.30f, EHudLayoutMode::Desktop)), static_cast<int32>(EHudLayoutMode::Desktop));
    TestEqual(TEXT("deadzone keeps Mobile"), static_cast<int32>(ResolveLayoutMode(1.30f, EHudLayoutMode::Mobile)), static_cast<int32>(EHudLayoutMode::Mobile));
    // 경계 바깥에서만 전환.
    TestEqual(TEXT("below 1.25 from Desktop -> Mobile"), static_cast<int32>(ResolveLayoutMode(1.24f, EHudLayoutMode::Desktop)), static_cast<int32>(EHudLayoutMode::Mobile));
    TestEqual(TEXT("above 1.35 from Mobile -> Desktop"), static_cast<int32>(ResolveLayoutMode(1.36f, EHudLayoutMode::Mobile)), static_cast<int32>(EHudLayoutMode::Desktop));
    // 정확 경계: 1.25는 Mobile, 1.35는 Desktop (경계 포함성 회귀 방지).
    TestEqual(TEXT("exactly 1.25 -> Mobile"), static_cast<int32>(ResolveLayoutMode(1.25f, EHudLayoutMode::Desktop)), static_cast<int32>(EHudLayoutMode::Mobile));
    TestEqual(TEXT("exactly 1.35 -> Desktop"), static_cast<int32>(ResolveLayoutMode(1.35f, EHudLayoutMode::Mobile)), static_cast<int32>(EHudLayoutMode::Desktop));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavStateMachineTest,
    "IdleProject.UI.HUD.NavStateMachine",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavStateMachineTest::RunTest(const FString& Parameters)
{
    FHudNavState S;
    TestFalse(TEXT("default closed"), IsNavOpen(S));
    TestEqual(TEXT("default category None"), static_cast<int32>(S.ActiveCategory), static_cast<int32>(EHudCategory::None));

    SelectCategory(S, EHudCategory::Gear);
    TestTrue(TEXT("open after select"), IsNavOpen(S));
    TestEqual(TEXT("Gear active"), static_cast<int32>(S.ActiveCategory), static_cast<int32>(EHudCategory::Gear));
    TestEqual(TEXT("first panel = Enhance"), static_cast<int32>(S.ActivePanel), static_cast<int32>(EHudPanel::Enhance));

    SelectPanel(S, EHudPanel::Rune);
    TestEqual(TEXT("now Rune"), static_cast<int32>(S.ActivePanel), static_cast<int32>(EHudPanel::Rune));

    SelectCategory(S, EHudCategory::Social);
    TestEqual(TEXT("Social first = Guild"), static_cast<int32>(S.ActivePanel), static_cast<int32>(EHudPanel::Guild));
    SelectCategory(S, EHudCategory::Gear);
    TestEqual(TEXT("Gear restores Rune"), static_cast<int32>(S.ActivePanel), static_cast<int32>(EHudPanel::Rune));

    SelectCategory(S, EHudCategory::Gear);
    TestFalse(TEXT("re-select closes"), IsNavOpen(S));
    TestEqual(TEXT("category cleared"), static_cast<int32>(S.ActiveCategory), static_cast<int32>(EHudCategory::None));

    SelectCategory(S, EHudCategory::Daily);
    CloseNav(S);
    TestFalse(TEXT("closed by CloseNav"), IsNavOpen(S));

    SelectCategory(S, EHudCategory::Combat);
    SelectPanel(S, EHudPanel::Shop); // Shop은 Gear → 무시
    TestEqual(TEXT("cross-category panel ignored"), static_cast<int32>(S.ActivePanel), static_cast<int32>(EHudPanel::Tower));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavLocKeyTest,
    "IdleProject.UI.HUD.NavLocKey",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavLocKeyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("combat key"), CategoryLocKey(EHudCategory::Combat), FString(TEXT("HUD_CAT_COMBAT")));
    TestEqual(TEXT("social key"), CategoryLocKey(EHudCategory::Social), FString(TEXT("HUD_CAT_SOCIAL")));
    TestEqual(TEXT("tower key"), PanelLocKey(EHudPanel::Tower), FString(TEXT("HUD_PANEL_TOWER")));
    TestEqual(TEXT("runecodex key"), PanelLocKey(EHudPanel::RuneCodex), FString(TEXT("HUD_PANEL_RUNECODEX")));
    for (const EHudCategory Cat : AllHudCategories())
    {
        TestFalse(TEXT("category key non-empty"), CategoryLocKey(Cat).IsEmpty());
        for (const EHudPanel P : PanelsForCategory(Cat))
        {
            TestFalse(TEXT("panel key non-empty"), PanelLocKey(P).IsEmpty());
        }
    }
    return true;
}

#endif
