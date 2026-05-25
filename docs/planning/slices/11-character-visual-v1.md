# PR #11 기획서 — 캐릭터 비주얼 V1 (3D 애니메 풍 / VRoid + VRM4U + Mixamo)

> **GDD 스타일 변경 결정** (2026-05-25 사용자 명시): PR #1 의 도트 풍 → **3D 스타일라이즈드 애니메 풍** 으로 전환.
> 메이플 키우기 풍 카툰/일본 RPG 분위기에는 도트 또는 셀쉐이딩 3D 가 자연스러움.

---

## 1. 목표 / DoD

UE5 5.7 에디터에서 PIE 시작 시:
1. 캐릭터 큐브 placeholder → **VRoid 미소년/미소녀 Skeletal Mesh** 로 교체
2. **Idle / Walking / Sword Slash / Hit React / Death** 애니메이션 자동 재생
3. **표정 변화** — 전투 중 (Battle), 피격 (Hit), 사망 (Dead), 평상시 (Idle) 별로 Morph Target weight 변경
4. 자동 전투 시 캐릭터가 Walking 으로 추격 + 사거리 도달 시 Sword Slash 트리거
5. 슬라임 사망 시 캐릭터가 짧은 Smile 표정 (1초)
6. 카메라 SpringArm 거리 + 각도 조정 (캐릭터 크기 ~180cm 기준)

### DoD 검증 시나리오
- VRoid 미소년/미소녀 SK 가 게임 시작 시 화면에 등장
- 슬라임 추격 시 Walking 애니메이션
- 공격 시 Sword Slash + 분노 표정
- 슬라임 사망 시 캐릭터 짧은 미소 (1초 후 Idle 복귀)
- 피격 시 Hit React + 고통 표정 (0.5초)
- Output Log: `[FacialExpression] Battle → Smile → Idle` 등 상태 전환

## 2. 범위 (In Scope)

### 2.1 인프라
- `client/Plugins/VRM4U/` (이미 PM 자동 설치, .gitignore — 사용자 환경 전용)
- VRoid Studio (사용자 PC 설치, `winget install pixivInc.VRoidStudio`)
- Mixamo 무료 애니메이션 (사용자 다운로드)

### 2.2 CharacterSystem 갱신
- `AIdleCharacter` 변경:
  - `UStaticMeshComponent* PlaceholderMesh` → **`USkeletalMeshComponent* CharacterMesh`** 로 교체
  - SK_Mesh / AnimInstance / Skeleton 경로는 INI (`/Script/IdleProject.IdleCharacter.SkeletalMeshPath`) 또는 DataAsset 으로 받음
  - 캐릭터 크기 조정 (Capsule 88 → ~90, Mesh offset 조정)
- 신규 `client/Source/IdleProject/CharacterSystem/FacialExpressionComponent.h/cpp`:
  - `UENUM EFacialExpression : uint8 { None=0, Idle, Battle, Smile, Hit, Death, LevelUp }`
  - `UFacialExpressionComponent : UActorComponent`
  - `void SetExpression(EFacialExpression NewExpression, float Duration = -1.0f)` (Duration > 0 면 자동 Idle 복귀)
  - 내부: Skeletal Mesh 의 Morph Target weight 매핑 (예: Smile → "BlendShape.Smile" weight 1.0)
  - VRoid 의 ARKit 호환 blend shape 이름 사용 (`Joy`, `Angry`, `Sorrow`, `Fun`, `Surprised`)

### 2.3 애니메이션 통합
- `client/Source/IdleProject/CharacterSystem/IdleAnimInstance.h/cpp`:
  - `UAnimInstance` 베이스 클래스
  - 변수: bool bIsMoving, bool bIsAttacking, bool bIsDead, float MovementSpeed
  - State Machine 은 사용자가 BP_AnimInstance 로 작성 (C++ 만으로는 State Machine 어려움)
  - C++ 변수만 노출 → BP 가 State Machine 에서 참조
- AIdleCharacter.cpp 에서 매 Tick (또는 BattleAI 콜백) 으로 변수 갱신:
  - 이동 중: bIsMoving = true, MovementSpeed = GetVelocity().Size()
  - Attack 트리거: bIsAttacking = true (1프레임)
  - HP 0: bIsDead = true

### 2.4 데이터 / 설정
- `client/Config/DefaultEngine.ini` 에 캐릭터 자산 경로 추가:
  ```ini
  [/Script/IdleProject.IdleCharacter]
  ; 사용자가 VRoid + VRM4U import 후 경로 입력
  SkeletalMeshPath=/Game/Characters/Soyeon/SK_Soyeon.SK_Soyeon
  AnimInstanceClassPath=/Game/Characters/Soyeon/ABP_Soyeon.ABP_Soyeon_C
  ```
- 또는 더 안전: `UDataAsset IdleCharacterDataAsset` 으로 DataAsset 기반 (BP 작업 필요)

### 2.5 GDD / Art Direction 갱신
- `docs/planning/01-game-design.md §1.1`:
  - "도트 풍 횡스크롤" → "**3D 스타일라이즈드 애니메 풍 횡스크롤 / 사이드뷰**"
  - "픽셀 아트" → "셀쉐이딩 3D"
- `docs/planning/04-art-direction.md`:
  - 캐릭터 비율: 8헤드 (1:7~8) — VRoid 표준
  - 사이즈: SK Mesh ~180cm (UE unit 180)
  - 픽셀 8x96 px 폐기
  - 표정: ARKit 53 blend shapes (VRoid 일부 매핑)
  - 컬러 토큰 유지 (UI 만 픽셀 풍 또는 셀쉐이딩 매칭)

### 2.6 UI / 카메라 조정
- `AIdleCharacter` 의 SpringArm:
  - TargetArmLength: 850 → **600** (캐릭터 보기 좋게 더 가까이)
  - SocketOffset (0, 0, 100) — 캐릭터 상체 중심으로
- 카메라 FOV 조정 검토

### 2.7 테스트
- `client/Source/IdleProject/Tests/FacialExpressionTests.cpp`:
  - SetExpression 결정성 (같은 입력 → 같은 출력)
  - Duration 자동 Idle 복귀
  - Hit / Death enum 매핑

## 3. 범위 외 (Out of Scope)

| 항목 | 시점 |
| --- | --- |
| 캐릭터 의상 교체 시스템 | 후속 PR |
| 머리 / 헤어 색상 게임 내 변경 | 1.0 |
| Mocap / Lip sync | 1.0 (사운드 슬라이스) |
| 컷씬 카메라 | M4 (스토리) |
| 캐릭터 직업 별 다른 모델 (전사/마법사/궁수) | PR #12 (다직업) |
| 멀티 캐릭터 (펫) | M5 |
| Niagara 이펙트 (공격 트레일, 스킬 이펙트) | M5 |
| LOD (멀리서 폴리곤 감소) | 출시 직전 최적화 |

## 4. 7파트 작업 분배

| 파트 | 작업 | Codex 호출 |
| --- | --- | --- |
| **캐릭터·아이템·능력치 (메인)** | AIdleCharacter 변경 + FacialExpressionComponent + IdleAnimInstance + 카메라 조정 + Tests | ✅ 메인 |
| 디자이너 | INI 의 캐릭터 자산 경로 + Art Direction 갱신 + 비율/사이즈 표 | ✅ 보조 |
| 밸런스 | (해당 없음 — 비주얼만) | ❌ |
| 백엔드·DB | (해당 없음) | ❌ |
| 스토리 | 캐릭터 이름 placeholder 정리 (06-story-bible.md 의 주인공 정의에 맞춰) | ✅ 보조 (작음) |
| 퀘스트 | N/A | ❌ |
| QA | docs/qa/scenarios/M2-character-visual-v1.md + 회귀 §1 갱신 | ✅ 보조 |

→ Codex 실호출 **2회** (캐릭터 메인 + 보조 3 합동)

### 사용자 작업 의존
- **사용자가 VRoid 캐릭터 디자인 + Mixamo 애니메이션 import + AnimBP 생성** 완료해야 PR 완전 검증 가능
- PM 코드 골격은 **자산 경로 미정 상태에서도 컴파일 가능** 하도록 작성 (자산 nullptr 시 fallback to PlaceholderMesh 큐브)

## 5. 호출 순서

1. **캐릭터 메인** — Skeletal Mesh / AnimInstance / FacialExpression 코드 골격 (~30분)
2. **보조 3 합동** — INI 갱신 / Art Direction / QA 시나리오 / 스토리 캐릭터 placeholder (~15분)

## 6. 워크플로우 v3 (학습 반영)
- 단계 [2] 후 PM 산출 게시
- 단계 [3] Claude 리뷰 + fix
- 단계 [4] Codex 리뷰 + fix + PM 산출 게시
- 단계 [5] Claude 검증 + (사용자 자산 import 완료 시) PIE 검증
- 단계 [N] PM 종합 + 머지

## 7. 일정 (잠정)

- [1] 본 문서 + GDD/Art Direction 갱신 → 30분
- [2] Codex 2회 → 45분
- [3]~[5] 30분
- [N] 머지 → 5분
- **사용자 작업 (병행)**: VRoid 캐릭터 ~1시간 + VRM4U import ~10분 + Mixamo 애니 ~30분 + AnimBP ~30분 → **약 2시간**
- **총: 약 3~4시간** (사용자 작업 병행 시)

## 8. 리스크

| 항목 | 위험 | 대응 |
| --- | --- | --- |
| VRoid VRM 의 blend shape 이름이 ARKit 비표준 | 중 | VRM4U 의 표준 매핑 (Joy/Angry/Sorrow/Fun/Surprised) + 코드에서 양쪽 시도 |
| Mixamo FBX retarget 시 일부 본 mismatch | 중 | UE5 IK Retargeter 사용 (사용자 BP 작업) |
| AnimBlueprint State Machine 은 BP 작업 필요 — Codex 가 못 만듦 | 높음 | 본 PR 단계: C++ 베이스 (UAnimInstance + 변수만) + 사용자가 ABP_Soyeon BP 작성. PM 이 BP 작성 가이드 제공 |
| 카메라 사이드뷰 거리 / 각도가 캐릭터 너무 가깝거나 멀음 | 중 | 사용자 PIE 검증 후 INI 의 SpringArm 값 hotfix |
| 사용자 자산 경로 변경 시 코드 hardcode | 중 | INI 또는 DataAsset 으로 외부화, 사용자가 자산 import 후 INI 갱신 commit |

## 9. 후속 PR 예고
- **PR #12 (M2 스킬 트리)** — 전사 액티브 4 + 패시브 2 + 궁극기 1 + 스킬 이펙트
- **PR #13 (다직업)** — 마법사 / 궁수 + 각자 VRoid 캐릭터
- **PR #14 (사운드)** — BGM + SFX + Voice (선택)

## 10. 사용자 작업 가이드 요약

### A. VRoid Studio 설치 + 캐릭터 디자인 (~1시간)
1. `winget install pixivInc.VRoidStudio` (PowerShell 관리자)
2. 미소년/미소녀 1명 디자인
3. Export → VRM (Reduce poly 활성, Bone count 유지, Blend shapes 활성)
4. 저장 경로: `client/Content/Characters/<Name>.vrm`

### B. VRM4U import (~10분)
1. UE5 에디터 재시작 (Plugin 인식)
2. Content Browser → `Content/Characters/` → VRM 파일 drag-drop
3. import 다이얼로그 OK → 자동 생성: SK_<Name>, <Name>_Skeleton, MI_*, BP_<Name>

### C. Mixamo 애니메이션 (~30분)
1. https://mixamo.com → Adobe 무료
2. Idle, Walking, Sword Slash, Hit React, Death 검색 → 다운로드 (FBX, "Without Skin")
3. UE5 import → Skeleton: VRoid 스켈레톤 선택 (retarget)
4. 위치: `Content/Characters/Animations/A_Idle.A_Idle` 등

### D. AnimBlueprint 작성 (~30분)
1. Content Browser → `Content/Characters/<Name>/` 에서 `ABP_<Name>` 신규 (Anim Blueprint, Parent: UIdleAnimInstance)
2. Anim Graph → State Machine 추가:
   - States: Idle, Walking, Attacking, Dying
   - Transitions: bIsMoving → Walking, bIsAttacking → Attacking, bIsDead → Dying
3. 각 State 에 해당 애니메이션 connect

### E. INI 갱신 (PM 이 commit, 사용자가 경로 알려줌)
```ini
[/Script/IdleProject.IdleCharacter]
SkeletalMeshPath=/Game/Characters/Soyeon/SK_Soyeon.SK_Soyeon
AnimInstanceClassPath=/Game/Characters/Soyeon/ABP_Soyeon.ABP_Soyeon_C
```

본 PR 머지 후 메모리 갱신.
