#include "ItemSystem/EquipmentDrop.h"

#include "CharacterSystem/IdleCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "ItemSystem/InventoryComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UI/UIThemeTokens.h"
#include "UObject/ConstructorHelpers.h"

AEquipmentDrop::AEquipmentDrop()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRelativeScale3D(FVector(0.28f, 0.28f, 0.28f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		Mesh->SetMaterial(0, BasicMaterial.Object);
	}
}

void AEquipmentDrop::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* DynamicMaterial = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), GetRarityColor());
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PickupDelayHandle, this, &AEquipmentDrop::EnablePickup, 0.5f, false);
	}
}

void AEquipmentDrop::Tick(float DeltaSeconds)
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
		if (UInventoryComponent* Inventory = TargetCharacter->FindComponentByClass<UInventoryComponent>())
		{
			Inventory->AddItem(Payload);
		}
		Destroy();
	}
}

void AEquipmentDrop::EnablePickup()
{
	bPickupEnabled = true;
}

AActor* AEquipmentDrop::FindClosestCharacter() const
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

FLinearColor AEquipmentDrop::GetRarityColor() const
{
	using namespace IdleProject::UI::Theme;

	switch (Payload.Rarity)
	{
	case EItemRarity::Uncommon:
		return RarityUncommon;
	case EItemRarity::Rare:
		return RarityRare;
	case EItemRarity::Common:
	default:
		return RarityCommon;
	}
}
