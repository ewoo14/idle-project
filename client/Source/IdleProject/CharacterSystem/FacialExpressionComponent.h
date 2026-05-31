#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FacialExpressionComponent.generated.h"

class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EFacialExpression : uint8
{
	None = 0 UMETA(Hidden),
	Idle = 1 UMETA(DisplayName = "Idle"),
	Battle = 2 UMETA(DisplayName = "Battle"),
	Smile = 3 UMETA(DisplayName = "Smile"),
	Hit = 4 UMETA(DisplayName = "Hit"),
	Death = 5 UMETA(DisplayName = "Death"),
	LevelUp = 6 UMETA(DisplayName = "LevelUp")
};

/** VRoid/VRM4U Morph Target 이름에 맞춰 캐릭터 표정을 제어합니다. */
UCLASS(ClassGroup = (Idle), meta = (BlueprintSpawnableComponent))
class IDLEPROJECT_API UFacialExpressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFacialExpressionComponent();

	UFUNCTION(BlueprintCallable, Category = "Idle|Facial")
	void SetExpression(EFacialExpression NewExpression, float Duration = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Idle|Facial")
	EFacialExpression GetCurrentExpression() const { return CurrentExpression; }

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Idle|Facial")
	EFacialExpression CurrentExpression = EFacialExpression::Idle;

	FTimerHandle RevertTimerHandle;

	// --- 자동 깜빡임 ---
	void UpdateBlink(float DeltaTime);
	float BlinkTimer = 0.0f;          // 다음 깜빡임까지 누적 시간
	float NextBlinkInterval = 3.0f;   // 랜덤 2~6s 간격
	bool bBlinking = false;           // 현재 깜빡임 중 여부
	float BlinkElapsed = 0.0f;        // 깜빡임 시작 후 경과 시간
	static constexpr float BlinkDuration = 0.12f; // 눈 감는 데 걸리는 초

	USkeletalMeshComponent* GetTargetMesh() const;
	void ApplyBlendShapes(EFacialExpression Expression);
	void RevertToIdle();
};
