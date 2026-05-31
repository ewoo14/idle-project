#include "Misc/AutomationTest.h"
#include "CharacterSystem/CharacterFaceMotion.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharFaceMotionTest, "IdleProject.Character.FaceMotion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharFaceMotionTest::RunTest(const FString& Parameters)
{
    using namespace IdleProject::Character;
    // 깜빡임: 중간(0.5*dur)=완전 감음(1), 시작/끝=0, 범위 밖=0
    TestTrue(TEXT("blink mid ~1"), FMath::IsNearlyEqual(ComputeBlinkWeight(0.06f, 0.12f), 1.0f, 0.01f));
    TestTrue(TEXT("blink start ~0"), FMath::IsNearlyEqual(ComputeBlinkWeight(0.0f, 0.12f), 0.0f, 0.01f));
    TestTrue(TEXT("blink end ~0"), FMath::IsNearlyEqual(ComputeBlinkWeight(0.12f, 0.12f), 0.0f, 0.01f));
    TestEqual(TEXT("blink past = 0"), ComputeBlinkWeight(0.2f, 0.12f), 0.0f);
    // 입벌림: 상승은 Attack 속도로 증가
    TestTrue(TEXT("mouth rising"), ComputeMouthOpen(1.0f, 0.0f, 0.1f, 5.0f, 10.0f) > 0.0f);
    TestTrue(TEXT("mouth rise rate"), FMath::IsNearlyEqual(ComputeMouthOpen(1.0f, 0.0f, 0.1f, 5.0f, 10.0f), 0.5f, 0.001f)); // 0 + 5*0.1
    // 하강은 Release 속도
    TestTrue(TEXT("mouth falling"), ComputeMouthOpen(0.0f, 1.0f, 0.1f, 5.0f, 10.0f) < 1.0f);
    // 클램프 0..1 (큰 dt)
    TestTrue(TEXT("mouth clamp hi"), ComputeMouthOpen(1.0f, 0.9f, 10.0f, 5.0f, 10.0f) <= 1.0f);
    TestTrue(TEXT("mouth clamp lo"), ComputeMouthOpen(0.0f, 0.1f, 10.0f, 5.0f, 10.0f) >= 0.0f);
    return true;
}

#endif
