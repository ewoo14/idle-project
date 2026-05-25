#include "CharacterSystem/IdleMonster.h"

#include "CharacterSystem/IdleCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ItemSystem/EquipmentDrop.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/GoldDrop.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AIdleMonster::AIdleMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 44.0f);

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -34.0f));
	PlaceholderMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(SphereMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMaterial.Succeeded())
	{
		PlaceholderMesh->SetMaterial(0, BasicMaterial.Object);
	}

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	BattleAI = CreateDefaultSubobject<UBattleAIComponent>(TEXT("BattleAI"));

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 220.0f;
	Movement->bOrientRotationToMovement = false;
	Movement->bConstrainToPlane = true;
	Movement->SetPlaneConstraintNormal(FVector::YAxisVector);
}

void AIdleMonster::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* DynamicMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.8f, 0.2f));
	}

	if (Combat)
	{
		Combat->InitializeCombat(50.0f, 8.0f, 5.0f, 1.0f);
		Combat->OnDeath.AddDynamic(this, &AIdleMonster::HandleDeath);
	}

	if (BattleAI)
	{
		BattleAI->TargetActorClass = AIdleCharacter::StaticClass();
		BattleAI->StartBattle();
	}
}

void AIdleMonster::HandleDeath(AActor* DyingActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGoldDrop* GoldDrop = World->SpawnActor<AGoldDrop>(AGoldDrop::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
	if (GoldDrop)
	{
		GoldDrop->Amount = static_cast<int64>(10 + FMath::RandRange(0, 5));
	}

	if (FMath::FRand() < 0.05f)
	{
		const FItemInstance DropItem = FItemFactory::RandomDropFromMonster(1);
		if (DropItem.Rarity != EItemRarity::None)
		{
			AEquipmentDrop* EquipmentDrop = World->SpawnActor<AEquipmentDrop>(
				AEquipmentDrop::StaticClass(),
				GetActorLocation(),
				FRotator::ZeroRotator,
				SpawnParameters);
			if (EquipmentDrop)
			{
				EquipmentDrop->Payload = DropItem;
			}
		}
	}

	Destroy();
}
