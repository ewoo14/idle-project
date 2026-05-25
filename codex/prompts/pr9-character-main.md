# PR #9 캐릭터 메인 호출 (인벤토리 + 장비 V1)

당신은 idle-project Codex 캐릭터·아이템·능력치 메인 에이전트.

## 컨텍스트
- 브랜치: plan/09-inventory-v1 (체크아웃됨)
- 베이스: main (PR #1~#8 머지)
- PR: GitHub #9
- 기획서: `docs/planning/slices/09-inventory-v1.md` (반드시 먼저 읽기)
- UE 엔진: 5.7 (사용자 환경 빌드 검증 완료 + PIE 시각화 정상 — 캐릭터/슬라임/HUD 모두 동작)

## PR #4/#5/#6/#8 학습 사항 (반드시 반영)
1. UENUM 에 항상 `None = 0 UMETA(Hidden)` (UHT WarningsAsErrors 회피)
2. `IdleProject.Build.cs` 의 `PublicIncludePaths.Add(ModuleDirectory)` 이미 적용됨
3. `TestEqual` 호출 시 float 인자 `.0f`, int64 인자 `static_cast<int64>()` 명시 캐스트
4. BP/.uasset 직접 생성 불가 — 모든 UI/Actor 는 C++ + Slate 또는 spawn 코드
5. include 경로는 모듈 루트 기준 (예: `#include "ItemSystem/InventoryComponent.h"`)
6. Engine include: `Engine/StaticMesh.h`, `Components/StaticMeshComponent.h` 등 정확한 경로
7. UE 5.7 의 SkyAtmosphere 는 추가 모듈 필요 — 본 PR 무관

## 임무 — 기획서 §2 In Scope 거의 전부 (보조 §2.4 ItemDB CSV, §2.5 server equipment.ts 제외)

### A. ItemSystem 모듈 신규

#### `client/Source/IdleProject/ItemSystem/ItemTypes.h`
- `UENUM(BlueprintType) enum class EItemSlot : uint8` — None=0 UMETA(Hidden), Weapon=1, Helmet, Top, Bottom, Shoes, Gloves, Cloak, Accessory
- `UENUM(BlueprintType) enum class EItemRarity : uint8` — None=0 UMETA(Hidden), Common=1, Uncommon=2, Rare=3
- `USTRUCT(BlueprintType) FItemInstance`:
  - `FName ItemId`, `EItemSlot Slot`, `EItemRarity Rarity`, `FText DisplayName`, `float BonusAtk`, `float BonusDef`, `float BonusHp`, `int32 EnhanceLevel` (0~5)
  - BlueprintType + GENERATED_BODY()
- `struct FItemPowerScore` — static `int32 Compute(const FItemInstance&)` = `(Atk + Def + Hp/10) × (1 + EnhanceLevel × 0.1)`

#### `client/Source/IdleProject/ItemSystem/InventoryComponent.h/cpp`
- `UInventoryComponent : UActorComponent`
- 멤버:
  - `UPROPERTY() TArray<FItemInstance> Items` (max 100)
  - `UPROPERTY() TMap<EItemSlot, int32> EquippedIndex` (Slot → Items index, -1=없음)
- 메서드:
  - `void AddItem(const FItemInstance& NewItem)` — 인벤토리 추가 + 자동 장착 비교 (현 슬롯 장비 PowerScore < NewItem 이면 EquipItem 호출)
  - `void EquipItem(int32 ItemIndex)`
  - `void UnequipSlot(EItemSlot Slot)`
  - `FDerivedStats ComputeEquipmentBonus() const` — 모든 EquippedItems 의 Bonus 합계를 FDerivedStats 로 (StatFormulas::DeriveStats 의 EquipmentBonus 인자로 전달 가능)
  - `const FItemInstance* GetEquippedItem(EItemSlot) const`
- DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedChanged, EItemSlot, Slot); FOnEquippedChanged OnEquippedChanged;
- 인벤토리 추가 시 OnEquippedChanged.Broadcast(Slot) (장착됐을 때만)

#### `client/Source/IdleProject/ItemSystem/EquipmentDrop.h/cpp`
- `AEquipmentDrop : AActor` (AGoldDrop 와 비슷 패턴)
- 멤버:
  - `UPROPERTY() FItemInstance Payload;`
  - `UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;` (Cube placeholder, 등급별 색상 — DynamicMaterial 또는 Vertex Color)
- BeginPlay 0.5s 후 자동 흡수 시작 (FTimerHandle)
- Tick 또는 Timer: 가장 가까운 AIdleCharacter 로 VInterpTo
- 거리 50 이내: `IdleCharacter->FindComponentByClass<UInventoryComponent>()->AddItem(Payload)` + Destroy
- 등급별 색상: Common 회색, Uncommon 녹색, Rare 파랑 (UIThemeTokens 미러)

#### `client/Source/IdleProject/ItemSystem/ItemFactory.h/cpp`
- struct FItemFactory (static 함수만)
- `static FItemInstance RandomDropFromMonster(int32 MonsterLevel)`:
  - Rarity: 70% Common / 20% Uncommon / 8% Rare / 2% None (드롭 없음 — 실제 mob 드롭률 5% 와 별개로 함수 내 None 처리)
  - Slot: Weapon 50%, 방어구 6슬롯 균등 7%씩 (Cloak 8% Accessory 8%)
  - Bonus: `MonsterLevel × FMath::FRandRange(1.0, 2.0)` (Atk/Def/Hp 중 Slot 별 가중)
    - Weapon: 100% ATK
    - Helmet/Top/Bottom/Shoes/Gloves/Cloak: 70% DEF + 30% HP/10
    - Accessory: 50% ATK + 30% DEF + 20% HP/10
  - EnhanceLevel: 0 (드롭 즉시는 0)
  - DisplayName: `{Rarity Adj} {Slot Noun}` 한글 placeholder (예: "정련된 검", "거친 신발", "광택의 외투")

### B. CharacterSystem 확장

#### `AIdleCharacter` 갱신
- 생성자에 `Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));`
- BeginPlay 끝에:
  - `Inventory->OnEquippedChanged.AddDynamic(this, &AIdleCharacter::HandleEquippedChanged);`
  - `RefreshDerivedStats();`
- 새 메서드 `void RefreshDerivedStats()`:
  - `Primary = FStatFormulas::DefaultPrimaryStats(DefaultClassId, Level);`
  - `EquipBonus = Inventory->ComputeEquipmentBonus();`
  - `Derived = FStatFormulas::DeriveStats(Primary, Level, EquipBonus);`
  - Combat->InitializeCombat(Derived.Hp, Derived.PhysAtk, Derived.PhysDef, Derived.AtkSpeed) — CurrentHp 는 maxHp 비율 유지
  - UE_LOG Display 한 줄
- `UFUNCTION() void HandleEquippedChanged(EItemSlot Slot)` — RefreshDerivedStats() 호출

#### `AIdleMonster` 갱신
- OnDeath 콜백에 추가:
  - 5% 확률 EquipmentDrop spawn (FItemFactory::RandomDropFromMonster(1) — 본 PR 단순화, Level 1 고정)
  - 기존 GoldDrop spawn 유지
- 위치: 몬스터 사망 위치

### C. UI - HUD 베이스만 (Slate 위젯 실 구현은 보조 호출 위임 OK)
- `IdleHUD.h/cpp` 에 InventoryComponent.OnEquippedChanged 구독용 콜백 + Slate Widget 갱신 메서드 골격 추가
- 캐릭터의 Inventory 에 접근하는 방법 정의 (PostInitializeComponents 에서 PlayerPawn 의 InventoryComponent 캐시)

### D. INI (영향 없으면 변경 불필요)
- 기존 DefaultEngine.ini 유지

### E. Tests

#### `client/Source/IdleProject/Tests/InventoryTests.cpp`
- IMPLEMENT_SIMPLE_AUTOMATION_TEST (5건):
  1. FItemPowerScore::Compute 결정성 (같은 아이템 → 같은 점수)
  2. Common 무기 → Uncommon 무기 교체 시 자동 장착
  3. 두 슬롯 (Weapon + Helmet) 동시 장착 시 ComputeEquipmentBonus 합계
  4. UnequipSlot 후 ComputeEquipmentBonus 감소
  5. ItemFactory::RandomDropFromMonster(1) 결과 슬롯/등급/보너스가 범위 내

## 사전 조사
1. cat docs/planning/slices/09-inventory-v1.md
2. ls -R client/Source/IdleProject/
3. cat client/Source/IdleProject/CharacterSystem/IdleCharacter.h
4. cat client/Source/IdleProject/CharacterSystem/IdleCharacter.cpp | head -100
5. cat client/Source/IdleProject/CharacterSystem/IdleMonster.cpp
6. cat client/Source/IdleProject/CharacterSystem/StatFormulas.h
7. cat client/Source/IdleProject/ItemSystem/GoldDrop.h
8. cat client/Source/IdleProject/ItemSystem/GoldDrop.cpp
9. cat client/Source/IdleProject/CombatSystem/CombatComponent.h
10. cat client/Source/IdleProject/UI/IdleHUD.h
11. cat client/Source/IdleProject/UI/UIThemeTokens.h

## 자기 검증 (커밋 전 의무)
- UE C++ 컴파일 sanity check (include / 타입 / API)
- 모든 UENUM 에 None=0
- TestEqual float `.0f` / int64 `static_cast<int64>()` 명시
- include 경로 모듈 루트 기준
- UPROPERTY / UFUNCTION 매크로 정합
- DECLARE_DYNAMIC_MULTICAST_DELEGATE 매개변수 BP 호환
- 한글 주석

**가능하면** 실 빌드 검증:
- `& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:/game/idle game/repo/client/IdleProject.uproject" -WaitMutex`
- exit 0 (Result: Succeeded) 확인

## 커밋
- 5~7 commit, prefix `codex(character):`
- 예:
  - codex(character): ItemTypes (8슬롯, 3등급, FItemInstance)
  - codex(character): InventoryComponent (자동 장착, 능력치 합)
  - codex(character): AEquipmentDrop 자동 흡수
  - codex(character): ItemFactory 드롭 생성
  - codex(character): AIdleCharacter Inventory + RefreshDerivedStats
  - codex(character): AIdleMonster OnDeath 장비 드롭 5%
  - codex(character): UE Automation Inventory 테스트

## 푸시
모든 commit 후 git push origin plan/09-inventory-v1

## 범위 외 (절대 금지)
- Epic/Legendary/Mythic 등급
- 강화 +6~+15
- 잠재 옵션
- 세트 효과
- 장비 비교 UI
- 인벤토리 전체 위젯 (옵션이므로 시간 남으면 placeholder)
- 서버 측 검증
- BP/.uasset
- Steam SDK
- 사운드 / Niagara

## 완료 출력
```
## Codex PR #9 캐릭터 메인 결과
### 추가/수정 파일 + 커밋 (목록)
### 주요 구현
### 자기 검증 (UE 빌드 결과)
### 알려진 한계 / 후속
```

작업 디렉터리 `C:\game\idle game\repo`, 브랜치 `plan/09-inventory-v1`. 이제 시작하세요.
