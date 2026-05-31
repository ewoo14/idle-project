#include "CharacterSystem/CharacterFaceMotion.h"

namespace IdleProject::Character
{
    float ComputeBlinkWeight(float Elapsed, float BlinkDuration)
    {
        if (BlinkDuration <= 0.0f || Elapsed <= 0.0f || Elapsed >= BlinkDuration)
        {
            return 0.0f;
        }
        const float t = Elapsed / BlinkDuration;            // 0..1
        return 1.0f - FMath::Abs(2.0f * t - 1.0f);          // 삼각: 중간 피크 1
    }

    float ComputeMouthOpen(float Target, float Prev, float DeltaSeconds, float AttackPerSec, float ReleasePerSec)
    {
        const float Rate = (Target > Prev) ? AttackPerSec : ReleasePerSec;
        const float Step = Rate * DeltaSeconds;
        float Next;
        if (Target > Prev) { Next = FMath::Min(Prev + Step, Target); }
        else               { Next = FMath::Max(Prev - Step, Target); }
        return FMath::Clamp(Next, 0.0f, 1.0f);
    }
}
