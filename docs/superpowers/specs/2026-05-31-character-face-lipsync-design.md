# 캐릭터 얼굴 생동 + 립싱크 스캐폴드 설계+계획

- 작성일: 2026-05-31
- 상태: 설계 승인됨(사용자 "구현 요청") → 구현
- 분류: 프론트엔드(UE5 CharacterSystem). **클라 전용 — 서버/세이브/parity 무관(SaveVer 29 무변경).**
- 브랜치: `feat/character-face-lipsync` (main 분기).
- 상위: [[project-visual-overhaul]]. 캐릭터(Seed-san VRoid, VRM4U) 생동화. 슬라이스 3(애님 스캐폴드) 후속.

## 1. 배경 / 목표

VRoid 아바타 Seed-san이 이미 게임에 등장(T포즈, 무애님). `FacialExpressionComponent`가 `Mesh->SetMorphTarget(이름, 값)`으로 VRM 블렌드셰이프(Joy/Angry/Sorrow/Fun/Surprised)를 구동. VRM 0.x 표준엔 **Blink, A/I/U/E/O(입모양)** 도 존재 → **표정·깜빡임·립싱크를 전부 코드로 구동 가능**(에셋 무의존). 몸 동작(이동/공격)은 별도 에셋 경로(후속).

### 목표
1. **자동 깜빡임**: 주기적 Blink 모프 구동(살아있는 느낌).
2. **표정**: 기존 상태별 표정 유지(강화 선택).
3. **립싱크 스캐폴드**: `A/I/U/E/O` 입모양 모프를 음성 진폭으로 구동 → **추후 TTS 오디오 연결 시 입이 자동으로 움직임**.

### 비목표
- 몸 본 애니메이션(이동/공격) — 에셋/AnimBP 후속.
- 실제 TTS 엔진 통합(여기선 진폭 입력 인터페이스 + 디버그 구동만).

## 2. 순수 로직 (`CharacterSystem/CharacterFaceMotion.{h,cpp}`, `IdleProject::Character`)

```cpp
// 깜빡임 가중치: Elapsed(깜빡임 시작 후 경과) → 0..1(1=완전 감음). 삼각: 0→1→0, BlinkDuration에 걸쳐.
float ComputeBlinkWeight(float Elapsed, float BlinkDuration);
// 입 벌림 평활: Target(목표 진폭 0..1) 향해 Prev에서 접근. 상승=Attack, 하강=Release 속도. 0..1 클램프.
float ComputeMouthOpen(float Target, float Prev, float DeltaSeconds, float AttackPerSec, float ReleasePerSec);
```
- `ComputeBlinkWeight`: `t=Elapsed/BlinkDuration`(0..1), `weight = 1 - |2t-1|`(삼각, 중간 피크 1). Elapsed≥Duration이면 0.
- `ComputeMouthOpen`: `rate = (Target>Prev)?Attack:Release`; `Prev ± rate*Dt` 를 Target으로 클램프 접근. 0..1.
- 단위 테스트: 깜빡임 중간=1·끝=0, 입벌림 상승/하강/클램프.

## 3. 립싱크 컴포넌트 (`UCharacterLipSyncComponent : UActorComponent`)

- `void SetSpeechAmplitude(float Amplitude01)` — 외부(추후 TTS)에서 매 프레임 0..1 입력. 캐시.
- `UPROPERTY bool bDebugOscillate=false` — true면 사인파로 자체 진폭 생성(에셋/TTS 없이 입 움직임 시연/검증용).
- `TickComponent`: Target=(bDebugOscillate? 사인 : 캐시 진폭). `MouthOpen = ComputeMouthOpen(...)`. 타깃 메시(소유자의 USkeletalMeshComponent)에 `SetMorphTarget(TEXT("A"), MouthOpen)` (턱 벌림 주도). 진폭 0이면 닫힘.
- 메시 탐색은 `FacialExpressionComponent` 패턴(`Owner->FindComponentByClass<USkeletalMeshComponent>()`) 재사용. 메시/모프 없으면 무동작(폴백 안전).
- `AIdleCharacter` 생성자에서 `CreateDefaultSubobject`로 부착.

## 4. 자동 깜빡임 (`FacialExpressionComponent` 확장)

- `PrimaryComponentTick.bCanEverTick=true`, `TickComponent` 추가.
- 멤버: `float BlinkTimer`, `float NextBlinkInterval`(2~6s 랜덤), `bool bBlinking`, `float BlinkElapsed`. `BlinkDuration≈0.12s`.
- 매 틱: 타이머 누적 → 간격 도달 시 깜빡임 시작. 깜빡임 중 `ComputeBlinkWeight` → `SetMorphTarget(TEXT("Blink"), w)`(+ "Blink_L"/"Blink_R" 있으면 동일). 끝나면 정지 + Blink=0 + 다음 간격 랜덤.
- 표정 morph(Joy/Angry/…)와 Blink는 다른 모프라 충돌 없음. `ApplyBlendShapes`의 리셋 목록에 Blink 미포함(또는 깜빡임이 매 틱 재적용).

## 5. 영향 / 리스크

- 클라 전용, **SaveVer 29 무변경**, 서버 무관.
- 모프 없으면 `SetMorphTarget` 무동작(폴백) → Seed-san에 A/I/U/E/O/Blink 모프 없을 시에도 회귀 0.
- 검증: 순수 로직 단위 테스트. 모프 적용은 `bDebugOscillate`로 PIE 클로즈업(입 벌림) 확인. 실제 립싱크는 TTS 진폭 연결 후.
- jumbo ODR 주의.

## 6. 구현 태스크
1. **순수 로직** CharacterFaceMotion.{h,cpp} + 테스트(깜빡임/입벌림).
2. **립싱크 컴포넌트** UCharacterLipSyncComponent + 캐릭터 부착 + 디버그 사인. 빌드+회귀.
3. **자동 깜빡임** FacialExpressionComponent 틱 + 최종 회귀. (선택: PIE 입벌림/깜빡임 클로즈업 검증)

## 7. 추후 TTS 연결(비목표지만 인터페이스 명시)
- TTS가 `USoundWave`/스트림 재생 시, 오디오 엔벨로프(진폭)를 매 프레임 `LipSync->SetSpeechAmplitude(amp)`로 전달 → 입 자동. 음소(viseme) 기반 A/I/U/E/O 분배는 후속 확장 지점(현재 A 주도).
