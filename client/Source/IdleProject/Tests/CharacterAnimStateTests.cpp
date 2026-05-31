#include "Misc/AutomationTest.h"
#include "CharacterSystem/CharacterAnimState.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCharAnimStateTest, "IdleProject.Character.AnimStateSelect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCharAnimStateTest::RunTest(const FString& Parameters)
{
    using namespace IdleProject::Character;
    auto S = [](ECharAnimState e){ return static_cast<int32>(e); };
    // 사망 최우선
    TestEqual(TEXT("dead wins"), S(SelectAnimState(ECharAnimState::Move, true, true, true, true, true)), S(ECharAnimState::Death));
    // 새 공격 요청 즉시 전환
    TestEqual(TEXT("attack req"), S(SelectAnimState(ECharAnimState::Idle, false, false, true, false, false)), S(ECharAnimState::Attack));
    // 피격 요청
    TestEqual(TEXT("hit req"), S(SelectAnimState(ECharAnimState::Idle, false, false, false, true, false)), S(ECharAnimState::Hit));
    // 원샷 재생 중(요청 없음) → 현재 유지
    TestEqual(TEXT("oneshot hold"), S(SelectAnimState(ECharAnimState::Attack, false, true, false, false, true)), S(ECharAnimState::Attack));
    // 이동
    TestEqual(TEXT("move"), S(SelectAnimState(ECharAnimState::Idle, false, false, false, false, true)), S(ECharAnimState::Move));
    // 대기
    TestEqual(TEXT("idle"), S(SelectAnimState(ECharAnimState::Move, false, false, false, false, false)), S(ECharAnimState::Idle));
    return true;
}

#endif
