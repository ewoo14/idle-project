#include "ItemSystem/GoldDrop.h"

#include "CharacterSystem/IdleCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameCore/IdleGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AGoldDrop::AGoldDrop()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.25f, 0.25f, 0.25f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Mesh->SetStaticMesh(SphereMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AGoldDrop::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(1.0f, 0.72f, 0.05f));
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PickupDelayHandle, this, &AGoldDrop::EnablePickup, 0.5f, false);
	}
}

void AGoldDrop::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPickupEnabled)
	{
		return;
	}

	AActor* TargetCharacter = FindClosestCharacter();
	if (!TargetCharacter)
	{
		return;
	}

	const FVector TargetLocation = TargetCharacter->GetActorLocation();
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaSeconds, 8.0f));

	if (FVector::Dist(GetActorLocation(), TargetLocation) <= 50.0f)
	{
		if (UIdleGameInstance* GameInstance = Cast<UIdleGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GameInstance->AddGold(Amount);
		}
		Destroy();
	}
}

void AGoldDrop::EnablePickup()
{
	bPickupEnabled = true;
}

AActor* AGoldDrop::FindClosestCharacter() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AActor* ClosestActor = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AIdleCharacter> It(World); It; ++It)
	{
		AIdleCharacter* Candidate = *It;
		if (!IsValid(Candidate))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestActor = Candidate;
		}
	}

	return ClosestActor;
}
