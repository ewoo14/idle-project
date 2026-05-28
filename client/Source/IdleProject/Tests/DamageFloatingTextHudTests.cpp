#include "Misc/AutomationTest.h"

#include "CombatSystem/CombatComponent.h"
#include "GameCore/StageService.h"
#include "Internationalization/IdleLocalization.h"
#include "UI/IdleHUD.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageIndicatorHudViewModelTest,
	"IdleProject.UI.HUD.StageIndicatorViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageIndicatorHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	FStageInfo NormalStage;
	NormalStage.Chapter = 1;
	NormalStage.Stage = 3;
	NormalStage.KillsThisStage = 7;
	NormalStage.KillsToAdvance = 10;
	NormalStage.bBossStage = false;
	NormalStage.WeakElement = ESkillElement::Ice;

	const FIdleHUDStageViewModel NormalView = BuildStageViewModel(NormalStage);
	TestEqual(TEXT("Stage title is localized"), NormalView.TitleLabel.ToString(), FString(TEXT("Stage")));
	TestEqual(TEXT("Chapter label is localized separately"), NormalView.ChapterLabel.ToString(), FString(TEXT("Chapter 1")));
	TestEqual(TEXT("Stage progress includes stage and kills"), NormalView.ProgressLabel.ToString(), FString(TEXT("Stage 1-3 7/10")));
	TestEqual(TEXT("Normal stage omits boss badge"), NormalView.BossBadgeLabel.ToString(), FString());
	TestEqual(TEXT("Weak element is localized"), NormalView.WeaknessLabel.ToString(), FString(TEXT("Weak: Ice")));
	TestEqual(TEXT("Progress ratio is clamped from kills"), NormalView.ProgressRatio, 0.7f);
	TestFalse(TEXT("Normal stage is not highlighted as boss"), NormalView.bBossStage);
	TestEqual(TEXT("Weakness uses ice blue token"), NormalView.WeaknessColor.B, AccentBlue.B);

	FStageInfo BossStage = NormalStage;
	BossStage.Stage = 5;
	BossStage.KillsThisStage = 0;
	BossStage.KillsToAdvance = 1;
	BossStage.bBossStage = true;
	BossStage.WeakElement = ESkillElement::Fire;

	const FIdleHUDStageViewModel BossView = BuildStageViewModel(BossStage);
	TestEqual(TEXT("Boss stage badge is localized"), BossView.BossBadgeLabel.ToString(), FString(TEXT("Boss")));
	TestEqual(TEXT("Boss stage uses gold border color"), BossView.BorderColor.R, AccentGold.R);
	TestEqual(TEXT("Fire weakness uses danger token"), BossView.WeaknessColor.R, AccentRed.R);

	FStageInfo InvalidTargetStage = NormalStage;
	InvalidTargetStage.KillsThisStage = 4;
	InvalidTargetStage.KillsToAdvance = 0;

	const FIdleHUDStageViewModel InvalidTargetView = BuildStageViewModel(InvalidTargetStage);
	TestEqual(TEXT("Zero target is shown explicitly"), InvalidTargetView.ProgressLabel.ToString(), FString(TEXT("Stage 1-3 4/0")));
	TestEqual(TEXT("Zero target produces empty progress ratio"), InvalidTargetView.ProgressRatio, 0.0f);

	FStageInfo OverflowStage = NormalStage;
	OverflowStage.KillsThisStage = 14;
	OverflowStage.KillsToAdvance = 10;

	const FIdleHUDStageViewModel OverflowView = BuildStageViewModel(OverflowStage);
	TestEqual(TEXT("Overflow stage progress is displayed as reported"), OverflowView.ProgressLabel.ToString(), FString(TEXT("Stage 1-3 14/10")));
	TestEqual(TEXT("Overflow stage progress ratio is clamped"), OverflowView.ProgressRatio, 1.0f);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageChapterFeedbackHudViewModelTest,
	"IdleProject.UI.HUD.StageChapterFeedbackViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageChapterFeedbackHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;

	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	FStageInfo ChapterTwoStage;
	ChapterTwoStage.Chapter = 2;
	ChapterTwoStage.Stage = 3;
	ChapterTwoStage.KillsThisStage = 6;
	ChapterTwoStage.KillsToAdvance = 12;
	ChapterTwoStage.WeakElement = ESkillElement::Ice;

	const FIdleHUDStageViewModel ChapterTwoView = BuildStageViewModel(ChapterTwoStage);
	TestEqual(TEXT("Chapter two label is localized"), ChapterTwoView.ChapterLabel.ToString(), FString(TEXT("Chapter 2")));
	TestEqual(TEXT("Chapter two progress preserves stage coordinate"), ChapterTwoView.ProgressLabel.ToString(), FString(TEXT("Stage 2-3 6/12")));

	FStageInfo LightningStage = ChapterTwoStage;
	LightningStage.Stage = 2;
	LightningStage.WeakElement = ESkillElement::Lightning;
	const FIdleHUDStageViewModel LightningView = BuildStageViewModel(LightningStage);
	TestEqual(TEXT("Lightning weakness is localized"), LightningView.WeaknessLabel.ToString(), FString(TEXT("Weak: Lightning")));

	TestEqual(TEXT("Chapter entry feedback is localized"), BuildChapterEntryFeedbackLabel(2).ToString(), FString(TEXT("Chapter 2 Enter")));
	TestEqual(TEXT("Chapter clear feedback is localized"), BuildChapterClearFeedbackLabel(2).ToString(), FString(TEXT("Chapter 2 Cleared")));

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FChapterThreeEliteDarkStageHudViewModelTest,
	"IdleProject.UI.HUD.ChapterThreeEliteDarkStageViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FChapterThreeEliteDarkStageHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	FStageInfo ChapterThreeElite;
	ChapterThreeElite.Chapter = 3;
	ChapterThreeElite.Stage = 5;
	ChapterThreeElite.KillsThisStage = 0;
	ChapterThreeElite.KillsToAdvance = 1;
	ChapterThreeElite.bEliteStage = true;
	ChapterThreeElite.WeakElement = ESkillElement::Dark;

	const FIdleHUDStageViewModel EliteView = BuildStageViewModel(ChapterThreeElite);
	TestEqual(TEXT("Chapter three label is localized"), EliteView.ChapterLabel.ToString(), FString(TEXT("Chapter 3")));
	TestEqual(TEXT("Chapter three progress preserves 10-stage coordinates"), EliteView.ProgressLabel.ToString(), FString(TEXT("Stage 3-5 0/1")));
	TestEqual(TEXT("Elite badge is localized"), EliteView.EliteBadgeLabel.ToString(), FString(TEXT("Elite")));
	TestEqual(TEXT("Dark weakness is localized"), EliteView.WeaknessLabel.ToString(), FString(TEXT("Weak: Dark")));
	TestEqual(TEXT("Dark icon uses compact elemental glyph"), EliteView.WeaknessIconLabel.ToString(), FString(TEXT("D")));
	TestTrue(TEXT("Elite stage is exposed separately from boss"), EliteView.bEliteStage);
	TestFalse(TEXT("Elite stage is not a boss stage"), EliteView.bBossStage);
	TestEqual(TEXT("Elite stage uses dark border color"), EliteView.BorderColor.R, ElementDark.R);
	TestEqual(TEXT("Dark weakness uses dark element token"), EliteView.WeaknessColor.B, ElementDark.B);

	FStageInfo ChapterThreeBoss = ChapterThreeElite;
	ChapterThreeBoss.Stage = 10;
	ChapterThreeBoss.KillsThisStage = 1;
	ChapterThreeBoss.bEliteStage = false;
	ChapterThreeBoss.bBossStage = true;
	ChapterThreeBoss.WeakElement = ESkillElement::Holy;

	const FIdleHUDStageViewModel BossView = BuildStageViewModel(ChapterThreeBoss);
	TestEqual(TEXT("Chapter three boss progress keeps X-10 coordinate"), BossView.ProgressLabel.ToString(), FString(TEXT("Stage 3-10 1/1")));
	TestEqual(TEXT("Boss badge remains localized"), BossView.BossBadgeLabel.ToString(), FString(TEXT("Boss")));
	TestEqual(TEXT("Boss stage does not show elite badge"), BossView.EliteBadgeLabel.ToString(), FString());

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FBossHudViewModelTest,
	"IdleProject.UI.HUD.BossViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FBossHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	IdleProject::Localization::SetLanguageForTests(TEXT("en"));

	const FIdleHUDBossViewModel PhaseTwoView = BuildBossViewModel(250.0f, 500.0f);
	TestTrue(TEXT("Boss HUD is visible for a valid boss health pool"), PhaseTwoView.bVisible);
	TestEqual(TEXT("Boss HP ratio is normalized"), PhaseTwoView.HpRatio, 0.5f);
	TestEqual(TEXT("Boss HP percent is exposed"), PhaseTwoView.HpPercent, 50.0f);
	TestEqual(TEXT("Boss phase mirrors formula"), PhaseTwoView.Phase, 2);
	TestEqual(TEXT("Boss title is localized"), PhaseTwoView.TitleLabel.ToString(), FString(TEXT("Boss")));
	TestEqual(TEXT("Boss HP label is localized"), PhaseTwoView.HpLabel.ToString(), FString(TEXT("HP 250 / 500")));
	TestEqual(TEXT("Boss phase label is localized"), PhaseTwoView.PhaseLabel.ToString(), FString(TEXT("Phase 2")));
	TestEqual(TEXT("Phase 2 uses warning color"), PhaseTwoView.PhaseColor.G, Warn.G);

	const FIdleHUDBossViewModel PhaseTwoBoundaryView = BuildBossViewModel(330.0f, 500.0f);
	TestEqual(TEXT("Boss HUD mirrors 0.66 boundary as phase 2"), PhaseTwoBoundaryView.Phase, 2);

	const FIdleHUDBossViewModel PhaseThreeBoundaryView = BuildBossViewModel(165.0f, 500.0f);
	TestEqual(TEXT("Boss HUD mirrors 0.33 boundary as phase 3"), PhaseThreeBoundaryView.Phase, 3);

	const FIdleHUDBossViewModel PhaseThreeView = BuildBossViewModel(0.0f, 500.0f);
	TestEqual(TEXT("Zero HP remains phase 3"), PhaseThreeView.Phase, 3);
	TestEqual(TEXT("Empty HP bar is allowed for a defeated boss frame"), PhaseThreeView.HpRatio, 0.0f);
	TestEqual(TEXT("Phase 3 uses danger color"), PhaseThreeView.PhaseColor.R, AccentRed.R);

	const FIdleHUDBossViewModel InvalidView = BuildBossViewModel(10.0f, 0.0f);
	TestFalse(TEXT("Boss HUD is hidden for invalid max HP"), InvalidView.bVisible);

	IdleProject::Localization::SetLanguageForTests(TEXT("ko"));
	return true;
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStatusIndicatorHudViewModelTest,
	"IdleProject.UI.HUD.StatusIndicatorViewModel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStatusIndicatorHudViewModelTest::RunTest(const FString& Parameters)
{
	using namespace IdleProject::UI;
	using namespace IdleProject::UI::Theme;

	TArray<FActiveSkillStatus> Statuses;
	FActiveSkillStatus Burn;
	Burn.Type = ESkillStatusEffect::Burn;
	Burn.EndTime = 15.0f;
	Statuses.Add(Burn);

	FActiveSkillStatus Freeze;
	Freeze.Type = ESkillStatusEffect::Freeze;
	Freeze.EndTime = 13.5f;
	Statuses.Add(Freeze);

	FActiveSkillStatus Poison;
	Poison.Type = ESkillStatusEffect::Poison;
	Poison.EndTime = 14.0f;
	Statuses.Add(Poison);

	const TArray<FIdleHUDStatusIndicatorViewModel> Indicators = BuildStatusIndicatorViewModels(Statuses, 12.0f, 1.0f);

	TestEqual(TEXT("HUD shows one indicator per active status"), Indicators.Num(), 3);
	TestEqual(TEXT("Poison is shown first for stable horizontal layout"), static_cast<int32>(Indicators[0].Type), static_cast<int32>(ESkillStatusEffect::Poison));
	TestEqual(TEXT("Poison uses a compact glyph"), Indicators[0].Label, FString(TEXT("P")));
	TestEqual(TEXT("Poison uses green rarity token"), Indicators[0].Color.R, RarityUnique.R);
	TestEqual(TEXT("Poison remaining time is exposed"), Indicators[0].RemainingSeconds, 2.0f);

	TestEqual(TEXT("Burn is shown second for stable horizontal layout"), static_cast<int32>(Indicators[1].Type), static_cast<int32>(ESkillStatusEffect::Burn));
	TestEqual(TEXT("Burn uses a compact glyph"), Indicators[1].Label, FString(TEXT("B")));
	TestEqual(TEXT("Burn uses legendary orange token"), Indicators[1].Color.G, RarityLegendary.G);

	TestEqual(TEXT("Freeze is shown third for stable horizontal layout"), static_cast<int32>(Indicators[2].Type), static_cast<int32>(ESkillStatusEffect::Freeze));
	TestEqual(TEXT("Freeze uses a compact glyph"), Indicators[2].Label, FString(TEXT("F")));
	TestEqual(TEXT("Freeze uses blue token"), Indicators[2].Color.B, AccentBlue.B);

	FActiveSkillStatus ExpiredPoison;
	ExpiredPoison.Type = ESkillStatusEffect::Poison;
	ExpiredPoison.EndTime = 11.5f;
	const TArray<FIdleHUDStatusIndicatorViewModel> ExpiredIndicators = BuildStatusIndicatorViewModels({ExpiredPoison}, 12.0f, 1.0f);
	TestTrue(TEXT("Expired statuses are hidden"), ExpiredIndicators.IsEmpty());

	return true;
}

#endif
