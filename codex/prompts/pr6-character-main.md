# PR #6 캐릭터 메인 호출 (자동 전투 V1)

당신은 idle-project Codex 캐릭터·아이템·능력치 메인 에이전트.

## 컨텍스트
- 브랜치: plan/06-auto-battle-v1 (체크아웃됨)
- 베이스: main (PR #1~#5 머지)
- PR: GitHub #6
- 기획서: `docs/planning/slices/06-auto-battle-v1.md` (반드시 먼저 읽기)
- UE 엔진: 5.7 (사용자 환경 검증 완료, Build.bat → Result: Succeeded)

## PR #4 학습 사항 (반드시 반영)
1. UENUM enum 에 항상 `None = 0 UMETA(Hidden)` 추가 (UHT WarningsAsErrors 회피)
2. 도메인 디렉터리 `IdleProject.Build.cs` 의 `PublicIncludePaths.Add(ModuleDirectory)` 이미 적용됨
3. `TestEqual` 호출 시 float 인자 `.0f`, int64 인자 `static_cast<int64>()` 명시 캐스트
4. BP/.uasset 직접 생성 불가 — 모든 UI/Actor 는 C++ + Slate 또는 spawn 코드
5. include 경로는 모듈 루트 기준 (예: `#include "CombatSystem/CombatComponent.h"`)

## 임무 — 기획서 §2 In Scope (보조 §2.7 server combat.ts 제외)

### A. CombatSystem 모듈 신규
- `client/Source/IdleProject/CombatSystem/CombatComponent.h/cpp`
  - `UCombatComponent : UActorComponent`
  - 멤버: MaxHp, CurrentHp, Atk, Def, AtkSpeed (float, BlueprintReadWrite), AttackRange (기본 200)
  - UFUNCTION TakeDamage(float Damage, AActor* Instigator)
  - DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DyingActor); FOnDeath OnDeath
  - DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHpChanged, float, NewHp); FOnHpChanged OnHpChanged
  - bool IsDead() const

- `client/Source/IdleProject/CombatSystem/CombatFormulas.h/cpp`
  - struct FCombatFormulas (static 함수)
  - `static float ComputeDamage(float Atk, float Def)` = `FMath::Max(Atk * 0.05f, Atk - Def * 0.6f)`

- `client/Source/IdleProject/CombatSystem/BattleAIComponent.h/cpp`
  - `UBattleAIComponent : UActorComponent`
  - FTimerHandle 0.2s (5Hz) — TickComponent 비활성
  - State enum (EBattleState: None=0, Idle, Chase, Attack, Dead)
  - `void StartBattle()`, `void StopBattle()`, `AActor* FindClosestEnemy()`
  - 적이 AttackRange 안 + cooldown 끝 → Attack (TakeDamage 호출)
  - 아니면 거리 보간 이동 (캐릭터: AddMovementInput, 몬스터: SetActorLocation 보간)
  - TargetActorClass (UPROPERTY) 로 캐릭터/몬스터 진영 구분

### B. CharacterSystem 확장
- `AIdleCharacter` 수정: UCombatComponent + UBattleAIComponent 추가
  - BeginPlay 끝: StatFormulas::DeriveStats(DefaultPrimaryStats(DefaultClassId,1),1) 결과로 Combat 초기화
  - BattleAI->TargetActorClass = AIdleMonster::StaticClass(); BattleAI->StartBattle()

- `AIdleMonster` 신규 (`CharacterSystem/IdleMonster.h/cpp`)
  - ACharacter 기반
  - 슬라임 placeholder: /Engine/BasicShapes/Sphere.Sphere, 녹색 PlaceholderMaterial 옵션
  - Combat + BattleAI 컴포넌트
  - BeginPlay: MaxHp=50, Atk=8, Def=5, AtkSpeed=1, BattleAI->TargetActorClass = AIdleCharacter::StaticClass(); StartBattle()
  - Combat->OnDeath 구독: AGoldDrop spawn (10 + RandRange(0,5)) + Destroy

### C. ItemSystem 신규
- `client/Source/IdleProject/ItemSystem/GoldDrop.h/cpp`
  - `AGoldDrop : AActor`
  - 멤버: Amount (int64), Mesh (UStaticMeshComponent, Coin/Sphere 황금색)
  - BeginPlay 0.5s 후 자동 흡수 시작
  - Tick or Timer: 가장 가까운 AIdleCharacter 로 VInterpTo
  - 거리 50 이내: UIdleGameInstance::AddGold(Amount) + Destroy

### D. GameCore 확장
- `UIdleGameInstance` 갱신:
  - 멤버: Gold (int64), CharacterLevel (int32=1), CurrentExp (int64), NextExp (int64 = ExpToNext(1))
  - AddGold / AddExp / LevelUp 메서드 + OnGoldChanged / OnExpChanged / OnLevelUp 델리게이트
  - Init() 에서 UApiClient::RegisterGuest() 비동기 호출

### E. GameMode 확장
- `AIdleProjectGameModeBase` 갱신:
  - MonsterClass (TSubclassOf<AIdleMonster>), InitialMonsterCount (=5), MonsterRespawnDelay (=5.0f)
  - HandleStartingNewPlayer override → SpawnInitialMonsters()
  - SpawnInitialMonsters: 캐릭터 위치 반지름 800 안 5 지점 spawn
  - 각 몬스터의 OnDeath 에 ScheduleRespawn 구독 → 5초 후 동일 위치 재spawn

### F. NetworkClient
- `UApiClient` 갱신:
  - `void RegisterGuest(TFunction<void(bool, FString)> Callback)`
  - uuid 기반 email/password 자동 생성, POST /v1/auth/register
  - 토큰 보관 (멤버 또는 GameInstance)
  - Graceful failure (서버 미기동 시 로그만, 게임 진행 차단 X)
- 서버 base URL: GConfig (DefaultEngine.ini) 또는 default `http://localhost:3000`

### G. UI - HUD 베이스만
- `client/Source/IdleProject/UI/IdleHUD.h/cpp`
  - `AHUD` 상속
  - PostInitializeComponents() 에서 GameInstance OnGoldChanged/OnExpChanged 구독
  - Slate Widget pointer 골격만 (실 Slate 구현은 보조 호출)

### H. INI 갱신
- `client/Config/DefaultEngine.ini` [/Script/IdleProject.IdleProjectGameModeBase]:
  - `HUDClass=/Script/IdleProject.IdleHUD`
  - `DefaultPawnClass=/Script/IdleProject.IdleCharacter`

### I. Tests
- `client/Source/IdleProject/Tests/CombatTests.cpp` (3건):
  - ComputeDamage(100, 20) = 88
  - ComputeDamage(10, 100) = 0.5 (최소 보장)
  - ComputeDamage(50, 0) = 50
- (선택) AutoBattleSimTests.cpp — 단위 시뮬레이션 (캐릭터 + 몬스터 직접 객체)

## 자기 검증 (커밋 전)
- [ ] 모든 UENUM 에 None = 0
- [ ] TestEqual float `.0f` / int64 `static_cast<int64>()` 명시
- [ ] include 경로 모듈 루트 기준
- [ ] UPROPERTY/UFUNCTION 매크로 정합
- [ ] DECLARE_DYNAMIC_MULTICAST_DELEGATE 매개변수 BP 호환
- [ ] 한글 주석

## 커밋
- 6~8 commit, prefix `codex(character):`
- 예시:
  - `codex(character): CombatComponent + CombatFormulas 추가`
  - `codex(character): BattleAIComponent 자동 전투 AI`
  - `codex(character): AIdleMonster 슬라임 placeholder`
  - `codex(character): AGoldDrop 자동 흡수`
  - `codex(character): UIdleGameInstance Gold/Exp/Level`
  - `codex(character): GameMode 몬스터 spawn + respawn`
  - `codex(character): UApiClient guest register 첫 호출`
  - `codex(character): UE Automation 전투 테스트 + HUD 베이스`

## 푸시
모든 commit 후 `git push origin plan/06-auto-battle-v1`

## 범위 외 (절대 금지)
- 스킬 / 인벤토리 / 강화 / 오프라인 보상 / 환생 / 다직업 / 스토리 / 퀘스트
- BP/.uasset
- Steam SDK / 사운드 / Niagara (옵션 — 보조 호출)

## 완료 출력
```
## Codex PR #6 캐릭터 메인 결과
### 추가/수정 파일 + 커밋 목록
### 주요 구현
### 자기 검증
### 알려진 한계
```

작업 디렉터리 `C:\game\idle game\repo`, 브랜치 `plan/06-auto-battle-v1`. 이제 시작하세요.
