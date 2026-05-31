# 원신풍 비주얼 파운데이션 (슬라이스 1+2) 설계 스펙

- 작성일: 2026-05-31
- 상태: 설계 승인됨(사용자 "승인 — 스펙 작성으로") → 사용자 스펙 검토 후 plan 단계화
- 분류: 프론트엔드(UE5 HUD) + 렌더링(포스트프로세스·라이팅·머티리얼). **클라 전용 — 서버/세이브/parity 무관(SaveVer 29 무변경).**
- 브랜치: `feat/visual-foundation` (스택: `feat/hud-nav-shell` PR #110 위 — nav 셸 코드 포함).
- 상위 비전: [[project-visual-overhaul]]. 전체 룩 목표 = **원신(Genshin)**. 본 슬라이스는 **에셋 무의존(코드/절차)** 범위만.

## 1. 배경 / 목표

현재 룩이 "옛날 게임"처럼 납작·저대비(HUD = `UIThemeTokens` 단색 + 각진 `DrawRect`, 씬 = 포스트프로세스 없음). 사용자 목표 = **원신풍 고그래픽**. 6슬라이스 로드맵 중 **에셋 없이 즉시 가능한 1(원신 UI)+2(렌더 파운데이션)를 묶어** 먼저 구현해 비주얼 기반을 깐다. 캐릭터/몬스터/장비/환경 *에셋*은 후속 슬라이스(3~6).

### 목표
1. **원신 UI**: 양피지 크림 반투명 패널 + 얇은 골드 더블 프레임 + 둥근 모서리 + 다크 슬레이트 타이포 계층 + 골드 액센트 + 별 등급 + 원소색을, HUD 전체(~25 패널 + nav 셸 + 상시요소)에 적용.
2. **렌더 파운데이션**: PostProcessVolume(언바운드) + 라이팅 튠 + SkyAtmosphere + 셀셰이딩 아웃라인으로 씬을 원신풍으로.
3. 회귀: 표준 jumbo 빌드 GREEN + 전체 IdleProject Automation 유지. SaveVer 29 무변경.

### 비목표
- 캐릭터/몬스터/장비/환경 **3D 에셋 도입**(슬라이스 3~6).
- HUD **레이아웃/정보구조 변경**(nav 셸 구조는 PR #110 그대로, **시각만** 교체).
- 게임플레이/공식/세이브 변경.
- 원신 전용 *서체*·*장식 텍스처* 임포트(하이브리드 후속 — 본 슬라이스는 절차 생성 + 기존 한글 폰트 계층으로).

## 2. 테마 토큰 (`UI/UIThemeTokens.h`)

기존 토큰은 유지하되(타 코드 참조 깨짐 방지) **원신 팔레트 토큰 추가** + 패널/텍스트 토큰 값 갱신. 신규:
```cpp
// 원신 팔레트
inline constexpr FLinearColor PanelCream      = FLinearColor(0.953f, 0.933f, 0.886f, 0.94f); // 양피지(반투명)
inline constexpr FLinearColor PanelSlate      = FLinearColor(0.231f, 0.255f, 0.322f, 0.93f); // 다크(활성/대비)
inline constexpr FLinearColor FrameGold       = FLinearColor(0.784f, 0.667f, 0.431f, 1.f);   // #C8AA6E
inline constexpr FLinearColor FrameGoldLight  = FLinearColor(1.0f,   0.965f, 0.875f, 1.f);   // #FFF6DF
inline constexpr FLinearColor TextSlate       = FLinearColor(0.227f, 0.255f, 0.314f, 1.f);   // 크림 위 헤더
inline constexpr FLinearColor TextWarmGray    = FLinearColor(0.478f, 0.431f, 0.345f, 1.f);   // 라벨
inline constexpr FLinearColor TextOnSlate     = FLinearColor(0.953f, 0.918f, 0.824f, 1.f);   // 다크 위 텍스트
inline constexpr FLinearColor AccentGoldWarm  = FLinearColor(0.827f, 0.737f, 0.553f, 1.f);   // 골드 강조
inline constexpr FLinearColor StarGold        = FLinearColor(0.953f, 0.663f, 0.227f, 1.f);   // 별
inline constexpr float PanelCornerRadius      = 14.0f; // 9-slice 코너 반경(레퍼런스 px)
```
- 기존 `BgPanel/TextPrimary/TextMuted/Accent*`는 **존치**(점진 마이그레이션). 패널/텍스트는 새 토큰으로 그리도록 헬퍼에서 일원화.

## 3. Canvas 드로잉 툴킷

`Canvas::DrawRect`는 각진 단색뿐 → **둥근 골드프레임 패널 = 절차 생성 9-slice 텍스처**(프로젝트 기존 헤드리스 커맨드릿 패턴 = `GenerateMapThemeMaterialCommandlet`/`GenerateMapSkyAssetsCommandlet` 동일 정석).

### 3-1. 신규 커맨드릿 `UGenerateUiChromeAssetsCommandlet`
`WITH_EDITOR` 조건부. `UTexture2D` Source(`Source.Init(W,H,1,1,TSF_BGRA8)` + 픽셀 채움 + `UpdateResource`)로 생성·LFS 커밋:
- `T_GenshinPanel`(둥근 코너 + 골드 더블 프레임 + 크림 채움, 알파 9-slice). 256×256, 코너 가이드 = `PanelCornerRadius` 기준 margin.
- `T_GenshinPanelSlate`(다크 변형).
- `T_GenshinButtonGold` / `T_GenshinButtonSlate`.
- `T_SoftShadow`(드롭섀도 방사 알파).
- `T_GoldStar`(별 아이콘).
> 프레임 색은 베이크, 채움은 draw-time 틴트(곱) 가능하게 흰 채움+알파로. 9-slice margin은 헬퍼 상수.

### 3-2. 신규 HUD 헬퍼 (`AIdleHUD`)
```cpp
void DrawPanelChrome(float X, float Y, float W, float H, EPanelStyle Style); // 섀도+9slice 타일, Style=Cream/Slate
void DrawGoldButton(float X, float Y, float W, float H, const FString& Label, bool bEnabled, EBtnStyle Style);
void DrawRarityStars(float X, float Y, int32 Count, float Scale);
void DrawGradientRect(const FLinearColor& Top, const FLinearColor& Bottom, float X, float Y, float W, float H);
```
- `DrawTile`(9-slice 수동: 9개 `DrawTile` 호출 또는 `FCanvasTileItem`)로 임의 크기 둥근 패널.
- 텍스처 lazy 로드(`LoadObject<UTexture2D>` 캐시), 에셋 무효 시 기존 `DrawRect` 폴백(회귀 안전).

## 4. 패널·셸·상시요소 리스타일

`DrawHUD`의 패널들이 **공통 헬퍼로 그리게 교체**(내용/위치 로직 불변, 외형만):
- 패널 배경: `DrawRect(BgPanel...)` → `DrawPanelChrome(..., Cream)`. 활성/강조 패널 = Slate.
- 헤더/라벨/수치: `TextSlate`(헤더 굵게) / `TextWarmGray`(라벨) / `TextSlate`+굵게(수치). 폰트 스케일 계층.
- 버튼: 기존 골드 사각 → `DrawGoldButton`(골드/다크 더블, 둥근).
- **nav 셸**: 레일/탭바 칩 = 크림+골드 프레임, 활성 = Slate+골드(`DrawPanelChrome`/9-slice 칩). 서브탭 동일 톤. 닫기 X = 골드.
- **상시요소**: 스테이지/보스 바 = 크림 트랙 + 골드 필, 스킬 슬롯 = 둥근 골드 프레임, 궁극기 게이지 = 골드 그라데이션, 속성 범례 = 크림 패널.
- 등급 표시처에 `DrawRarityStars`, 원소처에 원소색 점.
> 단계: nav+상시 먼저 → 카테고리별 패널 7묶음(전투/성장/환생/장비/수집/일일/소셜) 점진, 각 빌드+회귀.

## 5. 렌더 파운데이션 (`IdleProjectGameModeBase`)

### 5-1. PostProcessVolume (언바운드 신규 스폰)
`APostProcessVolume` 스폰, `bUnbound=true`, `Settings`:
- WhiteBalance Temp ~6800(따뜻), Bloom Intensity ~0.6 Threshold ~1.0(소프트), AmbientOcclusion Intensity ~0.5, Vignette ~0.4, FilmGrain 0, Tonemapper(Filmic 기본), Saturation ~1.08 Contrast ~1.05.
- 멤버 보관(`ThemePostProcess`) → 테마/챕터별 그레이드 갱신 여지.

### 5-2. 라이팅 튠
- `ThemeSun`(기존 DirectionalLight): 색온도 따뜻, 강도 ↑, SourceAngle ↑(부드러운 그림자), 각도 재조정.
- `ThemeSky`(SkyLight): Intensity ↑(밝은 환경광), 실시간 캡처.
- `ThemeFog`: 밀도/색 원신풍 재조정(옅은 대기 원근).

### 5-3. SkyAtmosphere
- `IdleProject.Build.cs` 에 모듈 의존 추가(필요 시). `ASkyAtmosphere` 스폰, DirectionalLight를 atmosphere sun으로 연결 → 진짜 대기 산란 하늘. 기존 절차 스카이스피어(`ThemeSkySphere`) 대체/공존 판단(plan에서 정밀화).

### 5-4. 셀셰이딩 아웃라인
- 포스트프로세스 머티리얼 `M_ToonOutline`(커맨드릿 생성): SceneTexture Depth/Normal 엣지 검출 → 외곽선. 약한 톤 포스터라이즈(선택). PostProcessVolume `Settings.WeightedBlendables`에 추가.
- (캐릭터별 정밀 툰 셰이딩 = 슬라이스 3, 임포트 메시에 toon 머티리얼.)

## 6. 구현 단계 (슬라이스 내, plan에서 태스크화)

1. 테마 토큰 + `UGenerateUiChromeAssetsCommandlet`(텍스처 생성·LFS) + Canvas 툴킷 헬퍼 + 단위/회귀.
2. nav 셸 + 상시요소 리스타일.
3. 카테고리별 패널 리스타일(7묶음, 각 회귀).
4. PostProcessVolume + 라이팅 + SkyAtmosphere.
5. `M_ToonOutline` 커맨드릿 + 포스트프로세스 적용.
6. PIE 시각 검증(데스크톱/모바일, 전 카테고리) + 스크린샷.

## 7. 세이브 / parity / 리스크

- **세이브 무변경(SaveVer 29)**, 서버 무관. 시각 전용.
- 신규 UE 모듈 의존(SkyAtmosphere) 1건 — `Build.cs` 빌드 영향만.
- **에셋 의존 런타임 = lazy 로드 + `DrawRect`/기존 라이팅 폴백**(에셋 무효 시 회귀 0, #104→#105 교훈).
- 커맨드릿 = `WITH_EDITOR`+에디터 타깃 조건부(맵테마 커맨드릿 선례). jumbo ODR 주의(신규 익명 헬퍼 동명 grep).
- 헤드리스 렌더 픽셀은 PIE 최종 확인(헤드리스 뷰포트 1280×720 고정 한계 — 모바일은 모드 강제 확인).

## 8. 테스트 / 검증

- Automation: 툴킷 헬퍼의 순수 부분(9-slice 좌표 계산, 별 개수→그리기 카운트, 스타일→토큰 매핑) 단위. 커맨드릿 산출 텍스처 존재/규격.
- 회귀: 기존 HUD ViewModel/Localization 테스트 무변경 유지(외형만 바뀜).
- 수동 PIE: 데스크톱/모바일 × 7카테고리 — 원신 패널·골드프레임·타이포·별·버튼, 포스트프로세스 색감·블룸·AO·비네트, SkyAtmosphere 하늘, 툰 아웃라인. 스크린샷 종합(PR 단일 리뷰).
- 게이트: `tools/ci/ue-automation.ps1` 표준 jumbo + 전체 Automation GREEN.
