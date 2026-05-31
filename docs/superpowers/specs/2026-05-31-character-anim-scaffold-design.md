# 캐릭터 애니메이션 C++ 구동 스캐폴드 (슬라이스 3) 설계+계획

- 작성일: 2026-05-31
- 상태: 설계 승인됨 → 구현
- 분류: 프론트엔드(UE5 CharacterSystem). **클라 전용 — 서버/세이브/parity 무관(SaveVer 29 무변경).**
- 브랜치: `feat/character-anim` (main 분기 — HUD/비주얼 슬라이스와 독립).
- 상위 비전: [[project-visual-overhaul]] 로드맵 슬라이스 3.

## 1. 배경 / 목표

캐릭터 메시 스왑은 이미 배선됨(`[/Script/IdleProject.IdleCharacter] SkeletalMeshPath` → 자산 있으면 큐브→실제 메시, VRM4U/VRoid). `UIdleAnimInstance`에 상태 변수(bIsMoving/bIsAttacking/bIsDead/MovementSpeed)가 `Tick`→`UpdateAnimInstanceVariables`(IdleCharacter.cpp:556)에서 갱신되나 **재생 로직 없음**(비주얼 AnimBP 의존 설계). 목표: **비주얼 AnimBP 없이 C++로 시퀀스 재생** — 사용자가 메시+애님 시퀀스를 임포트하면 즉시 동작, 없으면 현 동작 폴백.

### 비목표
- 메시/애님 *자산* 제작·임포트(사용자 수동).
- 비주얼 AnimBlueprint.
- 전투 공식/세이브 변경.

## 2. 재생 방식

`USkeletalMeshComponent::PlayAnimation(UAnimSequence*, bLooping)` (싱글노드 애님 모드 — AnimBP 불필요). 상태 변화 시에만 호출(루프 대기/이동, 원샷 공격/피격→완료 후 복귀, 사망=정지 프레임 유지).

## 3. 상태 선택(순수 로직, 테스트 대상)

신규 `CharacterSystem/CharacterAnimState.h` (namespace `IdleProject::Character`):
```cpp
enum class ECharAnimState : uint8 { Idle, Move, Attack, Hit, Death };
// 우선순위: Death > (Attack|Hit 원샷 재생 중) > Move > Idle.
ECharAnimState SelectAnimState(bool bDead, bool bOneShotPlaying, bool bAttackRequested, bool bHitRequested, bool bMoving);
```
- 사망이면 Death. 원샷(공격/피격) 재생 중이면 그 상태 유지(중단 안 함). 새 공격/피격 요청이면 해당 원샷. 아니면 이동/대기.
- 단위 테스트: 우선순위·원샷 보존·복귀.

## 4. 캐릭터 통합 (`IdleCharacter`)

- **config 로드**(메시 로드 인근, ApplyConfig): `[IdleCharacter]` 키 `IdleAnimPath`/`MoveAnimPath`/`AttackAnimPath`/`HitAnimPath`/`DeathAnimPath` → `StaticLoadObject<UAnimSequence>`(빈/실패=nullptr). 멤버 `TObjectPtr<UAnimSequence>` 5종 + `ECharAnimState CurrentAnimState`.
- **Tick 구동**(UpdateAnimInstanceVariables 인근 또는 신규 `UpdateLocomotionAnimation`): `SelectAnimState(...)` → desired. desired≠current 이고 해당 시퀀스 non-null이면 `CharacterMesh->PlayAnimation(Seq, bLoop)`(Idle/Move=loop, Attack/Hit/Death=once). 원샷 완료 판정: `CharacterMesh->GetSingleNodeInstance()->GetTimeRemaining()<=0` 또는 경과시간≥길이 → 원샷 플래그 해제.
- **전투 훅**: 기존 `Combat->OnDeath`→사망. 공격=BattleAI State==Attack 진입 에지에서 공격 원샷 요청. 피격=Combat 피격 델리게이트 있으면 연동(없으면 생략/후속).
- **폴백**: 어떤 시퀀스도 없으면 PlayAnimation 미호출(메시 기본/T포즈), 메시 없으면 큐브(기존).

## 5. 표정(선택)

`FacialExpressionComponent` 존재 시 상태별 표정 호출(공격=화남/사망=무표정 등). API 확인 후 가능하면 연동, 아니면 후속.

## 6. 영향 / 리스크

- 클라 전용, **SaveVer 29 무변경**, 서버 무관.
- 자산 의존 전부 nullptr 폴백(회귀 0 — 자산 없는 현 상태에서 동작 동일).
- PlayAnimation 싱글노드 모드는 AnimInstance(상태변수) 우회 — 기존 변수 갱신은 유지(후속 AnimBP 호환), 단 재생은 PlayAnimation이 담당.
- jumbo ODR 주의. PIE 검증은 자산 임포트 후(현재는 컴파일+폴백 회귀만).

## 7. 구현 태스크

1. **순수 로직** `CharacterAnimState.{h,cpp}` + `SelectAnimState` + Automation 테스트(우선순위/원샷보존).
2. **캐릭터 통합**: config 키 + 시퀀스 로드 + Tick 구동(PlayAnimation) + 전투 훅 + 폴백. 빌드 GREEN + 전체 Automation 회귀.
3. **표정 연동**(가능 시) + DefaultEngine.ini 키 주석 추가 + 최종 회귀.

## 8. 테스트

- Automation: `SelectAnimState` 우선순위/원샷보존 단위.
- 회귀: 전체 IdleProject Automation(자산 없는 현 상태에서 폴백 동작 동일 → 회귀 0).
- 수동(자산 임포트 후): 대기/이동/공격/피격/사망 재생 PIE.
