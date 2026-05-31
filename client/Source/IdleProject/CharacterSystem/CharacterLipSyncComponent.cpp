#include "CharacterSystem/CharacterLipSyncComponent.h"
#include "CharacterSystem/CharacterFaceMotion.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UCharacterLipSyncComponent::UCharacterLipSyncComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterLipSyncComponent::SetSpeechAmplitude(float Amplitude01)
{
    TargetAmplitude = FMath::Clamp(Amplitude01, 0.0f, 1.0f);
}

USkeletalMeshComponent* UCharacterLipSyncComponent::GetTargetMesh() const
{
    const AActor* Owner = GetOwner();
    return Owner ? Owner->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}

void UCharacterLipSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float Target = TargetAmplitude;
    if (bDebugOscillate)
    {
        DebugTime += DeltaTime;
        // 0..1 사인(약 3Hz) — 말하는 느낌
        Target = 0.5f + 0.5f * FMath::Sin(DebugTime * 3.0f * 2.0f * PI);
    }

    MouthOpen = IdleProject::Character::ComputeMouthOpen(Target, MouthOpen, DeltaTime, AttackPerSec, ReleasePerSec);

    if (USkeletalMeshComponent* Mesh = GetTargetMesh())
    {
        // 턱 벌림 주도(추후 음소별 A/I/U/E/O 분배로 확장). 모프 없으면 무동작.
        Mesh->SetMorphTarget(TEXT("A"), MouthOpen);
    }
}
