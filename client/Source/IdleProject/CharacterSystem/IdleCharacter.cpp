#include "CharacterSystem/IdleCharacter.h"

#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "CharacterSystem/FacialExpressionComponent.h"
#include "CharacterSystem/IdleMonster.h"
#include "CharacterSystem/IdleAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "CombatSystem/SkillComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameCore/IdleGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "ItemSystem/InventoryComponent.h"
#include "Misc/ConfigCacheIni.h"
#include "UI/IdleHUD.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const TCHAR* IdleCharacterConfigSection = TEXT("/Script/IdleProject.IdleCharacter");
}

AIdleCharacter::AIdleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	LastObservedBattleState = EBattleState::Idle;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 88.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 100.0f);
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

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));
	CharacterMesh->SetupAttachment(RootComponent);
	CharacterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh->SetVisibility(false);
	CharacterMesh->SetHiddenInGame(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CubeMesh.Object);
	}

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	BattleAI = CreateDefaultSubobject<UBattleAIComponent>(TEXT("BattleAI"));
	Skills = CreateDefaultSubobject<USkillComponent>(TEXT("Skills"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	Facial = CreateDefaultSubobject<UFacialExpressionComponent>(TEXT("Facial"));

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
	ConfigureCharacterVisuals();

	if (Inventory)
	{
		Inventory->OnEquippedChanged.AddDynamic(this, &AIdleCharacter::HandleEquippedChanged);
	}
	if (Skills)
	{
		Skills->LoadDefaultWarriorSkills();
	}
	RefreshDerivedStats();
	LastObservedHp = Combat ? Combat->CurrentHp : 0.0f;

	if (BattleAI)
	{
		BattleAI->TargetActorClass = AIdleMonster::StaticClass();
		BattleAI->StartBattle();
		LastObservedBattleState = BattleAI->State;
	}

	if (Combat)
	{
		Combat->OnHpChanged.AddDynamic(this, &AIdleCharacter::HandleHpChanged);
		Combat->OnDeath.AddDynamic(this, &AIdleCharacter::HandleDeath);
	}

	if (UIdleGameInstance* IdleGameInstance = GetGameInstance<UIdleGameInstance>())
	{
		IdleGameInstance->OnLevelUp.AddDynamic(this, &AIdleCharacter::HandleLevelUp);
	}
}

void AIdleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAnimInstanceVariables();
	UpdateBattleFacialExpression();
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
	EnhancedInput->BindAction(QuestLogToggleAction, ETriggerEvent::Started, this, &AIdleCharacter::ToggleQuestLog);
}

void AIdleCharacter::RefreshDerivedStats()
{
	const FPrimaryStats Primary = FStatFormulas::DefaultPrimaryStats(DefaultClassId, Level);
	const FDerivedStats EquipBonus = Inventory ? Inventory->ComputeEquipmentBonus() : FDerivedStats();
	FDerivedStats Derived = FStatFormulas::DeriveStats(Primary, Level, EquipBonus);
	if (Skills)
	{
		Skills->ApplyPassivesToStats(Derived);
	}

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

void AIdleCharacter::HandleHpChanged(float NewHp)
{
	if (Facial && NewHp < LastObservedHp)
	{
		Facial->SetExpression(EFacialExpression::Hit, 0.5f);
	}

	LastObservedHp = NewHp;
}

void AIdleCharacter::HandleDeath(AActor* DyingActor)
{
	if (Facial)
	{
		Facial->SetExpression(EFacialExpression::Death);
	}
}

void AIdleCharacter::HandleLevelUp(int32 NewLevel)
{
	if (Facial)
	{
		Facial->SetExpression(EFacialExpression::LevelUp, 1.5f);
	}
}

void AIdleCharacter::ConfigureInputActions()
{
	MoveAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MoveLeftRight"));
	MoveAction->ValueType = EInputActionValueType::Axis1D;

	AttackAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_Attack"));
	AttackAction->ValueType = EInputActionValueType::Boolean;

	MenuToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_MenuToggle"));
	MenuToggleAction->ValueType = EInputActionValueType::Boolean;

	QuestLogToggleAction = CreateDefaultSubobject<UInputAction>(TEXT("IA_QuestLogToggle"));
	QuestLogToggleAction->ValueType = EInputActionValueType::Boolean;

	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_IdleDefault"));
	DefaultMappingContext->MapKey(MoveAction, EKeys::D);
	DefaultMappingContext->MapKey(MoveAction, EKeys::Right);

	FEnhancedActionKeyMapping& MoveLeftA = DefaultMappingContext->MapKey(MoveAction, EKeys::A);
	MoveLeftA.Modifiers.Add(NewObject<UInputModifierNegate>(DefaultMappingContext));

	FEnhancedActionKeyMapping& MoveLeftArrow = DefaultMappingContext->MapKey(MoveAction, EKeys::Left);
	MoveLeftArrow.Modifiers.Add(NewObject<UInputModifierNegate>(DefaultMappingContext));

	DefaultMappingContext->MapKey(AttackAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(MenuToggleAction, EKeys::Escape);
	DefaultMappingContext->MapKey(QuestLogToggleAction, EKeys::Q);
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

void AIdleCharacter::ConfigureCharacterVisuals()
{
	FString SkeletalMeshPath;
	if (GConfig)
	{
		GConfig->GetString(IdleCharacterConfigSection, TEXT("SkeletalMeshPath"), SkeletalMeshPath, GEngineIni);
	}

	if (!SkeletalMeshPath.IsEmpty())
	{
		if (USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *SkeletalMeshPath)))
		{
			CharacterMesh->SetSkeletalMesh(LoadedMesh);
			// SetSkeletalMesh 는 메시의 Physics Asset(VRM4U PHYS_*)으로 물리 상태를 재생성한다.
			// 캡슐만 게임플레이 충돌체로 쓰므로, 메시 지오메트리가 몬스터 추격과 충돌하지 않도록
			// NoCollision 을 재확정한다. (미설정 시 몬스터가 메시에 부딪혀 위로 튕겨 사라짐 — hotfix/13.)
			CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			CharacterMesh->SetCollisionProfileName(TEXT("NoCollision"));
			CharacterMesh->SetSimulatePhysics(false);
			CharacterMesh->SetVisibility(true);
			CharacterMesh->SetHiddenInGame(false);
			PlaceholderMesh->SetVisibility(false);
			PlaceholderMesh->SetHiddenInGame(true);

			// VRoid/VRM4U 모델에 포함된 액세서리(백팩·로봇팔 등) 머티리얼 슬롯을 숨긴다.
			HideAccessoryMaterialSlots();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CharacterVisual] SkeletalMesh load failed: %s"), *SkeletalMeshPath);
		}
	}

	FString AnimClassPath;
	if (GConfig)
	{
		GConfig->GetString(IdleCharacterConfigSection, TEXT("AnimInstanceClassPath"), AnimClassPath, GEngineIni);
	}

	if (!AnimClassPath.IsEmpty())
	{
		if (UClass* LoadedClass = StaticLoadClass(UIdleAnimInstance::StaticClass(), nullptr, *AnimClassPath))
		{
			AnimInstanceClass = LoadedClass;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[CharacterVisual] AnimInstanceClass load failed: %s"), *AnimClassPath);
		}
	}

	if (CharacterMesh && AnimInstanceClass)
	{
		CharacterMesh->SetAnimInstanceClass(AnimInstanceClass);
	}
}

void AIdleCharacter::HideAccessoryMaterialSlots()
{
	if (!CharacterMesh)
	{
		return;
	}

	// 숨김 키워드는 INI 로 조정 가능 (재컴파일 불필요).
	auto ReadKeywords = [](const TCHAR* Key, const TCHAR* DefaultValue)
	{
		FString Raw;
		if (GConfig)
		{
			GConfig->GetString(IdleCharacterConfigSection, Key, Raw, GEngineIni);
		}
		if (Raw.IsEmpty())
		{
			Raw = DefaultValue;
		}
		TArray<FString> Keywords;
		Raw.ParseIntoArray(Keywords, TEXT(","), true);
		return Keywords;
	};

	// 머티리얼 슬롯 키워드: 백팩 마운트(backpack_*) 와 로봇팔 케이싱(arm_mat/arm_plastic/armgear_plastic) 표면.
	const TArray<FString> SlotKeywords = ReadKeywords(TEXT("HiddenMaterialSlotKeywords"), TEXT("backpack,arm"));
	// 본(bone) 키워드: 로봇팔 골격 전체(robo_root_pole/robo_*_L/robo_wire_*/robo_arm). 본 단위로 접으면
	// 전선·관절·글로우 등 머티리얼이 무엇이든 로봇팔 지오메트리가 완전히 사라진다. 인간 팔(upper_arm_L 등)은 'robo' 미포함이라 안전.
	const TArray<FString> BoneKeywords = ReadKeywords(TEXT("HiddenBoneKeywords"), TEXT("robo"));

	// 1) 본 단위 숨김 — 로봇팔 골격 전체 제거 (가장 확실).
	if (USkeletalMesh* SkelMesh = CharacterMesh->GetSkeletalMeshAsset())
	{
		const FReferenceSkeleton& RefSkeleton = SkelMesh->GetRefSkeleton();
		for (int32 BoneIndex = 0; BoneIndex < RefSkeleton.GetNum(); ++BoneIndex)
		{
			const FName BoneFName = RefSkeleton.GetBoneName(BoneIndex);
			const FString BoneName = BoneFName.ToString();
			for (const FString& Keyword : BoneKeywords)
			{
				const FString Trimmed = Keyword.TrimStartAndEnd();
				if (!Trimmed.IsEmpty() && BoneName.Contains(Trimmed, ESearchCase::IgnoreCase))
				{
					CharacterMesh->HideBoneByName(BoneFName, PBO_None);
					UE_LOG(LogTemp, Display, TEXT("[CharacterVisual] 본 숨김 %d (%s) — 키워드 '%s'"), BoneIndex, *BoneName, *Trimmed);
					break;
				}
			}
		}
	}

	// 2) 머티리얼 슬롯 숨김 — 백팩 마운트 등 본 단위로 안 잡히는 표면 처리. 전체 슬롯명은 로그로 남겨 키워드 조정에 활용.
	const TArray<FName> SlotNames = CharacterMesh->GetMaterialSlotNames();
	for (int32 SlotIndex = 0; SlotIndex < SlotNames.Num(); ++SlotIndex)
	{
		const FString SlotName = SlotNames[SlotIndex].ToString();
		UE_LOG(LogTemp, Display, TEXT("[CharacterVisual] 머티리얼 슬롯 %d = %s"), SlotIndex, *SlotName);

		for (const FString& Keyword : SlotKeywords)
		{
			const FString Trimmed = Keyword.TrimStartAndEnd();
			if (!Trimmed.IsEmpty() && SlotName.Contains(Trimmed, ESearchCase::IgnoreCase))
			{
				// VRoid 메시는 LOD0 에서 섹션:머티리얼 = 1:1 이므로 SectionIndex == SlotIndex 로 숨긴다.
				CharacterMesh->ShowMaterialSection(SlotIndex, SlotIndex, false, 0);
				UE_LOG(LogTemp, Display, TEXT("[CharacterVisual] 액세서리 슬롯 숨김 %d (%s) — 키워드 '%s'"), SlotIndex, *SlotName, *Trimmed);
				break;
			}
		}
	}
}

void AIdleCharacter::UpdateAnimInstanceVariables()
{
	if (!CharacterMesh)
	{
		return;
	}

	UIdleAnimInstance* IdleAnimInstance = Cast<UIdleAnimInstance>(CharacterMesh->GetAnimInstance());
	if (!IdleAnimInstance)
	{
		return;
	}

	const float Speed = GetVelocity().Size();
	IdleAnimInstance->MovementSpeed = Speed;
	IdleAnimInstance->bIsMoving = Speed > 10.0f;
	IdleAnimInstance->bIsAttacking = BattleAI && BattleAI->State == EBattleState::Attack;
	IdleAnimInstance->bIsDead = Combat && Combat->IsDead();
}

void AIdleCharacter::UpdateBattleFacialExpression()
{
	if (!Facial || !BattleAI)
	{
		return;
	}

	const EBattleState CurrentState = BattleAI->State;
	const bool bEnteredBattle = CurrentState != LastObservedBattleState &&
		(CurrentState == EBattleState::Chase || CurrentState == EBattleState::Attack);

	if (bEnteredBattle)
	{
		Facial->SetExpression(EFacialExpression::Battle);
	}

	LastObservedBattleState = CurrentState;
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

void AIdleCharacter::ToggleQuestLog(const FInputActionValue& Value)
{
	if (!Value.Get<bool>())
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	AIdleHUD* IdleHUD = PlayerController ? Cast<AIdleHUD>(PlayerController->GetHUD()) : nullptr;
	if (IdleHUD)
	{
		IdleHUD->ToggleQuestLog();
	}
}
