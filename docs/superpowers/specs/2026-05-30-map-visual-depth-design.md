# 맵 시각 심화 — 안개 + 프롭 머티리얼 변형 + 절차 스카이 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(풀 스코프, 클릭) → plan 단계화
- 분류: 프론트엔드(UE5 클라 전용 시각). **서버/세이브/parity 무관.**
- 선행: 맵 테마 시스템(#104) + 하이브리드 B 정밀 색(#105). 브랜치 `feat/map-visual-depth`.

## 1. 배경

맵 테마(#104)가 챕터별 조명/바닥/프롭 색을, 하이브리드 B(#105)가 `M_MapTheme`(Color 벡터 파라미터)로 바닥·프롭 정밀 색을 입혔다. 그러나 ① **대기 무드(안개)**가 없어 깊이감/원근이 평면적이고, ② 프롭이 모두 동일 질감(BaseColor+약발광)이라 **재질 다양성**이 없으며, ③ **하늘**이 없어(SkyLight 앰비언트만) 배경이 비어 있다.

사용자 요청 "맵 시각 심화"(풀 스코프, 클릭): **안개 + 프롭 머티리얼 변형 + 절차 스카이**를 한 슬라이스로 묶어 "다른 장소" 몰입을 완성한다. #105의 **헤드리스 커맨드릿 + LFS 에셋** 패턴을 재사용한다.

## 2. 목표 / 비목표

### 목표
1. **안개**: 챕터별 `ExponentialHeightFog` 색/밀도(대기 무드·원근).
2. **프롭 머티리얼 변형**: `M_MapTheme`에 `Metallic`/`Roughness`/`EmissiveStrength` 스칼라 파라미터 추가, 프롭별 값으로 금속/무광/발광 질감 차별화.
3. **절차 스카이**: 그라데이션 `TextureCube`(`TC_MapSky`) + Unlit 스카이 머티리얼(`M_Sky`, 큐브맵 샘플 × `SkyTint`) → 역방향 스피어 배경 + SkyLight 앰비언트, 챕터별 하늘색.
4. 헤드리스 Automation 회귀(안개 컴포넌트 값, 프롭 MID 스칼라, 스카이 에셋 로드/적용).

### 비목표
- 임포트 환경 메시/아트 에셋(별도 후속).
- 동적 일주(낮/밤 시간 흐름), 구름/날씨 시뮬.
- 서버/세이브/parity/밸런스 변경(시각 전용, SaveVer 29 유지).
- 게임플레이 영향(안개/스카이/프롭 전부 NoCollision·비간섭).
- 디자이너 수작업 머티리얼/텍스처(절차 커맨드릿으로 충분).

## 3. 데이터 모델 확장 (클라 C++)

`FMapProp` 추가 필드(질감):
```
float Metallic = 0.0f;        // 0=비금속 ~ 1=금속
float Roughness = 0.85f;      // 0=거울 ~ 1=무광
float EmissiveStrength = 0.15f; // 자발광 배수(어두운 챕터 식별성)
```

`FMapTheme` 추가 필드(대기·하늘):
```
FLinearColor FogColor = FLinearColor(0.5,0.55,0.6);
float        FogDensity = 0.02f;   // ExponentialHeightFog Density
FLinearColor SkyTint = FLinearColor::White; // 스카이 큐브맵 틴트(챕터별 하늘색)
```

8 챕터 팔레트에 안개/스카이 값 추가(plan에서 RGB 확정, 스토리 톤 매칭: 초원=옅은 청백 안개·하늘 / 균열=붉은 안개 / 후존계=차가운 청백 정적).

## 4. 에셋 생성 (헤드리스 커맨드릿, #105 패턴)

### 4-1. `M_MapTheme` 확장 (기존 커맨드릿 수정)
`UGenerateMapThemeMaterialCommandlet`에 스칼라 파라미터 3종 추가:
- `UMaterialExpressionScalarParameter` `Metallic`(기본 0) → Metallic.
- `Roughness`(기본 0.85) → Roughness(기존 상수 대체).
- `EmissiveStrength`(기본 0.15) → `Color × EmissiveStrength` → Emissive(기존 0.15 상수 대체).
- 재실행해 `M_MapTheme.uasset` 재생성(LFS 갱신). 기존 `Color` 파라미터·연결 유지(하위호환).

### 4-2. 신규 스카이 커맨드릿 `GenerateMapSkyAssets`
`UGenerateMapSkyAssetsCommandlet`(`WITH_EDITOR`):
- **`TC_MapSky`** (`UTextureCube`): 절차 그라데이션 — 각 면(6) 픽셀을 월드 방향(+Z 천정 짙은색 → 수평 밝은색 → -Z 바닥 어두운색) 그라데이션으로 채움. 중립 톤(런타임 `SkyTint`로 챕터색). 적정 해상도(예 64²/면). `Source.Init` + 압축. 저장 `/Game/Maps/TC_MapSky`(LFS).
- **`M_Sky`** (`UMaterial`): ShadingModel Unlit, TwoSided. `TextureSampleParameterCube`(파라미터 `SkyCube`, 기본 `TC_MapSky`)를 뷰 방향으로 샘플 × `SkyTint`(벡터 파라미터) → EmissiveColor. 저장 `/Game/Maps/M_Sky`.
- 멱등(재실행 시 동일 산출).

> 스카이 큐브맵은 **단일 중립 그라데이션 에셋**, 챕터 색차는 런타임 `SkyTint` MID로 표현(에셋 1개로 8 챕터 — #105 파라미터 머티리얼 철학 일관). TwoSided Unlit로 역방향 스피어 안쪽이 배경으로 보임.

## 5. 런타임 적용 (GameMode)

`SpawnDefaultEnvironment`에 추가 spawn(1회, 멤버 보관):
- `AExponentialHeightFog* ThemeFog`.
- `AStaticMeshActor* ThemeSky`(Engine `/Engine/BasicShapes/Sphere`, 큰 음수 스케일로 역방향, NoCollision, `M_Sky` MID).

`ApplyMapTheme(Chapter)` 확장:
- **안개**: `ThemeFog`의 `UExponentialHeightFogComponent`에 `SetFogInscatteringColor(Theme.FogColor)`(또는 해당 API), `SetFogDensity(Theme.FogDensity)`.
- **프롭 질감**: 프롭 MID에 기존 `Color` + 신규 `SetScalarParameterValue("Metallic"/"Roughness"/"EmissiveStrength", Prop.*)`.
- **스카이**: `ThemeSky` MID `SetVectorParameterValue("SkyTint", Theme.SkyTint)`. SkyLight `SetCubemap(TC_MapSky)` + `SetSourceType(SLS_SpecifiedCubemap)`(앰비언트, best-effort) — 에셋 무효 시 기존 SkyColor 유지.
- 전부 **에셋/액터 무효 시 skip**(폴백, 크래시 0). lazy 로드 캐시(`bThemeMaterialLoadAttempted` 패턴 확장: `ThemeSkyMaterial`, `ThemeSkyCubemap`).

## 6. 정직한 범위 / 헤드리스 한계
- 안개·프롭 질감·스카이 MID/에셋 적용은 **컴포넌트 프로퍼티/파라미터 값**으로 헤드리스 검증 가능.
- `nullrhi` 헤드리스에선 실제 렌더 픽셀은 확인 불가 — 테스트는 "적용 호출이 올바른 값을 설정했는가"(컴포넌트 상태/MID 파라미터)까지. 시각 최종 확인은 PIE(사용자/후속).
- 폴백 경로로 에셋 누락 시 #104/#105 동작 유지(회귀 0).

## 7. 8 챕터 안개/스카이 방향 (plan에서 확정)
- ch1 초원: 옅은 청백 안개(저밀도)·맑은 하늘 / ch2 숲: 녹빛 옅은 안개 / ch3 차원그림자: 짙은 보라 안개 / ch5 심연: 짙은 청흑 / ch7 균열: 붉은 안개·불길 하늘 / ch8 후존계: 차가운 청백 정적 안개. 안개 밀도는 어두운/심부 챕터일수록↑.

## 8. 테스트 / 게이트 (헤드리스)
- `MapThemeTests` 확장:
  - 라이브러리: 8 챕터 `FogDensity>0`, `SkyTint` 유효.
  - 적용: `ApplyMapTheme` 후 `ThemeFog` 컴포넌트 Density==테마, 프롭 MID `Metallic`/`Roughness`/`EmissiveStrength`==프롭값, `ThemeSky` MID `SkyTint`==테마.
  - 에셋: `M_MapTheme`(`Metallic` 스칼라 파라미터 보유), `M_Sky` 로드 + `SkyTint`/`SkyCube` 파라미터, `TC_MapSky` 로드(UTextureCube).
- 표준 jumbo 빌드 + 전체 IdleProject Automation. 서버/`IdleSaveGame.h` diff 무변경(SaveVer 29). 서버 vitest/biome 무변경(손대지 않음).

## 9. 안전 가드
- 커맨드릿 `WITH_EDITOR` + 에디터 타깃 전용 모듈(#105 재사용) → 런타임/패키지 빌드 무영향.
- 모든 신규 액터 NoCollision·환경 1회 spawn 가드·챕터 클램프 유지.
- 에셋/액터 무효 시 요소별 skip 폴백(null 역참조 0).
- 시각 전용 — 세이브/서버/밸런스/parity 무변경. 멱등 재적용.

## 10. 구현 단계화 (plan)
1. `MapThemeTypes.h` 필드 확장(FMapProp 질감 3 + FMapTheme 안개/스카이 3) + `MapThemeLibrary` 8 팔레트 값.
2. `GenerateMapThemeMaterialCommandlet` 스칼라 파라미터 3 추가 → `M_MapTheme.uasset` 재생성(LFS).
3. `GenerateMapSkyAssetsCommandlet` 신규 → `TC_MapSky`/`M_Sky` 생성(LFS) → 헤드리스 실행 커밋.
4. GameMode: ThemeFog/ThemeSky spawn + `ApplyMapTheme` 안개·프롭질감·스카이 적용 + lazy 로드/폴백.
5. `MapThemeTests` 확장(안개/질감/스카이 적용·에셋).
6. 게이트: 표준 jumbo + 전체 Automation, 세이브·서버 무변경 확인 → PR.

## 11. 후속
- 동적 일주(시간대 색 보간), 구름/날씨, 임포트 환경 메시 스왑, 프롭 인스턴싱(HISM) 대량 배치.
