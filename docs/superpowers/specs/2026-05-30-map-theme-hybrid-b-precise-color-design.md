# 맵 테마 하이브리드 B — 정밀 색(파라미터 머티리얼 헤드리스 생성) 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(정밀 색만, 헤드리스 커맨드릿 생성) → plan 단계화
- 분류: 프론트엔드(UE5 클라 전용 시각). **서버/세이브/parity 무관.**
- 선행: 맵 테마 시스템(#104). 브랜치 `feat/map-theme-hybrid-b`.

## 1. 배경

맵 테마 시스템(#104)은 챕터별 **조명(Sun/Sky) 색·강도**로 주 무드를 입히고, 바닥·프롭은 `SetVectorParameterValue("Color", ...)` MID 틴트로 색을 **시도**한다(`IdleProjectGameModeBase.cpp:147, 187`). 그러나 바닥·프롭이 쓰는 베이스 머티리얼은 Engine 기본(`BasicShapeMaterial` 등)으로 **"Color" 벡터 파라미터가 없어** 틴트가 **no-op**이다. 즉 챕터별로 조명 톤은 바뀌지만 **바닥·프롭의 정밀 색은 아직 반영되지 않는다**(#104 스펙 §5에서 "후속" 명시).

하이브리드 B의 첫 후속으로 사용자는 **"정밀 색만(헤드리스 커맨드 생성)"**을 선택했다: 코드만으로는 만들 수 없는 **파라미터 머티리얼 에셋**(`Color` 벡터 파라미터 보유)을 **헤드리스 에디터 커맨드릿**으로 생성하고, 런타임이 바닥·프롭에 그 머티리얼을 적용해 기존 MID 틴트가 **실제로** 색을 입히게 한다.

> 임포트 환경 메시 교체(soft 경로 스왑)는 이번 비목표 — `FMapProp.Mesh` 경로는 그대로 두고 색만 살린다.

## 2. 목표 / 비목표

### 목표
1. 헤드리스 에디터 **커맨드릿**으로 `/Game/Maps/M_MapTheme` **파라미터 머티리얼** 에셋(.uasset) 생성: `Color` 벡터 파라미터 → BaseColor, 약한 Emissive 보강, Roughness 상수.
2. 생성된 `M_MapTheme.uasset`을 **LFS로 커밋**(저장소에 에셋 동봉, 런타임 로드).
3. 런타임 `ApplyMapTheme`이 바닥·프롭 베이스 머티리얼을 `M_MapTheme`로 교체한 뒤 MID 생성 → 기존 `Color` 틴트가 **실제 반영**. 에셋 부재/로드 실패 시 **기존 best-effort 경로로 폴백**(크래시·회귀 0).
4. 헤드리스 Automation 회귀: `M_MapTheme` 로드 + `Color` 파라미터 보유 + 적용 후 MID `Color` 값 == 기대.

### 비목표
- 임포트 메시 에셋 교체(soft 경로 스왑 — 별도 후속, 시스템 무변경).
- 안개/스카이 큐브맵/스테이지 내 세분(#104 후속 목록 유지).
- 서버/세이브/parity/밸런스 변경(시각 전용, SaveVer 29 유지).
- 디자이너 수작업 머티리얼 그래프(절차 생성 커맨드릿으로 충분).
- 프롭별 머티리얼 다양화(단일 파라미터 머티리얼 1종 재사용, 색만 인스턴스 차이).

## 3. 커맨드릿 (에디터 전용 자산 생성)

신규 `UGenerateMapThemeMaterialCommandlet`(클라 모듈 내, **`WITH_EDITOR` 가드**). 에디터 타깃(`IdleProjectEditor`, ue-automation 빌드 대상)에서만 컴파일·실행.

- 실행: `UnrealEditor-Cmd.exe "<IdleProject.uproject>" -run=GenerateMapThemeMaterial -unattended -nopause -nosplash`
- 동작:
  1. `UMaterialFactoryNew`로 `UMaterial` 신규 생성, 패키지 경로 `/Game/Maps/M_MapTheme`.
  2. 머티리얼 그래프(노드 직접 구성):
     - `UMaterialExpressionVectorParameter` `Color`(기본값 흰색) → **BaseColor**.
     - `Color` → `Multiply`(상수 0.15) → **Emissive Color**(어두운 챕터에서도 프롭이 식별되도록 약한 자발광).
     - `UMaterialExpressionConstant` Roughness 0.85 → **Roughness**(무광 스타일라이즈드).
     - ShadingModel = DefaultLit.
  3. `Material->PostEditChange()` + 컴파일, `UPackage::SavePackage`로 `.uasset` 저장.
  4. 멱등: 이미 존재해도 덮어쓰기(결정적 산출).
- 에디터 모듈 의존(`.Build.cs`에서 `Target.bBuildEditor`일 때만 `UnrealEd`, `MaterialEditor`, `AssetTools` 추가).
- 산출물 `client/Content/Maps/M_MapTheme.uasset` → `.gitattributes` `*.uasset filter=lfs`로 자동 LFS.

> **에셋 생성 책임**: 구현 서브에이전트가 커맨드릿 빌드 후 헤드리스 실행으로 `.uasset` 생성 → 커밋. PM 게이트가 빌드/Automation으로 검증. 에디터 환경(`C:\Program Files\Epic Games\UE_5.7`)은 ue-automation.ps1이 사용하는 동일 경로로 가용.

## 4. 런타임 적용 (GameMode)

`ApplyMapTheme`의 바닥·프롭 색 적용부 보강(`IdleProjectGameModeBase.cpp:137-151, 182-189`):

- 테마 머티리얼 1회 lazy 로드: `static const FSoftObjectPath ThemeMatPath(TEXT("/Game/Maps/M_MapTheme.M_MapTheme"))` → `LoadSynchronous`. 멤버 캐시(`TObjectPtr<UMaterialInterface> ThemeMaterial`).
- 바닥:
  - `ThemeMaterial` 유효 → `GroundMesh->SetMaterial(0, ThemeMaterial)` 후 `CreateAndSetMaterialInstanceDynamic(0)` → `SetVectorParameterValue("Color", Theme.GroundColor)` (이제 **실제 반영**).
  - 무효 → **기존 동작 유지**(베이스 그대로 MID, 틴트 no-op 폴백).
- 프롭: 각 프롭 컴포넌트도 동일 — `ThemeMaterial` 유효 시 `SetMaterial(0, ThemeMaterial)` 후 MID `Color` = `Prop.Color`, 무효 시 기존.
- 변경은 **색 적용 분기만**. 조명/프롭 spawn/교체/NoCollision/클램프 로직은 #104 그대로.

## 5. 정직한 범위
- `M_MapTheme` 커밋 후: 바닥·프롭이 챕터/프롭 색으로 **실제** 틴트(조명 톤 + 정밀 색 = 완성된 무드).
- 에셋 누락(LFS 미체크아웃 등) 또는 로드 실패: #104 동작으로 **무결 폴백**(조명만, 색 no-op) — 회귀·크래시 0.
- 머티리얼은 단순(BaseColor+약한 Emissive+Roughness). 후속에서 노멀/러프 텍스처·프롭별 머티리얼 확장 여지.

## 6. 테스트 / 게이트 (헤드리스)
- `MapThemeTests`(기존) 확장:
  - `M_MapTheme` `LoadSynchronous` 성공 + `UMaterialInterface` 캐스팅.
  - 머티리얼이 `Color` 벡터 파라미터 보유(`GetVectorParameterValue`/`GetAllVectorParameterInfo`로 존재 단언).
  - 헤드리스 월드에서 `ApplyMapTheme(c)` 후 바닥 MID `Color` 파라미터 == `Theme.GroundColor`(±tol), 프롭 MID `Color` == `Prop.Color`.
  - 에셋 유효 시 바닥/프롭 컴포넌트 머티리얼 == `M_MapTheme`(SetMaterial 적용 확인).
- 표준 jumbo 빌드(`IdleProjectEditor`) + 전체 IdleProject Automation(신규 포함). **세이브·서버 무변경**(SaveVer 29).
- 서버 vitest/biome: **무변경**(시각 전용 — 서버 코드 손대지 않음).

## 7. 안전 가드
- 커맨드릿 `WITH_EDITOR` 가드 + 에디터 타깃 전용 모듈 → 런타임/패키지 빌드 무영향.
- 런타임 머티리얼 로드 실패 시 폴백(기존 베이스 머티리얼) — null 역참조 없음.
- 프롭 NoCollision·환경 1회 spawn·챕터 클램프(#104) 유지.
- 시각 전용 — 세이브/서버/밸런스/parity 무변경.
- 멱등: 커맨드릿 재실행 시 결정적 동일 에셋, `ApplyMapTheme` 재적용 시 머티리얼 재설정 안전.

## 8. 구현 단계화 (plan)
1. `.Build.cs` 에디터 타깃 조건부 모듈(`UnrealEd`/`MaterialEditor`/`AssetTools`) + `GenerateMapThemeMaterialCommandlet.h/.cpp`(`WITH_EDITOR`).
2. 헤드리스 커맨드릿 실행 → `client/Content/Maps/M_MapTheme.uasset` 생성 + LFS 커밋.
3. GameMode 색 적용 분기 보강(`ThemeMaterial` lazy 로드 + 바닥/프롭 `SetMaterial`→MID, 폴백).
4. `MapThemeTests` 확장(에셋 로드/파라미터/적용 후 MID 값/머티리얼 설정).
5. 게이트: 표준 jumbo + Automation, 세이브·서버 무변경 확인.

## 9. 후속
- 프롭별 머티리얼/텍스처(노멀·러프), 메시 임포트 스왑(soft 경로), 안개/스카이 큐브맵 테마.
