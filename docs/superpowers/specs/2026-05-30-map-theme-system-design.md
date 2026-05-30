# 맵 테마 시스템 (챕터별 시각 환경 차별화) 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(하이브리드, 절차적 프롭 우선) → plan 단계화
- 분류: 프론트엔드(UE5 절차 씬 테마). **클라 전용 시각 — 서버/세이브/parity 무관.**
- 선행: 챕터 8까지(#103). 브랜치 `feat/map-theme-system`.

## 1. 배경

게임 씬은 `AIdleProjectGameModeBase::SpawnDefaultEnvironment()`가 **코드로 절차 생성**한다: DirectionalLight(태양, 5500K 따뜻한색) + SkyLight(환경광) + Engine `Plane` 바닥 50배. 모든 챕터/스테이지가 **동일한 단일 평면 아레나**를 공유한다. 맵 이름/분위기는 스토리 키(`STORY_MAP_C0*`)에 로어로만 존재하고 시각엔 반영되지 않는다.

"맵 상세구현"(사용자) = 챕터별로 **다른 장소 느낌**을 주는 시각 환경 차별화. 풍부한 환경(메시/프롭) 방향, **하이브리드**(절차적 프롭 우선 → 후속 임포트 에셋 교체).

## 2. 목표 / 비목표

### 목표
1. 챕터별 **맵 테마**(조명 + 바닥색 + 프롭 레이아웃) 데이터 구동.
2. GameMode가 챕터 변경(`StageService::OnStageChanged`) 시 테마 적용/전환.
3. 프롭은 **Engine 기본 셰이프**(스타일라이즈드) — soft 메시 경로로 **후속 임포트 에셋 교체 가능**(하이브리드).
4. 헤드리스 Automation 회귀(조명/프롭 적용).

### 비목표
- 임포트 환경 아트 에셋(후속 하이브리드 B — soft 경로 교체로 시스템 무변경).
- 서버/세이브/parity 변경(시각 전용).
- 실제 .umap 수작업 레벨 디자인(절차 생성 유지).
- 게임플레이 영향 지형/장애물/충돌(시각 데코만, 프롭은 NoCollision).

## 3. 데이터 모델 (클라 C++)

```
// 단일 데코 프롭. Mesh 는 soft 경로(지금=Engine 기본 셰이프, 후속=임포트 에셋 교체=하이브리드).
USTRUCT FMapProp {
  TSoftObjectPtr<UStaticMesh> Mesh;   // 예: /Engine/BasicShapes/Cylinder.Cylinder
  FVector  Location;
  FRotator Rotation;
  FVector  Scale = (1,1,1);
  FLinearColor Color = White;          // best-effort 틴트(파라미터 머티리얼 있을 때). 시각용
}

// 챕터 1개 테마. 조명이 주 무드 동인(완전 코드·테스트 가능), 프롭이 실루엣 변화.
USTRUCT FMapTheme {
  FLinearColor SunColor;
  float        SunIntensity;
  FRotator     SunRotation;
  float        SkyIntensity;
  FLinearColor SkyColor;
  FLinearColor GroundColor;            // best-effort 틴트
  TArray<FMapProp> Props;
}
```

테마 제공: `FMapThemeLibrary::GetTheme(int32 Chapter) -> FMapTheme` (챕터 1~8, 범위 밖은 가장 가까운 챕터로 클램프). 8 팔레트는 스토리 분위기 매칭(plan에서 구체 값 확정).

## 4. 적용 (GameMode)

- `SpawnDefaultEnvironment()`가 spawn한 **Sun/Sky/Ground 액터 참조를 멤버로 보관**(`TObjectPtr`), 프롭 액터는 `TArray<TObjectPtr<AStaticMeshActor>>`.
- `ApplyMapTheme(int32 Chapter)`:
  - Sun: `SetLightColor(SunColor)`, `SetIntensity`, `SetWorldRotation(SunRotation)`, 온도 사용 해제(`SetUseTemperature(false)`).
  - Sky: `SetIntensity(SkyIntensity)`, `SetLightColor(SkyColor)`.
  - Ground: 동적 머티리얼 인스턴스(MID) 생성 후 `Color` 벡터 파라미터 `GroundColor` 설정(파라미터 머티리얼 부재 시 no-op, 시각만).
  - Props: **기존 프롭 액터 전부 Destroy** 후 테마 `Props`를 spawn(StaticMeshActor + soft 메시 동기 로드, 트랜스폼, NoCollision, MID 틴트 best-effort).
- **챕터 변경 감지**: GameMode 초기화 시 `StageService::OnStageChanged`에 바인딩. 콜백에서 `NewStageInfo.Chapter`가 직전 적용 챕터와 다르면 `ApplyMapTheme(Chapter)`. 초기 진입 시 현재 챕터 1회 적용.
- 환경 spawn은 1회(`bDefaultEnvironmentSpawned`), 테마 적용은 챕터마다 갱신.

## 5. 색상 메커니즘 (정직한 범위)
- **주 무드 = 조명 색/강도**(Sun/Sky) — 완전 코드, 헤드리스 테스트 가능. 챕터마다 전체 톤이 바뀜.
- **프롭/바닥 정밀 색 = best-effort MID 틴트** — 파라미터 머티리얼(`Color` 벡터 파라미터)이 있을 때만 시각 반영. MVP는 파라미터 머티리얼 없이도 동작(틴트 no-op, 조명 틴트로 무드 확보). 후속(하이브리드 B 또는 디자이너 머티리얼 도입) 시 프롭별 색 강화.
- 프롭 실루엣(기본 셰이프 조합) + 조명 톤으로 "다른 장소" 느낌을 MVP에서 확보.

## 6. 8 챕터 팔레트 방향 (plan에서 RGB·프롭 확정)
- ch1 초원/자연(밝은 따뜻한 녹색, 원기둥+콘 나무) → ch2 숲 심부(짙은 녹) → ch3 차원 그림자(어두운 보라, 각진 큐브 #66 Dark) → ch4 → ch5 심연 옥좌(#93, 짙은 청흑) → ch6 무너지는 근원(붕괴, 회보라) → ch7 균열(#98, 불길 적색, 떠 있는 뾰족 파편) → ch8 후존계(#103, 차가운 청백 정적, 부서진 기둥).

## 7. 테스트 / 게이트 (헤드리스)
- `MapThemeTests`(신규 Automation):
  - `FMapThemeLibrary::GetTheme(c)` 챕터 1~8 유효 + 범위 클램프.
  - 헤드리스 월드에서 GameMode 환경 spawn 후 `ApplyMapTheme(c)` → Sun 라이트 색/강도/회전, Sky 강도 기대값 설정 확인.
  - 챕터 변경 시 프롭 액터 수가 새 테마 `Props.Num()`로 교체(이전 Destroy) 확인.
- 표준 jumbo 빌드 + 전체 Automation(신규 MapTheme 포함). **세이브·서버 무변경**(SaveVer 29 유지).

## 8. 안전 가드
- soft 메시 로드 실패 시 해당 프롭 skip(크래시 없음).
- 프롭 NoCollision(게임플레이 무간섭, 몬스터/캐릭터 이동 영향 0).
- 챕터 클램프(1~8, 미래 챕터 추가 시 가장 가까운 테마).
- 환경 1회 spawn 가드 유지. 테마 재적용은 멱등(프롭 전체 교체).
- 시각 전용 — 세이브/서버/밸런스 무변경.

## 9. 구현 단계화 (plan)
- `MapThemeTypes.h`(FMapProp/FMapTheme) + `MapThemeLibrary.h/.cpp`(8 테마 데이터).
- GameMode: Sun/Sky/Ground/Props 참조 보관 + `ApplyMapTheme` + OnStageChanged 바인딩 + 초기 적용.
- `MapThemeTests.cpp`(라이브러리 + 적용 + 챕터 전환 회귀).
- 게이트: 표준 jumbo + Automation, 세이브 무변경 확인.

## 10. 후속
- 하이브리드 B: 원하는 챕터부터 `FMapProp.Mesh` soft 경로 → 임포트 에셋(시스템 무변경).
- 파라미터 머티리얼(`M_MapTheme`) 도입 → 프롭/바닥 정밀 색.
- 안개(ExponentialHeightFog) 테마, 스카이 큐브맵, 스테이지별(챕터 내) 세분.
