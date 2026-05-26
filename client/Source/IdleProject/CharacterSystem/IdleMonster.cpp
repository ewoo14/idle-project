#include "CharacterSystem/IdleMonster.h"

#include "CharacterSystem/IdleCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CombatSystem/BattleAIComponent.h"
#include "CombatSystem/CombatComponent.h"
#include "GameCore/IdleGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ItemSystem/EquipmentDrop.h"
#include "ItemSystem/ItemFactory.h"
#include "ItemSystem/GoldDrop.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AIdleMonster::AIdleMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(34.0f, 44.0f);
	// 오토배틀에서 전투원끼리는 물리적으로 막을 필요가 없다. Pawn 채널을 무시해 플레이어·다른 몬스터
	// 캡슐과의 충돌 이탈(depenetration) 임펄스를 제거한다 (이 임펄스가 몬스터를 위로 튕겨 사라지게 함).
	// 지면(WorldStatic)은 그대로 Block 하여 캐릭터가 바닥에 선다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

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
	// 평면 스테이지의 오토배틀 — 몬스터는 지면선에 고정되어 X 로만 추격한다. 중력을 끄면 지면 충돌
	// 여부와 무관하게 낙하하지 않는다. (기존 중력 + 공중/바닥아래 spawn 조합이 몬스터를 월드 밖으로 떨어뜨려 사라지게 함.)
	Movement->GravityScale = 0.0f;
}

void AIdleMonster::BeginPlay()
{
	Super::BeginPlay();

	if (UMaterialInstanceDynamic* DynamicMaterial = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), bIsBoss ? FLinearColor(0.9f, 0.15f, 0.05f) : FLinearColor(0.05f, 0.8f, 0.2f));
	}

	if (Combat)
	{
		const float Defense = bIsBoss ? 12.0f : 5.0f;
		Combat->InitializeCombat(GetConfiguredMaxHp(), GetConfiguredAttack(), Defense, bIsBoss ? 0.8f : 1.0f, 0.0f, Defense, 0.0f, 1.5f);
		Combat->OnDeath.AddDynamic(this, &AIdleMonster::HandleDeath);
	}

	if (BattleAI)
	{
		BattleAI->TargetActorClass = AIdleCharacter::StaticClass();
		BattleAI->StartBattle();
	}
}

void AIdleMonster::SetBoss(bool bInBoss)
{
	bIsBoss = bInBoss;
	if (PlaceholderMesh)
	{
		PlaceholderMesh->SetRelativeScale3D(bIsBoss ? FVector(1.35f, 1.35f, 1.35f) : FVector(0.75f, 0.75f, 0.75f));
	}
}

float AIdleMonster::GetConfiguredMaxHp() const
{
	return bIsBoss ? BossMaxHp : NormalMaxHp;
}

float AIdleMonster::GetConfiguredAttack() const
{
	return bIsBoss ? BossAttack : NormalAttack;
}

void AIdleMonster::HandleDeath(AActor* DyingActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		Destroy();
		return;
	}

	// EXP 지급 — PR #1 §3.2.2 미러: monster level × 12 EXP (PR #9 단계 슬라임 = level 1).
	// AIdleGameInstance::AddExp 가 누적 / 레벨업 / 델리게이트 broadcast 처리.
	UIdleGameInstance* GameInstance = Cast<UIdleGameInstance>(UGameplayStatics::GetGameInstance(World));
	if (GameInstance)
	{
		const int64 ExpReward = 12;
		GameInstance->AddExp(ExpReward);
		GameInstance->RecordMonsterKilled();
		if (bIsBoss)
		{
			GameInstance->MarkChapter1BossDefeated();
		}
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGoldDrop* GoldDrop = World->SpawnActor<AGoldDrop>(AGoldDrop::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, SpawnParameters);
	if (GoldDrop)
	{
		const int64 BaseGoldAmount = static_cast<int64>(10 + FMath::RandRange(0, 5));
		GoldDrop->Amount = GameInstance ? GameInstance->ApplyEquippedPetGoldBonus(BaseGoldAmount) : BaseGoldAmount;
	}

	const float DropChance = GameInstance ? GameInstance->ApplyEquippedPetDropBonusChance(0.05f) : 0.05f;
	if (FMath::FRand() < DropChance)
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
