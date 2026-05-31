#include "CharacterSystem/CharacterAnimState.h"

namespace IdleProject::Character
{
    ECharAnimState SelectAnimState(ECharAnimState Current, bool bDead, bool bOneShotPlaying,
        bool bAttackRequested, bool bHitRequested, bool bMoving)
    {
        // 1. 사망은 최우선 — 모든 다른 플래그를 무시
        if (bDead)            { return ECharAnimState::Death; }
        // 2. 새 공격 요청 → 즉시 Attack 전환(원샷 시작)
        if (bAttackRequested) { return ECharAnimState::Attack; }
        // 3. 피격 요청
        if (bHitRequested)    { return ECharAnimState::Hit; }
        // 4. 원샷(Attack/Hit) 클립 재생 중 — 완료될 때까지 현재 상태 유지
        if (bOneShotPlaying)  { return Current; }
        // 5. 로코모션: 이동 또는 대기
        return bMoving ? ECharAnimState::Move : ECharAnimState::Idle;
    }
}
