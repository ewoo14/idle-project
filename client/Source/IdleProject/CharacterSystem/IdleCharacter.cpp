#include "CharacterSystem/IdleCharacter.h"

#include "Camera/CameraComponent.h"
#include "CharacterSystem/IdleMonster.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "ItemSystem/InventoryComponent.h"
#include "UObject/ConstructorHelpers.h"

AIdleCharacter::AIdleCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 850.0f;
	SpringArm->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;

	SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
	SideViewCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	SideViewCamera->bUsePawnControlRotation = false;

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -28.0f));
	PlaceholderMesh->SetRelativeScale3D(FVector(0.75f, 0.35f, 1.4f));
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CubeMesh.Object);
	}

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	BattleAI = CreateDefaultSubobject<UBattleAIComponent>(TEXT("BattleAI"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = MoveSpeed;
	Movement->bOrientRotationToMovement = false;
	Movement->bConstrainToPlane = true;
	Movement->SetPlaneConstraintNormal(FVector::YAxisVector);

	ConfigureInputActions();
}

void AIdleCharacter::BeginPlay()
{
	Super::BeginPlay();

	RegisterDefaultMappingContext();

	if (Inventory)
	{
		Inventory->OnEquippedChanged.AddDynamic(this, &AIdleCharacter::HandleEquippedChanged);
	}
	RefreshDerivedStats();

	if (BattleAI)
	{
		BattleAI->TargetActorClass = AIdleMonster::StaticClass();
		BattleAI->StartBattle();
	}
}

void AIdleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		UE_LOG(LogTemp, Warning, TEXT("IdleCharacter requires EnhancedInputComponent."));
		return;
	}

	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIdleCharacter::Move);
	EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AIdleCharacter::Move);
	EnhancedInput->BindAction(AttackAction, ETriggerEvent::Started, this, &AIdleCharacter::Attack);
	EnhancedInput->BindAction(MenuToggleAction, ETriggerEvent::Started, this, &AIdleCharacter::ToggleMenu);
}

void AIdleCharacter::RefreshDerivedStats()
{
	const FPrimaryStats Primary = FStatFormulas::DefaultPrimaryStats(DefaultClassId, Level);
	const FDerivedStats EquipBonus = Inventory ? Inventory->ComputeEquipmentBonus() : FDerivedStats();
	const FDerivedStats Derived = FStatFormulas::DeriveStats(Primary, Level, EquipBonus);

	UE_LOG(
		LogTemp,
		Display,
		TEXT("[Inventory] Stats refreshed L%d ClassId=%d HP=%.1f ATK=%.1f DEF=%.1f EquipAtk=%.1f EquipDef=%.1f EquipHp=%.1f"),
		Level,
		static_cast<int32>(DefaultClassId),
		Derived.Hp,
		Derived.PhysAtk,
		Derived.PhysDef,
		EquipBonus.PhysAtk,
		EquipBonus.PhysDef,
		EquipBonus.Hp);

	if (Combat)
	{
		const float HpRatio = Combat->MaxHp > 0.0f ? Combat->CurrentHp / Combat->MaxHp : 1.0f;
		Combat->InitializeCombat(Derived.Hp, Derived.PhysAtk, Derived.PhysDef, Derived.AtkSpeed);
		Combat->CurrentHp = FMath::Clamp(Derived.Hp * HpRatio, 0.0f, Combat->MaxHp);
		Combat->OnHpChanged.Broadcast(Combat->CurrentHp);
	}
}

void AIdleCharacter::HandleEquippedChanged(EItemSlot Slot)
{
	RefreshDerivedStats();
}

void AIdleCharacter::ConfigureInputActions()
{
	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveLeftRight"));
	MoveAction->ValueType = EInputActionValueType::Axis1D;

	AttackAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Attack"));
	AttackAction->ValueType = EInputActionValueType::Boolean;

	MenuToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MenuToggle"));
	MenuToggleAction->ValueType = EInputActionValueType::Boolean;

	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_IdleDefault"));
	DefaultMappingContext->MapKey(MoveAction, EKeys::D);
	DefaultMappingContext->MapKey(MoveAction, EKeys::Right);

	FEnhancedActionKeyMapping& MoveLeftA = DefaultMappingContext->MapKey(MoveAction, EKeys::A);
	MoveLeftA.Modifiers.Add(NewObject<UInputModifierNegate>(DefaultMappingContext));

	FEnhancedActionKeyMapping& MoveLeftArrow = DefaultMappingContext->MapKey(MoveAction, EKeys::Left);
	MoveLeftArrow.Modifiers.Add(NewObject<UInputModifierNegate>(DefaultMappingContext));

	DefaultMappingContext->MapKey(AttackAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(MenuToggleAction, EKeys::Escape);
}

void AIdleCharacter::RegisterDefaultMappingContext()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (Subsystem && DefaultMappingContext)
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
}

void AIdleCharacter::Move(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	AddMovementInput(FVector::XAxisVector, Axis);
}

void AIdleCharacter::Attack(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		UE_LOG(LogTemp, Display, TEXT("IdleCharacter Attack input received."));
	}
}

void AIdleCharacter::ToggleMenu(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		UE_LOG(LogTemp, Display, TEXT("IdleCharacter MenuToggle input received."));
	}
}
