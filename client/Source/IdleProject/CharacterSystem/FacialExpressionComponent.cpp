#include "CharacterSystem/FacialExpressionComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "CharacterSystem/CharacterFaceMotion.h"

UFacialExpressionComponent::UFacialExpressionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFacialExpressionComponent::BeginPlay()
{
	Super::BeginPlay();
	// 첫 깜빡임 간격을 2~6s 사이 랜덤 초기화
	NextBlinkInterval = FMath::RandRange(2.0f, 6.0f);
}

void UFacialExpressionComponent::SetExpression(EFacialExpression NewExpression, float Duration)
{
	if (NewExpression == EFacialExpression::None)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RevertTimerHandle);
	}

	CurrentExpression = NewExpression;
	ApplyBlendShapes(CurrentExpression);

	UE_LOG(LogTemp, Display, TEXT("[FacialExpression] %s"), *StaticEnum<EFacialExpression>()->GetNameStringByValue(static_cast<int64>(CurrentExpression)));

	if (Duration > 0.0f)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(RevertTimerHandle, this, &UFacialExpressionComponent::RevertToIdle, Duration, false);
		}
	}
}

void UFacialExpressionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateBlink(DeltaTime);
}

void UFacialExpressionComponent::UpdateBlink(float DeltaTime)
{
	USkeletalMeshComponent* Mesh = GetTargetMesh();
	if (!Mesh) { return; }

	if (bBlinking)
	{
		BlinkElapsed += DeltaTime;
		const float W = IdleProject::Character::ComputeBlinkWeight(BlinkElapsed, BlinkDuration);
		Mesh->SetMorphTarget(TEXT("Blink"),   W);
		Mesh->SetMorphTarget(TEXT("Blink_L"), W); // 있으면 적용, 없으면 무동작
		Mesh->SetMorphTarget(TEXT("Blink_R"), W);
		if (BlinkElapsed >= BlinkDuration)
		{
			// 깜빡임 종료 — 모프 초기화 및 다음 간격 설정
			bBlinking = false;
			Mesh->SetMorphTarget(TEXT("Blink"),   0.0f);
			Mesh->SetMorphTarget(TEXT("Blink_L"), 0.0f);
			Mesh->SetMorphTarget(TEXT("Blink_R"), 0.0f);
			BlinkTimer = 0.0f;
			NextBlinkInterval = FMath::RandRange(2.0f, 6.0f);
		}
	}
	else
	{
		BlinkTimer += DeltaTime;
		if (BlinkTimer >= NextBlinkInterval)
		{
			bBlinking = true;
			BlinkElapsed = 0.0f;
		}
	}
}

USkeletalMeshComponent* UFacialExpressionComponent::GetTargetMesh() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	if (!Mesh || !Mesh->IsVisible())
	{
		return nullptr;
	}

	return Mesh;
}

void UFacialExpressionComponent::ApplyBlendShapes(EFacialExpression Expression)
{
	USkeletalMeshComponent* Mesh = GetTargetMesh();
	if (!Mesh)
	{
		return;
	}

	static const TArray<FName> KnownBlendShapeNames = {
		TEXT("Joy"),
		TEXT("Angry"),
		TEXT("Sorrow"),
		TEXT("Fun"),
		TEXT("Surprised"),
	};

	for (const FName& BlendShapeName : KnownBlendShapeNames)
	{
		Mesh->SetMorphTarget(BlendShapeName, 0.0f);
	}

	static const TMap<EFacialExpression, TMap<FName, float>> ExpressionMap = {
		{ EFacialExpression::Idle, {} },
		{ EFacialExpression::Battle, { { TEXT("Angry"), 0.7f } } },
		{ EFacialExpression::Smile, { { TEXT("Joy"), 1.0f }, { TEXT("Fun"), 0.5f } } },
		{ EFacialExpression::Hit, { { TEXT("Sorrow"), 0.8f }, { TEXT("Angry"), 0.3f } } },
		{ EFacialExpression::Death, { { TEXT("Sorrow"), 1.0f } } },
		{ EFacialExpression::LevelUp, { { TEXT("Joy"), 1.0f }, { TEXT("Surprised"), 0.8f } } },
	};

	const TMap<FName, float>* BlendShapes = ExpressionMap.Find(Expression);
	if (!BlendShapes)
	{
		return;
	}

	for (const TPair<FName, float>& BlendShape : *BlendShapes)
	{
		Mesh->SetMorphTarget(BlendShape.Key, BlendShape.Value);
	}
}

void UFacialExpressionComponent::RevertToIdle()
{
	SetExpression(EFacialExpression::Idle);
}
