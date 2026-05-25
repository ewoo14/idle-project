# PR #11 AnimBlueprint Setup Guide

이 PR은 C++에서 `UIdleAnimInstance` 변수와 런타임 자산 경로만 제공합니다. AnimBlueprint State Machine과 VRM import 자산은 사용자가 UE 에디터에서 생성해야 합니다.

## 1. VRoid / VRM4U Import

1. VRoid Studio에서 캐릭터를 VRM으로 export합니다.
2. UE 에디터에서 VRM4U plugin이 활성화된 상태로 `Content/Characters/Soyeon/`에 VRM을 import합니다.
3. import 결과로 생성된 Skeletal Mesh 경로를 확인합니다.

예시:

```ini
SkeletalMeshPath=/Game/Characters/Soyeon/SK_Soyeon.SK_Soyeon
```

## 2. AnimBlueprint

1. `Content/Characters/Soyeon/`에 Anim Blueprint를 생성합니다.
2. Parent Class는 `IdleAnimInstance`로 설정합니다.
3. State Machine에 최소 상태를 구성합니다.
   - Idle
   - Walking
   - Attacking
   - Dying
4. 전이 조건은 C++ 변수를 사용합니다.
   - `bIsMoving`: Walking 진입
   - `bIsAttacking`: Attacking 진입
   - `bIsDead`: Dying 진입
   - `MovementSpeed`: BlendSpace를 사용할 때 속도 입력

예시:

```ini
AnimInstanceClassPath=/Game/Characters/Soyeon/ABP_Soyeon.ABP_Soyeon_C
```

## 3. Fallback

`SkeletalMeshPath` 또는 `AnimInstanceClassPath`가 비어 있거나 로드에 실패해도 게임은 컴파일/실행됩니다. 이 경우 기존 `PlaceholderMesh` 큐브가 표시되고 표정 Morph Target 적용은 no-op 처리됩니다.

## 4. Facial Blend Shape

기본 매핑은 VRoid/VRM4U에서 흔히 쓰는 이름을 기준으로 합니다.

| Expression | Morph Targets |
| --- | --- |
| Battle | `Angry=0.7` |
| Smile | `Joy=1.0`, `Fun=0.5` |
| Hit | `Sorrow=0.8`, `Angry=0.3` |
| Death | `Sorrow=1.0` |
| LevelUp | `Joy=1.0`, `Surprised=0.8` |

VRoid 버전이나 import 옵션에 따라 Morph Target 이름이 다를 수 있으므로 실제 import 후 필요하면 코드 매핑을 조정합니다.
