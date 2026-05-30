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

#endif
