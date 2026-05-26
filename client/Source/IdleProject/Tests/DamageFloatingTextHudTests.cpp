#include "Misc/AutomationTest.h"

#include "CombatSystem/CombatComponent.h"
#include "UI/IdleHUD.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDamageFloatingTextHudViewModelTest,
	"IdleProject.UI.HUD.DamageFloatingTextViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDamageFloatingTextHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	FIdleHUDFloatingDamageEntry PhysicalEntry;
	PhysicalEntry.Amount = 1234.4f;
	PhysicalEntry.Kind = EDamageKind::Physical;
	PhysicalEntry.StartTime = 10.0f;
	PhysicalEntry.WorldLocation = FVector(20.0f, 30.0f, 110.0f);

	const FIdleHUDFloatingDamageViewModel PhysicalView = BuildFloatingDamageViewModel(
		PhysicalEntry,
		11.0f,
		FVector2D(960.0f, 420.0f),
		1.0f);
	TestTrue(TEXT("Physical damage remains visible at one second"), PhysicalView.bVisible);
	TestEqual(TEXT("Damage amount is rounded for display"), PhysicalView.Label, FString(TEXT("1,234")));
	TestEqual(TEXT("Physical damage uses primary text red channel"), PhysicalView.Color.R, TextPrimary.R);
	TestEqual(TEXT("Physical damage uses primary text green channel"), PhysicalView.Color.G, TextPrimary.G);
	TestEqual(TEXT("Physical damage uses primary text blue channel"), PhysicalView.Color.B, TextPrimary.B);
	TestEqual(TEXT("Damage floats upward over lifetime"), static_cast<float>(PhysicalView.ScreenPosition.Y), 388.0f);
	TestEqual(TEXT("Damage fades over lifetime"), PhysicalView.Color.A, 0.0f);

	FIdleHUDFloatingDamageEntry MagicEntry;
	MagicEntry.Amount = 98.6f;
	MagicEntry.Kind = EDamageKind::Magic;
	MagicEntry.StartTime = 20.0f;

	const FIdleHUDFloatingDamageViewModel MagicView = BuildFloatingDamageViewModel(
		MagicEntry,
		20.35f,
		FVector2D(640.0f, 360.0f),
		1.5f);
	TestTrue(TEXT("Magic damage remains visible during pop"), MagicView.bVisible);
	TestEqual(TEXT("Magic damage uses rounded amount"), MagicView.Label, FString(TEXT("99")));
	TestEqual(TEXT("Magic damage uses token blue"), MagicView.Color.R, AccentBlue.R);
	TestEqual(TEXT("Magic damage uses token blue"), MagicView.Color.G, AccentBlue.G);
	TestEqual(TEXT("Magic damage uses token blue"), MagicView.Color.B, AccentBlue.B);
	TestEqual(TEXT("Scaled damage float uses token motion distance"), static_cast<float>(MagicView.ScreenPosition.Y), 343.2f);

	FIdleHUDFloatingDamageEntry CritEntry;
	CritEntry.Amount = 42.0f;
	CritEntry.bWasCrit = true;
	CritEntry.Kind = EDamageKind::Physical;
	CritEntry.StartTime = 30.0f;

	const FIdleHUDFloatingDamageViewModel CritView = BuildFloatingDamageViewModel(
		CritEntry,
		30.10f,
		FVector2D(500.0f, 240.0f),
		1.0f);
	TestEqual(TEXT("Critical damage adds emphasis mark"), CritView.Label, FString(TEXT("42!")));
	TestEqual(TEXT("Critical damage uses token gold"), CritView.Color.R, AccentGold.R);
	TestEqual(TEXT("Critical damage uses token gold"), CritView.Color.G, AccentGold.G);
	TestEqual(TEXT("Critical damage uses token gold"), CritView.Color.B, AccentGold.B);
	TestEqual(TEXT("Critical damage gets larger scale"), CritView.TextScale, 1.25f);

	const FIdleHUDFloatingDamageViewModel ExpiredView = BuildFloatingDamageViewModel(
		CritEntry,
		31.25f,
		FVector2D(500.0f, 240.0f),
		1.0f);
	TestFalse(TEXT("Damage text expires after lifetime"), ExpiredView.bVisible);

	return true;
}

#endif
