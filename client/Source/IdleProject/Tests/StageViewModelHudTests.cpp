#include "Misc/AutomationTest.h"

#include "UI/IdleHUD.h"
#include "GameCore/StageService.h"
#include "CombatSystem/StatusElementTypes.h" // ESkillElement

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageViewModelResistTest,
	"IdleProject.UI.HUD.StageViewModelResist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageViewModelResistTest::RunTest(const FString& Parameters)
{
	// ch9 스테이지: 저항 설정 → bHasResist + 라벨/아이콘 non-empty.
	FStageInfo Ch9;
	Ch9.WeakElement = ESkillElement::Dark;
	Ch9.ResistElement = ESkillElement::Ice;
	const FIdleHUDStageViewModel VmCh9 = IdleProject::UI::BuildStageViewModel(Ch9);
	TestTrue(TEXT("ch9 has resist"), VmCh9.bHasResist);
	TestFalse(TEXT("ch9 resist label non-empty"), VmCh9.ResistLabel.IsEmpty());
	TestFalse(TEXT("ch9 resist icon non-empty"), VmCh9.ResistIconLabel.IsEmpty());

	// 이전 챕터: 저항 None → bHasResist false.
	FStageInfo Early;
	Early.WeakElement = ESkillElement::Fire;
	Early.ResistElement = ESkillElement::None;
	const FIdleHUDStageViewModel VmEarly = IdleProject::UI::BuildStageViewModel(Early);
	TestFalse(TEXT("early no resist"), VmEarly.bHasResist);
	return true;
}

#endif
