#pragma once
#include "CoreMinimal.h"

namespace IdleProject::Character
{
    // 캐릭터 애님 상태 열거형. 우선순위: 사망 > 공격/피격 요청 > 원샷 유지 > 이동/대기.
    enum class ECharAnimState : uint8 { Idle, Move, Attack, Hit, Death };

    /**
     * 전투 플래그로부터 원하는 애님 상태를 선택하는 순수 로직 함수.
     * 렌더링/UObject 의존성 없음 — 단위 테스트 가능.
     *
     * 우선순위:
     *   1. bDead           → Death   (최우선, 모든 플래그 무시)
     *   2. bAttackRequested → Attack  (새 공격 요청 즉시 전환, 원샷 시작)
     *   3. bHitRequested   → Hit
     *   4. bOneShotPlaying  → Current (Attack/Hit 원샷 재생 중이면 현재 상태 유지)
     *   5. bMoving         → Move
     *   6. 그 외           → Idle
     */
    ECharAnimState SelectAnimState(ECharAnimState Current, bool bDead, bool bOneShotPlaying,
        bool bAttackRequested, bool bHitRequested, bool bMoving);
}
