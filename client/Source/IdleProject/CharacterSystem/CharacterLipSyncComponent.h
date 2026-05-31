#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterLipSyncComponent.generated.h"

class USkeletalMeshComponent;

/** VRM 입모양(A/I/U/E/O) 모프를 음성 진폭으로 구동하는 립싱크 컴포넌트. */
UCLASS(ClassGroup=(Idle), meta=(BlueprintSpawnableComponent))
class IDLEPROJECT_API UCharacterLipSyncComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UCharacterLipSyncComponent();

    // 추후 TTS가 매 프레임 0..1 진폭을 전달. (없으면 0 → 입 닫힘)
    UFUNCTION(BlueprintCallable, Category="Idle|LipSync")
    void SetSpeechAmplitude(float Amplitude01);

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    // true면 사인파로 자체 진폭 생성(에셋/TTS 없이 입 움직임 시연·검증용).
    UPROPERTY(EditAnywhere, Category="Idle|LipSync")
    bool bDebugOscillate = false;

    UPROPERTY(EditAnywhere, Category="Idle|LipSync")
    float AttackPerSec = 12.0f;
    UPROPERTY(EditAnywhere, Category="Idle|LipSync")
    float ReleasePerSec = 8.0f;

private:
    USkeletalMeshComponent* GetTargetMesh() const;

    float TargetAmplitude = 0.0f; // 외부 입력 캐시
    float MouthOpen = 0.0f;       // 현재 평활값
    float DebugTime = 0.0f;
};
