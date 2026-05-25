# PR #6 기획서 — 자동 전투 V1 (원 슬라이스 ID: S3, M1)

> **순서 복귀 완료**: PR #4 (UE5 클라이언트 코어 부트) + PR #5 (hotfix) 머지 + 사용자 PIE 검증 (`[StatFormulas] L1 ClassId=1 STR=12.0 HP=120.0` 로그 확인) 완료. 본 PR 이 M1 두 번째 슬라이스.

---

## 1. 목표 / DoD

UE5 5.7 에디터에서 PIE 진입 시 다음이 자동 동작:
1. AIdleCharacter (전사) 가 Game 맵에 spawn
2. AIdleMonster (슬라임) 5마리가 캐릭터 주변에 spawn
3. **캐릭터가 자동으로 가장 가까운 적 탐색 → 이동 → 공격**
4. 몬스터 HP 0 → 사망 + 골드 드롭 → 캐릭터 자동 수집
5. **HUD** 에 HP / EXP / Gold 실시간 표시
6. 몬스터 사망 5초 후 자동 재spawn (무한 사냥 루프)
7. **NetworkClient** 첫 호출 — 게임 시작 시 "guest" 계정 자동 register/login (PR #2 백엔드와 첫 연동)
8. UE Automation 테스트: 자동 전투 10초 시뮬레이션 → 몬스터 사망 + 골드 누적 검증

### DoD 검증 시나리오
```powershell
# 1. UE5 에디터 열기 → PIE 시작 (Alt+P)
# 2. 화면 중앙에 전사 + 주변에 슬라임 5마리 등장
# 3. 전사가 자동으로 슬라임 추격 → 공격
# 4. 슬라임 사망 → 골드 + 보이는 곳에 +N gold 텍스트
# 5. 좌상단 HUD: HP / EXP / Gold 갱신
# 6. Output Log 에 [NetworkClient] guest 계정 register 결과 출력
```

## 2. 범위 (In Scope)

### 2.1 CombatSystem 모듈 (`client/Source/IdleProject/CombatSystem/`)
- `CombatComponent.h/cpp` — 캐릭터 / 몬스터 공통 전투 컴포넌트:
  - HP, MaxHP, Atk, Def, AtkSpeed, AttackRange
  - `void TakeDamage(float Damage, AActor* Instigator)`
  - `void OnDeath()` (Multicast Delegate)
- `CombatFormulas.h/cpp` — 데미지 계산 (서버 미러):
  - `static float ComputeDamage(float Atk, float Def)` = `max(Atk × 0.05, Atk - Def × 0.6)`
- `BattleAIComponent.h/cpp` — 자동 전투 AI:
  - `FindClosestEnemy()`, `MoveTowards(AActor*)`, `Attack(AActor*)`
  - State machine: Idle → Chase → Attack → ... → Idle (적 사망 시)
  - 캐릭터 + 몬스터 모두 사용

### 2.2 CharacterSystem 확장
- `AIdleCharacter` 갱신:
  - `UCombatComponent* Combat` 추가 (CharacterStats 기반 초기화 — StatFormulas::DeriveStats(level=1) 결과)
  - `UBattleAIComponent* BattleAI` 추가
  - BeginPlay 에서 자동 전투 모드 활성화 (Tick 또는 Timer)
- `AIdleMonster` 신규 (`CharacterSystem/IdleMonster.h/cpp`):
  - APawn 기반 (가벼움)
  - 슬라임 임시 메시 (Sphere or Cone)
  - Combat + BattleAI 컴포넌트
  - 죽으면 `AGoldDrop` spawn

### 2.3 ItemSystem (최소)
- `AGoldDrop.h/cpp` (`client/Source/IdleProject/ItemSystem/`):
  - 골드 양 (int32)
  - Tick: 가장 가까운 AIdleCharacter 로 자동 이동 (자석 효과)
  - 거리 100 이내 도달 시 캐릭터 수집 + `UIdleGameInstance::AddGold(amount)` + 자기 자신 Destroy

### 2.4 GameCore 확장
- `UIdleGameInstance` 갱신:
  - `int64 Gold`, `int32 CharacterLevel`, `int64 CurrentExp` 추가
  - `void AddGold(int64 Amount)`, `void AddExp(int64 Amount)`, `void LevelUp()`
  - HUD 갱신용 OnChanged 델리게이트
- `AIdleProjectGameModeBase` 갱신:
  - `HandleStartingNewPlayer` 에서 AIdleMonster 5마리 spawn (캐릭터 주변 반지름 800)
  - Monster 사망 시 5초 후 동일 위치 재spawn

### 2.5 UI 확장
- `client/Source/IdleProject/UI/IdleHUD.h/cpp`:
  - `AHUD` 상속
  - C++ Slate (SCompoundWidget) 로 HUD 위젯:
    - 좌상단: HP 바, EXP 바, 캐릭터 레벨
    - 우상단: Gold (숫자 + 아이콘 placeholder)
  - GameInstance OnChanged 델리게이트 구독 → 자동 갱신
- `DefaultEngine.ini` 의 GameMode 가 IdleHUD 를 HUDClass 로 설정

### 2.6 NetworkClient 첫 실 호출
- `UApiClient` 갱신:
  - `bool RegisterGuest()` — 자동 생성 email/password (uuid 기반) 로 register, 토큰 보관
  - 성공 시 `UE_LOG(LogTemp, Display, TEXT("[NetworkClient] guest registered ..."))`
  - 실패 시 (서버 미기동 등) graceful (로그만, 게임 진행 방해 X)
- `UIdleGameInstance::Init()` 에서 한 번 호출 (게임 시작 시)
- 서버 URL: `Config/DefaultEngine.ini` 에 `[/Script/IdleProject.IdleGameInstance] ServerBaseUrl=http://localhost:3000`

### 2.7 서버 미러 갱신
- `server/src/core/formulas/combat.ts` 신규:
  - `export function computeDamage(atk: number, def: number): number` (클라이언트 동일)
  - 단위 테스트 (vitest)
- `server/src/modules/character/character.routes.ts` — POST /v1/characters 가 게스트 계정 (없으면 자동 캐릭터 생성) 처리 — 또는 후속 PR 위임

### 2.8 테스트
- `client/Source/IdleProject/Tests/CombatTests.cpp`:
  - 데미지 계산 (Atk 100, Def 20 → 88) 5건
  - 최소 보장 (Atk 10, Def 100 → 0.5)
- `client/Source/IdleProject/Tests/AutoBattleSimTests.cpp`:
  - PIE 시뮬레이션: 캐릭터 1 + 몬스터 1 → 10초 → 몬스터 HP 0 + 골드 누적 검증

## 3. 범위 외 (Out of Scope)

| 항목 | 시점 |
| --- | --- |
| 스킬 시스템 (액티브 4 + 패시브 2 + 궁극기) | PR #7 (M2-S5) |
| 인벤토리 / 장비 UI | PR #8 (M2-S4) |
| 강화 / 잠재 | PR #8 |
| 오프라인 보상 | PR #9 (M3-S6) |
| 환생 | PR #11 (M4-S9) |
| 다직업 (마법사 / 궁수) | PR #12 (M5-S10) |
| 펫 | PR #13 |
| 스토리 / 퀘스트 | PR #10 (M4-S8) |
| BP / .uasset (W_MainMenu, Game.umap, IMC/IA) | 별도 chore PR (또는 본 PR 머지 후 PM 에디터 작업) |
| 사운드 / Niagara 이펙트 | 사운드 M5, Niagara 부분 본 PR 에서 1건 시도 (몬스터 사망 시 미니 이펙트 — 옵션) |
| Steam SDK | PR #14 (M6) |
| 다국어 | 1.0 |

## 4. 7파트 작업 분배 — Codex 호출 계획

| 파트 | 작업 | Codex 호출 |
| --- | --- | --- |
| **백엔드·DB** | `core/formulas/combat.ts` + 단위 테스트 + character.service 가 guest register 처리 (자동 nickname 생성) | ✅ 보조 |
| **캐릭터·아이템·능력치 (메인)** | CombatComponent + BattleAIComponent + CombatFormulas + AIdleMonster + AGoldDrop + UIdleGameInstance 확장 + AIdleCharacter 통합 + NetworkClient 첫 호출 + UE Automation 테스트 | ✅ 메인 |
| **밸런스** | `LevelCurveDB` 의 시간당 EXP/Gold 추정 표 작성 (PR #1 §2.2 1시간 ≒ 1레벨 검증) + Monster 기본 수치 (HP/Atk/Def) 결정 (`docs/planning/05-balance-philosophy.md §3.2` 미러) | ✅ 보조 |
| **디자이너** | IdleHUD Slate UI (HP/EXP/Gold) + UIThemeTokens 적용 + Niagara 미니 이펙트 (몬스터 사망 — 옵션) | ✅ 보조 |
| **QA** | `docs/qa/scenarios/M1-auto-battle-v1.md` + `regression-checklist.md` §1/§4 갱신 + UE Automation 시뮬레이션 시나리오 | ✅ 보조 |
| 스토리 | (N/A) | ❌ |
| 퀘스트 | (N/A) | ❌ |

→ 총 Codex 실호출 **2회** (캐릭터 메인 + 보조 4 합동)

## 5. 호출 순서

1. **캐릭터 메인** — CombatComponent + BattleAIComponent + AIdleMonster + AGoldDrop + Tests (~30~50분)
2. **보조 4 합동** — server combat.ts / 밸런스 표 / HUD Slate / QA 시나리오 (~15~20분)

## 6. 워크플로우 v2 (PR #2/#4 학습 반영)

- 단계 [3b]/[6b] **Codex TM 프롬프트 단순화** 유지 (PR #4 패턴)
- 단계 [6b] Codex TM 재검토 안정성 이슈 시 Claude TM 2차 + PM 검증으로 대체
- PM 머지 직전 **로컬 UE 빌드 + PIE + Output Log 검증** 의무 (사용자 액션, ~5분)

## 7. 일정 (잠정)

- [1] 본 문서 (완료)
- [2] Codex 2회 호출 → 50~70분
- [3a/3b] TM 각 10~15분
- [4]~[7] 30분
- [8] PM 빌드 검증 + 종합 소견 + 머지 → 30분
- **총: 약 2~3시간**

## 8. 리스크

| 항목 | 위험 | 대응 |
| --- | --- | --- |
| 자동 전투 Tick 성능 (5마리 + 캐릭터) | 낮음 | 0.2초 Timer (5Hz) 로 AI 갱신, 모든 Actor Tick 끔 |
| Spawn 위치 collision | 중 | SpawnLocation 검증 + 실패 시 재시도 |
| 서버 미기동 시 NetworkClient 가 게임 차단 | 중 | 비동기 호출 + timeout 5초 + 실패 graceful (로그만) |
| BP/.uasset 부재로 HUD 안 보임 | 중 | Slate UI (C++) 로 모든 UI 작성, HUDClass 등록만 INI 로 |
| Niagara 미니 이펙트 작성 어려움 | 중 | 옵션 — Sprite + Particle System 사용 또는 skip 후 후속 PR |
| Game.umap 미생성 → 빈 맵에 spawn 시 collision | 중 | DefaultEngine.ini 의 EditorStartupMap 을 빈 맵으로 두고, GameMode 가 PlayerStart 없어도 spawn 위치 직접 결정 |

## 9. 후속 PR 예고

- **PR #7 (M2-S4)** — 인벤토리 + 장비 V1 (슬롯 8 + Common/Rare + 강화 +0~+5)
- **PR #8 (M2-S5)** — 스킬 트리 V1 (전사 액티브 4 + 패시브 2 + 궁극기 1)
- **PR #9 (M3-S6)** — 오프라인 보상 (12시간 누적)
- **PR #10 (M3-S7)** — 백엔드 V2 (캐릭터/세이브 실 동기화 — 자동 전투 결과 클라우드 백업)

본 PR 머지 후 `[[project-pr-order]]` 갱신.
