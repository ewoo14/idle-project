#pragma once
#include "CoreMinimal.h"

namespace IdleProject::Character
{
    // 깜빡임 가중치: Elapsed(깜빡임 시작 후 경과초) → 0..1(1=완전 감음). 삼각 0→1→0, BlinkDuration에 걸쳐.
    // Elapsed<=0 또는 >=BlinkDuration → 0.
    float ComputeBlinkWeight(float Elapsed, float BlinkDuration);

    // 입 벌림 평활: Target(0..1) 향해 Prev에서 접근. 상승=AttackPerSec, 하강=ReleasePerSec 속도. 0..1 클램프.
    float ComputeMouthOpen(float Target, float Prev, float DeltaSeconds, float AttackPerSec, float ReleasePerSec);
}
