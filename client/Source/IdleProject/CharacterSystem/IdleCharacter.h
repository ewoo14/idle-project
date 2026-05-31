#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterSystem/CharacterAnimState.h"
#include "CharacterSystem/StatFormulas.h"
#include "ItemSystem/ItemTypes.h"
#include "IdleCharacter.generated.h"

class UAnimSequence;

class UCameraComponent;
class UBattleAIComponent;
class UCombatComponent;
class UFacialExpressionComponent;
class UIdleAnimInstance;
class UInventoryComponent;
class UInputAction;
class UInputMappingContext;
class USkillComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UStaticMeshComponent;
enum class EBattleState : uint8;
enum class EDamageKind : uint8;

/**
 * M1 클라이언트 코어 부트용 임시 플레이어 캐릭터입니다.
 * 전사 1종을 기준으로 횡스크롤 이동, 공격 입력 로그, 메뉴 토글 입력만 제공합니다.
 */
UCLASS()
class IDLEPROJECT_API AIdleCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AIdleCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void RefreshDerivedStats();

	UFUNCTION(BlueprintCallable, Category = "Idle|Class")
	void SetClassId(EClassId NewClassId);

	UFUNCTION(BlueprintPure, Category = "Idle|Class")
	EClassId GetClassId() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	FPrimaryStats GetCurrentPrimaryStats() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	FDerivedStats GetCurrentDerivedStats() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	int64 GetCombatPower() const;

	UFUNCTION(BlueprintPure, Category = "Idle|Stats")
	int32 GetCurrentLevel() const;

	UFUNCTION()
	void HandleLevelUp(int32 NewLevel);

protected:
	/** 횡스크롤 카메라 거리와 충돌 처리를 담당합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	/** 캐릭터를 바라보는 사이드뷰 카메라입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Camera")
	TObjectPtr<UCameraComponent> SideViewCamera;

	/** BP 아트가 붙기 전까지 사용하는 큐브 플레이스홀더 메시입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Character")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	/** VRoid/VRM4U import 후 적용되는 실제 캐릭터 Skeletal Mesh입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Character")
	TObjectPtr<USkeletalMeshComponent> CharacterMesh;

	/** AnimBlueprint parent class입니다. INI 경로가 있으면 BeginPlay에서 교체됩니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Idle|Character")
	TSubclassOf<UIdleAnimInstance> AnimInstanceClass;

	/** VRoid Blend Shape 기반 표정 제어 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, Category = "Idle|Character")
	TObjectPtr<UFacialExpressionComponent> Facial;

	/** 자동 전투에 사용하는 전투 능력치 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UCombatComponent> Combat;

	/** 주변 몬스터를 자동 탐색하고 공격하는 AI 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<UBattleAIComponent> BattleAI;

	/** 전사 스킬 V1 쿨다운, 패시브, 궁극기 게이지를 관리합니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Combat")
	TObjectPtr<USkillComponent> Skills;

	/** 장비 획득과 자동 장착을 관리하는 캐릭터 인벤토리입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Inventory")
	TObjectPtr<UInventoryComponent> Inventory;

	/** 좌우 이동 입력 액션입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;

	/** 기본 공격 입력 액션입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> AttackAction;

	/** 메뉴 토글 입력 액션입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MenuToggleAction;

	/** 퀘스트 로그 토글 입력 액션입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> QuestLogToggleAction;

	/** 런타임에서 생성하는 EnhancedInput 기본 매핑입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Idle|Movement")
	float MoveSpeed = 450.0f;

	UPROPERTY(EditAnywhere, Category = "Idle")
	EClassId DefaultClassId = EClassId::Warrior;

	UPROPERTY(EditAnywhere, Category = "Idle")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Stats")
	FPrimaryStats CachedPrimaryStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Idle|Stats")
	FDerivedStats CachedDerivedStats;

private:
	UFUNCTION()
	void HandleEquippedChanged(EItemSlot Slot);

	UFUNCTION()
	void HandleStatPointsChanged();

	UFUNCTION()
	void HandleHpChanged(float NewHp);

	UFUNCTION()
	void HandleDeath(AActor* DyingActor);

	UFUNCTION()
	void HandleDamageReceived(float Amount, bool bWasCrit, EDamageKind Kind);

	void ConfigureInputActions();
	void RegisterDefaultMappingContext();
	void ConfigureCharacterVisuals();
	/** VRoid/VRM4U 모델의 액세서리(백팩·로봇팔 등) 머티리얼 슬롯을 INI 키워드 기준으로 숨깁니다. */
	void HideAccessoryMaterialSlots();
	void UpdateAnimInstanceVariables();
	void UpdateLocomotionAnimation();
	void UpdateBattleFacialExpression();
	void Move(const FInputActionValue& Value);
	void Attack(const FInputActionValue& Value);
	void ToggleMenu(const FInputActionValue& Value);
	void ToggleQuestLog(const FInputActionValue& Value);
	int32 GetEffectiveLevel() const;

	float LastObservedHp = 0.0f;
	EBattleState LastObservedBattleState;

	// config 구동 애님 시퀀스(에셋 없으면 nullptr → 폴백, 현 동작 불변).
	UPROPERTY(Transient) TObjectPtr<UAnimSequence> IdleAnimSeq;
	UPROPERTY(Transient) TObjectPtr<UAnimSequence> MoveAnimSeq;
	UPROPERTY(Transient) TObjectPtr<UAnimSequence> AttackAnimSeq;
	UPROPERTY(Transient) TObjectPtr<UAnimSequence> HitAnimSeq;
	UPROPERTY(Transient) TObjectPtr<UAnimSequence> DeathAnimSeq;

	IdleProject::Character::ECharAnimState CurrentAnimState = IdleProject::Character::ECharAnimState::Idle;
	bool bAnimOneShotPlaying = false;
	bool bPendingAttackAnim = false;   // 공격 진입 에지에서 set
	bool bPendingHitAnim = false;      // 피격 델리게이트에서 set
	bool bPrevAttackState = false;     // BattleAI Attack 상태 에지 검출용
};
