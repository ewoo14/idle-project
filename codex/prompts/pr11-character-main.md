# PR #11 캐릭터 메인 호출 — 캐릭터 비주얼 V1 (3D 애니메 풍, SK + AnimInstance + 표정)

당신은 idle-project Codex 캐릭터 메인 에이전트.

## 컨텍스트
- 브랜치: plan/11-character-visual-v1 (체크아웃)
- 베이스: main (PR #1~#10 머지)
- PR: GitHub #11
- 기획서: `docs/planning/slices/11-character-visual-v1.md` (반드시 먼저 읽기)
- UE 엔진: 5.7
- **GDD 스타일 변경**: 도트 → 3D 애니메 풍 (사용자 명시 2026-05-25)
- **VRM4U plugin 자동 설치 완료** (client/Plugins/VRM4U/)
- **사용자 작업 의존**: VRoid + VRM import + Mixamo + AnimBP — **자산 경로 미정 상태에서도 컴파일/실행** 되어야 함 (fallback)

## 이전 PR 학습 사항 (필수 반영)
- UENUM None=0 / TestEqual float `.0f` / int64 `static_cast<int64>()` 명시
- BP/.uasset 직접 생성 불가 — C++ 만 + INI / Static class path 로 자산 참조
- include 경로 모듈 루트 기준
- PR #1 ~ PR #10 의 모든 시스템 유지 (자동 전투, 인벤토리, HUD 등)
- 빌드: `Build.bat IdleProjectEditor Win64 Development -Project=...` → Result: Succeeded

## 임무

### A. AIdleCharacter 변경
**파일**: `client/Source/IdleProject/CharacterSystem/IdleCharacter.h/cpp`

#### 변경 1: PlaceholderMesh → CharacterMesh 동시 보유
- 기존 `UStaticMeshComponent* PlaceholderMesh` 는 **유지** (fallback for VRoid 미import 시)
- 신규 `UPROPERTY(VisibleAnywhere, BlueprintReadOnly) USkeletalMeshComponent* CharacterMesh` 추가
- 생성자에서 둘 다 CreateDefaultSubobject 후 RootComponent 에 attach
- BeginPlay 에서 INI 의 SkeletalMeshPath 가 있으면 동적 로드 + CharacterMesh 활성 + PlaceholderMesh 비활성
  - 없거나 로드 실패 시 PlaceholderMesh 큐브 유지 (graceful fallback)

#### 변경 2: SpringArm 거리 / Offset 조정
- TargetArmLength: 850 → **600** (캐릭터 보기 좋게 가까이)
- SocketOffset: (0, 0, 100) 추가 (캐릭터 상체 중심)

#### 변경 3: FacialExpression 통합
- 멤버 추가: `UPROPERTY(VisibleAnywhere) UFacialExpressionComponent* Facial;`
- BeginPlay 끝에:
  - Combat->OnHpChanged 구독 → HP 감소 시 Facial->SetExpression(Hit, 0.5f) (HP 가 이전보다 줄어든 경우만)
  - Combat->OnDeath 구독 → Facial->SetExpression(Death)
  - BattleAI 상태 변경 시 (Chase/Attack 진입) → Facial->SetExpression(Battle)
  - GameInstance->OnLevelUp 구독 → Facial->SetExpression(LevelUp, 1.5f)
  - 슬라임 사망 콜백 (UInventoryComponent 또는 별도 글로벌 이벤트) → Facial->SetExpression(Smile, 1.0f) — **선택, 어려우면 후속**

#### 변경 4: AnimInstance 연결
- 멤버: `UPROPERTY(EditDefaultsOnly) TSubclassOf<class UIdleAnimInstance> AnimInstanceClass;`
- BeginPlay 에서 INI 의 AnimInstanceClassPath 가 있으면 동적 로드 + CharacterMesh->SetAnimInstanceClass(LoadedClass)
- AnimInstance 변수 갱신 (Tick or Timer):
  - bIsMoving: GetVelocity().Size() > 10
  - bIsAttacking: BattleAI->State == EBattleState::Attack
  - bIsDead: Combat->IsDead()
  - MovementSpeed: GetVelocity().Size()

### B. UFacialExpressionComponent 신규
**파일**: `client/Source/IdleProject/CharacterSystem/FacialExpressionComponent.h/cpp`

```cpp
UENUM(BlueprintType)
enum class EFacialExpression : uint8 {
    None = 0 UMETA(Hidden),
    Idle, Battle, Smile, Hit, Death, LevelUp
};

UCLASS(ClassGroup=(Idle), meta=(BlueprintSpawnableComponent))
class IDLEPROJECT_API UFacialExpressionComponent : public UActorComponent {
    GENERATED_BODY()
public:
    UFunctionExpressionComponent();
    
    UFUNCTION(BlueprintCallable, Category="Idle|Facial")
    void SetExpression(EFacialExpression NewExpression, float Duration = -1.0f);
    // Duration > 0 면 자동으로 Idle 복귀 (FTimerHandle)
    
    UFUNCTION(BlueprintCallable, Category="Idle|Facial")
    EFacialExpression GetCurrentExpression() const { return CurrentExpression; }
    
    // 표정 → VRoid blend shape 이름 매핑 (Joy/Angry/Sorrow/Fun/Surprised)
    // 예: Smile → "Joy" weight 1.0, "Angry" weight 0
    
protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction*) override;
    
private:
    EFacialExpression CurrentExpression = EFacialExpression::Idle;
    FTimerHandle RevertTimerHandle;
    USkeletalMeshComponent* GetTargetMesh() const; // Owner 의 CharacterMesh
    void ApplyBlendShapes(EFacialExpression Expression);
    void RevertToIdle();
};
```

표정 → blend shape weight 매핑 표 (코드 상수):
```cpp
static const TMap<EFacialExpression, TMap<FName, float>> ExpressionMap = {
    { EFacialExpression::Idle,    {} }, // 모두 0
    { EFacialExpression::Battle,  { {TEXT("Angry"), 0.7f} } },
    { EFacialExpression::Smile,   { {TEXT("Joy"), 1.0f}, {TEXT("Fun"), 0.5f} } },
    { EFacialExpression::Hit,     { {TEXT("Sorrow"), 0.8f}, {TEXT("Angry"), 0.3f} } },
    { EFacialExpression::Death,   { {TEXT("Sorrow"), 1.0f} } },
    { EFacialExpression::LevelUp, { {TEXT("Joy"), 1.0f}, {TEXT("Surprised"), 0.8f} } },
};
```

`USkeletalMeshComponent::SetMorphTarget(FName, float)` 로 적용. VRoid 미적용 (CharacterMesh 비활성) 시 함수 no-op.

### C. UIdleAnimInstance 신규
**파일**: `client/Source/IdleProject/CharacterSystem/IdleAnimInstance.h/cpp`

```cpp
UCLASS(Blueprintable)
class IDLEPROJECT_API UIdleAnimInstance : public UAnimInstance {
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Idle|Anim")
    bool bIsMoving = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Idle|Anim")
    bool bIsAttacking = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Idle|Anim")
    bool bIsDead = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Idle|Anim")
    float MovementSpeed = 0.0f;
    
    // BP 가 NativeUpdateAnimation 에서 Owner Pawn 의 상태를 읽거나
    // AIdleCharacter 가 직접 위 변수 set 후 BP State Machine 동작.
};
```

C++ 만으로 State Machine 작성 어려움 — BP 작성 가이드 (별도 .md 작성 권장 또는 PR #11 본문에 명시) 제공.

### D. DefaultEngine.ini 추가
```ini
[/Script/IdleProject.IdleCharacter]
; PR #11 — 사용자가 VRoid + VRM4U import 후 경로 입력. 빈 값이면 PlaceholderMesh 큐브 fallback.
SkeletalMeshPath=
AnimInstanceClassPath=
```

### E. IdleProject.Build.cs 갱신
- Plugin 의존성 추가: `PrivateDependencyModuleNames.AddRange(new string[] { "VRM4U" });`
  - **단**: VRM4U 가 BuildTime 이 아닌 Runtime 만 사용한다면 PrivateDynamicLoadModuleNames 또는 의존성 불필요
  - 확인 후 추가 (없어도 동적 로드 가능)

### F. Tests
**파일**: `client/Source/IdleProject/Tests/FacialExpressionTests.cpp`
- IMPLEMENT_SIMPLE_AUTOMATION_TEST (3건):
  1. SetExpression(Battle) → GetCurrentExpression == Battle
  2. SetExpression(Hit, 0.5f) → 시간 경과 후 Idle 복귀 (시뮬레이션 어려우면 즉시 호출 검증)
  3. EFacialExpression::None 으로 호출 시 graceful (no-op)

## 사전 조사
1. cat docs/planning/slices/11-character-visual-v1.md
2. ls -R client/Source/IdleProject/
3. cat client/Source/IdleProject/CharacterSystem/IdleCharacter.h
4. cat client/Source/IdleProject/CharacterSystem/IdleCharacter.cpp
5. cat client/Source/IdleProject/IdleProject.Build.cs
6. cat client/Config/DefaultEngine.ini
7. ls client/Plugins/VRM4U/Source/  # VRM4U module 구조 확인
8. cat client/Plugins/VRM4U/VRM4U.uplugin

## 자기 검증 (커밋 전)
- UE Build.bat IdleProjectEditor Win64 Development → **Result: Succeeded** (필수)
- 자산 경로 비어있어도 PlaceholderMesh 큐브 fallback 으로 정상 spawn
- UENUM None=0 / TestEqual 캐스트
- 한글 주석

## 커밋
- 5~6 commit, prefix `codex(character):`
- 예시:
  - codex(character): AIdleCharacter SK + Placeholder fallback
  - codex(character): SpringArm 600 / Offset (캐릭터 보기 조정)
  - codex(character): UFacialExpressionComponent (Blend Shape 매핑)
  - codex(character): UIdleAnimInstance 베이스 + 변수
  - codex(character): INI 캐릭터 자산 경로 + Build.cs VRM4U 의존성
  - codex(character): UE Automation FacialExpression 테스트

## 푸시
git push origin plan/11-character-visual-v1

## 범위 외 (절대 금지)
- AnimBlueprint (.uasset) 작성 — BP 는 사용자 에디터 작업
- 사용자 자산 추정 / 임시 자산 자체 생성
- 다른 시스템 (인벤토리/스킬/백엔드) 변경
- 사운드 / Niagara

## 알려진 한계 (명시 OK)
- AnimBlueprint State Machine 은 BP — 사용자가 ABP_Soyeon 생성 필요
- VRM4U import 자체는 사용자 에디터 작업
- 표정 매핑 (VRoid blend shape 이름) 이 VRoid 버전에 따라 다를 수 있음 — 사용자가 실제 import 후 hotfix 가능

## 완료 출력
```
## Codex PR #11 캐릭터 메인 결과
### 추가/수정 파일 + 커밋
### 주요 구현
### 자기 검증 (UE Build 결과, 자산 fallback 검증)
### 알려진 한계 / 후속 (사용자 BP 작업 의존)
```

작업 디렉터리 `C:\game\idle game\repo`, 브랜치 `plan/11-character-visual-v1`.
이제 시작하세요.
