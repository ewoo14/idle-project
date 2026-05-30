# 원신풍 비주얼 파운데이션 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** HUD 전체와 씬 분위기를 원신(Genshin)풍으로 — 양피지 크림 반투명 패널·골드 더블프레임·둥근 모서리·타이포 계층·별·원소색 + 포스트프로세스·라이팅·SkyAtmosphere·툰 아웃라인. **에셋 무의존(코드/절차)**.

**Architecture:** 순수 로직(9-slice 좌표·스타일→토큰 매핑·별 배치)을 신규 `UI/HudChrome.{h,cpp}`로 분리해 Automation 단위 테스트. 둥근 골드 패널은 헤드리스 커맨드릿이 9-slice `UTexture2D`를 절차 생성(LFS). `AIdleHUD`에 `DrawPanelChrome`/`DrawGoldButton`/`DrawRarityStars` 헬퍼 추가 후 기존 패널들이 그걸로 그리게 교체(내용/위치 불변, 외형만). 렌더 파운데이션은 `IdleProjectGameModeBase`가 PostProcessVolume·SkyAtmosphere·라이팅을 스폰/튠.

**Tech Stack:** UE5.7.4, C++17, Canvas `AHUD`(DrawTile/DrawText), Commandlet(WITH_EDITOR) `UTexture2D::Source.Init(TSF_BGRA8)`, PostProcessVolume/SkyAtmosphere, UE Automation.

**클라 전용 — 서버/세이브/parity 무관. SaveVer 29 무변경.** 스펙: `docs/superpowers/specs/2026-05-31-visual-foundation-genshin-design.md`. 브랜치 `feat/visual-foundation`(스택: `feat/hud-nav-shell`). 상위 비전 [[project-visual-overhaul]].

> **공통 게이트(커밋 전):** `powershell -File tools/ci/ue-automation.ps1 -Project "C:\game\idle game\repo\client\IdleProject.uproject"` (필요 시 `-Filter`로 좁힘). 엔진 `C:\Program Files\Epic Games\UE_5.7`. 시각 결과는 PIE 오프스크린 렌더(헤드리스 §4 레시피)로 스크린샷 확인. jumbo ODR 주의(신규 익명 헬퍼 동명 grep).

---

## File Structure

| 파일 | 책임 | 작업 |
| --- | --- | --- |
| `client/Source/IdleProject/UI/HudChrome.h/.cpp` | 순수 로직: `EPanelStyle`/`EBtnStyle`, 9-slice 좌표 계산, 스타일→텍스처/틴트 매핑, 별 배치 | Create |
| `client/Source/IdleProject/Tests/HudChromeTests.cpp` | 위 로직 단위 테스트 | Create |
| `client/Source/IdleProject/UI/UIThemeTokens.h` | 원신 팔레트 토큰 추가 | Modify |
| `client/Source/IdleProject/Commandlets/GenerateUiChromeAssetsCommandlet.h/.cpp` | 9-slice 패널/버튼/별/섀도 `UTexture2D` 절차 생성→LFS | Create |
| `client/Content/UI/Chrome/T_*.uasset` | 생성 텍스처(LFS) | Create(생성물) |
| `client/Source/IdleProject/UI/IdleHUD.h/.cpp` | `DrawPanelChrome`/`DrawGoldButton`/`DrawRarityStars` 헬퍼 + 전 패널·셸·상시요소 리스타일 | Modify |
| `client/Source/IdleProject/IdleProjectGameModeBase.cpp` | PostProcessVolume·라이팅·SkyAtmosphere·툰 적용 | Modify |
| `client/Source/IdleProject/Commandlets/GenerateToonOutlineCommandlet.cpp`(또는 chrome에 통합) | `M_ToonOutline` 포스트프로세스 머티리얼 생성 | Create |
| `client/Source/IdleProject/IdleProject.Build.cs` | SkyAtmosphere 모듈 의존 | Modify |

---

## Phase 1 — 순수 로직 + 절차 텍스처

### Task 1: 원신 테마 토큰

**Files:** Modify `client/Source/IdleProject/UI/UIThemeTokens.h`; Test `client/Source/IdleProject/Tests/HudChromeTests.cpp`

- [ ] **Step 1: 실패 테스트** — `HudChromeTests.cpp` 신규:
```cpp
#include "Misc/AutomationTest.h"
#include "UI/UIThemeTokens.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGenshinTokenTest, "IdleProject.UI.HUD.GenshinTokens",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGenshinTokenTest::RunTest(const FString& Parameters)
{
    using namespace IdleProject::UI;
    // 크림 패널은 밝고 반투명, 슬레이트는 어둡고 반투명.
    TestTrue(TEXT("cream bright"), Theme::PanelCream.R > 0.8f && Theme::PanelCream.A < 1.0f);
    TestTrue(TEXT("slate dark"), Theme::PanelSlate.R < 0.4f && Theme::PanelSlate.A < 1.0f);
    TestTrue(TEXT("gold warm"), Theme::FrameGold.R > Theme::FrameGold.B); // 골드=R>B
    TestTrue(TEXT("text slate dark"), Theme::TextSlate.R < 0.4f);
    TestEqual(TEXT("corner radius"), Theme::PanelCornerRadius, 14.0f);
    return true;
}

#endif
```

- [ ] **Step 2: 실패 확인**(토큰 미존재). Run: `... -Filter IdleProject.UI.HUD.GenshinTokens`.

- [ ] **Step 3: 토큰 추가** — `UIThemeTokens.h`의 `Theme` 네임스페이스에(기존 토큰 아래):
```cpp
// ── 원신 팔레트(비주얼 파운데이션) ──
inline constexpr FLinearColor PanelCream     = FLinearColor(0.953f, 0.933f, 0.886f, 0.94f);
inline constexpr FLinearColor PanelSlate     = FLinearColor(0.231f, 0.255f, 0.322f, 0.93f);
inline constexpr FLinearColor FrameGold      = FLinearColor(0.784f, 0.667f, 0.431f, 1.0f);
inline constexpr FLinearColor FrameGoldLight = FLinearColor(1.0f,   0.965f, 0.875f, 1.0f);
inline constexpr FLinearColor TextSlate      = FLinearColor(0.227f, 0.255f, 0.314f, 1.0f);
inline constexpr FLinearColor TextWarmGray   = FLinearColor(0.478f, 0.431f, 0.345f, 1.0f);
inline constexpr FLinearColor TextOnSlate    = FLinearColor(0.953f, 0.918f, 0.824f, 1.0f);
inline constexpr FLinearColor AccentGoldWarm = FLinearColor(0.827f, 0.737f, 0.553f, 1.0f);
inline constexpr FLinearColor StarGold       = FLinearColor(0.953f, 0.663f, 0.227f, 1.0f);
inline constexpr float        PanelCornerRadius = 14.0f;
```

- [ ] **Step 4: 통과 확인**. Run: `... -Filter IdleProject.UI.HUD.GenshinTokens`. Expected PASS.

- [ ] **Step 5: 커밋**
```bash
git add client/Source/IdleProject/UI/UIThemeTokens.h client/Source/IdleProject/Tests/HudChromeTests.cpp
git commit -m "feat(visual): 원신 팔레트 테마 토큰"
```

### Task 2: HudChrome 순수 로직(9-slice·스타일·별)

**Files:** Create `UI/HudChrome.h/.cpp`; Modify `Tests/HudChromeTests.cpp`

- [ ] **Step 1: 실패 테스트 추가**:
```cpp
#include "UI/HudChrome.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudChromeLogicTest, "IdleProject.UI.HUD.ChromeLogic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHudChromeLogicTest::RunTest(const FString& Parameters)
{
    using namespace IdleProject::UI;
    // 9-slice: 100x80 dst, corner 10 → 9칸, 모서리 합이 dst를 덮음.
    FNineSliceCell cells[9];
    ComputeNineSlice(FUiRect{0,0,100,80}, 10.0f, 64.0f, 16.0f, cells);
    // 좌상 코너 dst = (0,0,10,10)
    TestEqual(TEXT("TL w"), cells[0].Dst.W, 10.0f);
    TestEqual(TEXT("TL h"), cells[0].Dst.H, 10.0f);
    // 중앙 칸 dst = (10,10, 80,60)
    TestEqual(TEXT("center w"), cells[4].Dst.W, 80.0f);
    TestEqual(TEXT("center h"), cells[4].Dst.H, 60.0f);
    // UV 코너 = 16/64 = 0.25
    TestTrue(TEXT("uv corner"), FMath::IsNearlyEqual(cells[0].UV.W, 0.25f));
    // 스타일→텍스처명
    TestEqual(TEXT("cream tex"), PanelTextureName(EPanelStyle::Cream), FString(TEXT("T_GenshinPanel")));
    TestEqual(TEXT("slate tex"), PanelTextureName(EPanelStyle::Slate), FString(TEXT("T_GenshinPanelSlate")));
    // 별 5개 배치: 시작 X부터 (size+gap) 간격, 개수 일치
    TArray<FUiRect> stars = ComputeStarRects(10.0f, 5.0f, 5, 12.0f, 3.0f);
    TestEqual(TEXT("5 stars"), stars.Num(), 5);
    TestEqual(TEXT("star0 x"), stars[0].X, 10.0f);
    TestEqual(TEXT("star1 x"), stars[1].X, 25.0f); // 10 + 12 + 3
    return true;
}
```

- [ ] **Step 2: 실패 확인**(헤더 미존재).

- [ ] **Step 3: `HudChrome.h`**:
```cpp
#pragma once
#include "CoreMinimal.h"

namespace IdleProject::UI
{
    enum class EPanelStyle : uint8 { Cream, Slate };
    enum class EBtnStyle   : uint8 { Gold, Slate };

    struct FUiRect { float X = 0, Y = 0, W = 0, H = 0; };
    struct FNineSliceCell { FUiRect Dst; FUiRect UV; }; // UV: 0..1

    // dst 사각형을 코너 CornerDst(px) 기준 9칸으로. TexSize=텍스처 한 변(px), CornerUV=텍스처 코너 px.
    // OutCells[0..8] = 행우선(TL,TC,TR, ML,MC,MR, BL,BC,BR).
    void ComputeNineSlice(const FUiRect& Dst, float CornerDst, float TexSize, float CornerUV, FNineSliceCell OutCells[9]);

    FString PanelTextureName(EPanelStyle Style);
    FString ButtonTextureName(EBtnStyle Style);

    // 별 N개를 (StartX,StarY)부터 가로로. 각 StarSize, 간격 Gap.
    TArray<FUiRect> ComputeStarRects(float StartX, float StarY, int32 Count, float StarSize, float Gap);
}
```

- [ ] **Step 4: `HudChrome.cpp`**:
```cpp
#include "UI/HudChrome.h"

namespace IdleProject::UI
{
    void ComputeNineSlice(const FUiRect& D, float C, float TexSize, float CUV, FNineSliceCell Out[9])
    {
        const float xs[4] = { D.X, D.X + C, D.X + D.W - C, D.X + D.W };
        const float ys[4] = { D.Y, D.Y + C, D.Y + D.H - C, D.Y + D.H };
        const float u = CUV / TexSize;            // 0..1 코너
        const float us[4] = { 0.0f, u, 1.0f - u, 1.0f };
        for (int32 r = 0; r < 3; ++r)
        for (int32 c = 0; c < 3; ++c)
        {
            FNineSliceCell& Cell = Out[r * 3 + c];
            Cell.Dst = FUiRect{ xs[c], ys[r], xs[c + 1] - xs[c], ys[r + 1] - ys[r] };
            Cell.UV  = FUiRect{ us[c], us[r], us[c + 1] - us[c], us[r + 1] - us[r] };
        }
    }

    FString PanelTextureName(EPanelStyle Style)
    {
        return Style == EPanelStyle::Slate ? TEXT("T_GenshinPanelSlate") : TEXT("T_GenshinPanel");
    }
    FString ButtonTextureName(EBtnStyle Style)
    {
        return Style == EBtnStyle::Slate ? TEXT("T_GenshinButtonSlate") : TEXT("T_GenshinButtonGold");
    }

    TArray<FUiRect> ComputeStarRects(float StartX, float StarY, int32 Count, float StarSize, float Gap)
    {
        TArray<FUiRect> R;
        for (int32 i = 0; i < Count; ++i)
        {
            R.Add(FUiRect{ StartX + i * (StarSize + Gap), StarY, StarSize, StarSize });
        }
        return R;
    }
}
```

- [ ] **Step 5: 통과 확인** Run: `... -Filter IdleProject.UI.HUD.ChromeLogic`. Expected PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Source/IdleProject/UI/HudChrome.h client/Source/IdleProject/UI/HudChrome.cpp client/Source/IdleProject/Tests/HudChromeTests.cpp
git commit -m "feat(visual): HudChrome 순수 로직(9-slice·스타일·별 배치)"
```

### Task 3: UI 크롬 텍스처 생성 커맨드릿

**Files:** Create `Commandlets/GenerateUiChromeAssetsCommandlet.h/.cpp`

> 패턴: 기존 `Commandlets/GenerateMapSkyAssetsCommandlet.cpp`(CreatePackage→NewObject<UTextureCube>→Source.Init(TSF_BGRA8)→UpdateResource→SavePackage)를 `UTexture2D`로. **반드시 그 파일을 먼저 읽어** 헤더/타깃 조건부(WITH_EDITOR)·LFS 경로 규약을 그대로 따른다.

- [ ] **Step 1: 헤더** `GenerateUiChromeAssetsCommandlet.h` — 기존 sky 커맨드릿 헤더를 그대로 본떠 `UGenerateUiChromeAssetsCommandlet : public UCommandlet` 선언(`WITH_EDITOR` 가드, `virtual int32 Main(const FString&) override;`).

- [ ] **Step 2: 구현** `GenerateUiChromeAssetsCommandlet.cpp` — 각 텍스처를 BGRA8 픽셀 버퍼로 절차 생성. 256×256 둥근 패널 알파+골드 프레임:
```cpp
// 둥근 사각형 + 골드 더블 프레임 패널 한 장 생성(Style별 채움색)
static TArray<uint8> MakePanel(int32 S, float CornerPx, FColor Fill, bool bGoldFrame)
{
    TArray<uint8> Px; Px.SetNumZeroed(S * S * 4);
    const float r = CornerPx;
    auto InRounded = [&](float x, float y)->bool {
        const float l = r, t = r, rt = S - r, bt = S - r;
        float dx = 0, dy = 0;
        if (x < l && y < t) { dx = l - x; dy = t - y; }
        else if (x > rt && y < t) { dx = x - rt; dy = t - y; }
        else if (x < l && y > bt) { dx = l - x; dy = y - bt; }
        else if (x > rt && y > bt) { dx = x - rt; dy = y - bt; }
        else return true;
        return (dx * dx + dy * dy) <= r * r;
    };
    auto EdgeDist = [&](float x, float y)->float { // 외곽까지 근사 거리(프레임용)
        return FMath::Min(FMath::Min(x, S - 1 - x), FMath::Min(y, S - 1 - y));
    };
    for (int32 y = 0; y < S; ++y)
    for (int32 x = 0; x < S; ++x)
    {
        uint8* P = &Px[(y * S + x) * 4];
        const bool in = InRounded(x + 0.5f, y + 0.5f);
        FColor c = Fill; uint8 a = in ? 255 : 0;
        if (in && bGoldFrame)
        {
            const float e = EdgeDist(x, y);
            if (e < 2.5f) { c = FColor(200, 170, 110); }            // 외곽 골드
            else if (e >= 4.0f && e < 5.5f) { c = FColor(255, 246, 223); } // 안쪽 골드라이트(더블)
        }
        P[0] = c.B; P[1] = c.G; P[2] = c.R; P[3] = a; // BGRA
    }
    return Px;
}
```
그리고 `MakePanel`로 `T_GenshinPanel`(Fill=크림 FColor(243,238,226), gold)·`T_GenshinPanelSlate`(Fill=FColor(59,65,82), gold)·`T_GenshinButtonGold`(작은 S=128, Fill=골드그라데 근사)·`T_GenshinButtonSlate`·별도 `T_SoftShadow`(중심→외곽 알파 감쇠, 프레임無)·`T_GoldStar`(별 다각형 알파)를 각각:
```cpp
UPackage* Pkg = CreatePackage(TEXT("/Game/UI/Chrome/T_GenshinPanel"));
UTexture2D* Tex = NewObject<UTexture2D>(Pkg, TEXT("T_GenshinPanel"), RF_Standalone | RF_Public);
TArray<uint8> Px = MakePanel(256, 28.0f, FColor(243,238,226), true);
Tex->Source.Init(256, 256, 1, 1, TSF_BGRA8, Px.GetData());
Tex->SRGB = true; Tex->CompressionSettings = TC_EditorIcon; // UI(알파)용 무압축류
Tex->UpdateResource();
FString File = FPackageName::LongPackageNameToFilename(TEXT("/Game/UI/Chrome/T_GenshinPanel"), FPackageName::GetAssetPackageExtension());
FSavePackageArgs Args; Args.TopLevelFlags = RF_Standalone | RF_Public;
UPackage::SavePackage(Pkg, Tex, *File, Args);
```
(코너 px 28 = 256 기준; 런타임 9-slice는 Task 2의 `ComputeNineSlice`에 `TexSize=256, CornerUV=28` 전달.)

- [ ] **Step 3: 빌드(에디터 타깃) + 커맨드릿 실행**:
```
powershell -File tools/ci/ue-automation.ps1 -Project "C:\game\idle game\repo\client\IdleProject.uproject" -Filter IdleProject.UI.HUD.ChromeLogic
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\game\idle game\repo\client\IdleProject.uproject" -run=GenerateUiChromeAssets -unattended -nopause
```
Expected: `/Game/UI/Chrome/T_*.uasset` 생성. `client/Content/UI/Chrome/`에 파일 확인.

- [ ] **Step 4: LFS 커밋**(png/uasset은 `.gitattributes` LFS)
```bash
git add client/Source/IdleProject/Commandlets/GenerateUiChromeAssetsCommandlet.h client/Source/IdleProject/Commandlets/GenerateUiChromeAssetsCommandlet.cpp client/Content/UI/Chrome/
git commit -m "feat(visual): 원신 UI 크롬 9-slice 텍스처 절차 생성 커맨드릿"
```

---

## Phase 2 — HUD 헬퍼 + 셸/상시요소 리스타일

### Task 4: AIdleHUD 크롬 드로잉 헬퍼

**Files:** Modify `UI/IdleHUD.h/.cpp`

- [ ] **Step 1: include + 멤버** — `IdleHUD.h`에 `#include "UI/HudChrome.h"`, 그리고 텍스처 캐시 멤버:
```cpp
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromePanel;
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromePanelSlate;
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromeButtonGold;
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromeButtonSlate;
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromeShadow;
UPROPERTY(Transient) TObjectPtr<UTexture2D> ChromeStar;
```
메서드 선언:
```cpp
UTexture2D* ResolveChrome(const TCHAR* AssetName, TObjectPtr<UTexture2D>& Cache);
void DrawNineSlice(UTexture2D* Tex, float X, float Y, float W, float H, const FLinearColor& Tint);
void DrawPanelChrome(float X, float Y, float W, float H, IdleProject::UI::EPanelStyle Style);
void DrawGoldButton(float X, float Y, float W, float H, const FString& Label, bool bEnabled, IdleProject::UI::EBtnStyle Style);
void DrawRarityStars(float X, float Y, int32 Count, float Scale);
```

- [ ] **Step 2: 구현** — `IdleHUD.cpp`:
```cpp
UTexture2D* AIdleHUD::ResolveChrome(const TCHAR* AssetName, TObjectPtr<UTexture2D>& Cache)
{
    if (!Cache)
    {
        Cache = LoadObject<UTexture2D>(nullptr, *FString::Printf(TEXT("/Game/UI/Chrome/%s.%s"), AssetName, AssetName));
    }
    return Cache;
}

void AIdleHUD::DrawNineSlice(UTexture2D* Tex, float X, float Y, float W, float H, const FLinearColor& Tint)
{
    using namespace IdleProject::UI;
    if (!Canvas || !Tex) { return; } // 폴백은 호출부에서
    const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
    const float CornerDst = Theme::PanelCornerRadius * Scale;
    FNineSliceCell cells[9];
    ComputeNineSlice(FUiRect{X, Y, W, H}, CornerDst, 256.0f, 28.0f, cells);
    for (int32 i = 0; i < 9; ++i)
    {
        const FNineSliceCell& c = cells[i];
        FCanvasTileItem Tile(FVector2D(c.Dst.X, c.Dst.Y), Tex->GetResource(),
            FVector2D(c.Dst.W, c.Dst.H), FVector2D(c.UV.X, c.UV.Y),
            FVector2D(c.UV.X + c.UV.W, c.UV.Y + c.UV.H), Tint);
        Tile.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(Tile);
    }
}

void AIdleHUD::DrawPanelChrome(float X, float Y, float W, float H, IdleProject::UI::EPanelStyle Style)
{
    using namespace IdleProject::UI;
    const float Scale = Canvas ? FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f) : 1.0f;
    // 드롭섀도
    if (UTexture2D* Sh = ResolveChrome(TEXT("T_SoftShadow"), ChromeShadow))
    {
        const float m = 10.0f * Scale;
        DrawNineSlice(Sh, X - m, Y - m + 4.0f * Scale, W + 2 * m, H + 2 * m, FLinearColor(0,0,0,0.35f));
    }
    UTexture2D* Tex = ResolveChrome(*PanelTextureName(Style),
        Style == EPanelStyle::Slate ? ChromePanelSlate : ChromePanel);
    if (Tex) { DrawNineSlice(Tex, X, Y, W, H, FLinearColor::White); }
    else { DrawRect(Style == EPanelStyle::Slate ? Theme::PanelSlate : Theme::PanelCream, X, Y, W, H); } // 폴백
}
```
`DrawGoldButton`(버튼 9-slice + 중앙 라벨 텍스트, 비활성 시 디밍), `DrawRarityStars`(Task 2 `ComputeStarRects` → 별 텍스처 타일, 폴백=문자 ★)도 동일 패턴으로 구현. **`FCanvasTileItem`/`SE_BLEND_Translucent`/`Canvas->DrawItem` 실제 시그니처를 기존 코드(스킬 아이콘 등 DrawTile 사용처)에서 확인 후 맞춘다.**

- [ ] **Step 3: 빌드 GREEN + 전체 Automation 유지**. Run: 전체 게이트. Expected: 컴파일 GREEN + 회귀 유지(헬퍼 미사용이라 외형 무변경).

- [ ] **Step 4: 커밋** `git commit -m "feat(visual): 크롬 9-slice 드로잉 헬퍼(패널/버튼/별, 폴백 포함)"`

### Task 5: nav 셸 + 상시요소 리스타일

**Files:** Modify `UI/IdleHUD.cpp`

- [ ] **Step 1:** `DrawCategoryRail`/`DrawCategoryTabBar`/`DrawPanelSubTabs`의 칩 배경 `DrawRect(...)` → `DrawPanelChrome(..., 활성?Slate:Cream)`, 라벨 색 → `TextSlate`/활성 `TextOnSlate`. 닫기 X 배경 = 골드 버튼.
- [ ] **Step 2:** 상시요소 — `DrawStageIndicator`/`DrawBossBar`(트랙=크림 패널, 필=골드 그라데), 스킬 슬롯(`DrawSkillSlot` 프레임=골드), 궁극기 게이지(골드), 속성 범례(크림 패널)에 헬퍼 적용. 위치/로직 불변.
- [ ] **Step 3:** 빌드 + 전체 Automation GREEN(외형만). PIE 오프스크린 스크린샷(데스크톱) — 레일/상시요소 원신 톤 확인.
- [ ] **Step 4:** 커밋 `refactor(visual): nav 셸 + 상시요소 원신 리스타일`

---

## Phase 3 — 카테고리 패널 리스타일

> **공통 레시피:** 각 `Draw*Panel`에서 ① 배경 `DrawRect(Theme::BgPanel..., X,Y,W,H)` → `DrawPanelChrome(X,Y,W,H, EPanelStyle::Cream)`(강조 패널은 Slate), ② 헤더/라벨/수치 색을 `Theme::TextSlate/TextWarmGray`로, ③ 액션 버튼 `DrawRect`+텍스트 → `DrawGoldButton(...)`, ④ 등급 표시처 `DrawRarityStars`. 위치/행/히트박스 불변. 각 그룹 후 빌드+Automation+PIE.

### Task 6: 전투 패널 리스타일(타워/던전/주간보스)
- [ ] `DrawTowerPanel`/`DrawDungeonPanel`/`DrawWeeklyBossPanel`에 레시피 적용 → 빌드+회귀+PIE → 커밋 `refactor(visual): 전투 패널 원신 리스타일`

### Task 7: 성장 패널(스탯분배/스탯정보/마스터리)
- [ ] 레시피 적용(`DrawStatAllocationPanel`/`DrawStatInfoPanel`/`DrawMasteryPanel`) → 빌드+회귀+PIE → 커밋

### Task 8: 환생 패널(환생/초월/환생퍼크)
- [ ] `DrawRebirthPanel`/`DrawTranscendPanel`/`DrawRebirthPerkPanel` → 커밋

### Task 9: 장비 패널(강화/잠재/룬/룬도감/상점) — 등급 별 활용처
- [ ] `DrawEnhancePanel`/`DrawPotentialPanel`/`DrawRunePanel`/`DrawRuneCodexPanel`/`DrawShopPanel`. 룬/장비 등급에 `DrawRarityStars` → 커밋

### Task 10: 수집 패널(펫/칭호/업적/보물상자)
- [ ] `DrawPetPanel`/`DrawTitlePanel`/`DrawAchievementPanel`/`DrawTreasureBoxPanel` → 커밋

### Task 11: 일일 패널(퀘스트/미션/출석/소비/시즌패스)
- [ ] `DrawQuestLog`/`DrawMissionPanel`/`DrawAttendancePanel`/`DrawConsumablePanel`/`DrawSeasonPassPanel` → 커밋

### Task 12: 소셜 패널(길드/리더보드) + 모달
- [ ] `DrawGuildPanel`/`DrawLeaderboardPanel` + 모달(`DrawOfflineRewardModal`/`DrawClassSelectionPanel`/`DrawClassSelectionOption`) 리스타일 → 커밋

---

## Phase 4 — 렌더 파운데이션

### Task 13: PostProcessVolume + 라이팅 튠

**Files:** Modify `client/Source/IdleProject/IdleProjectGameModeBase.cpp` (+ `.h` 멤버)

> 기존 `BeginPlay`/스폰 코드(ThemeSun/ThemeSky/ThemeFog, 57~102행 인근)를 먼저 읽고 같은 스타일로.

- [ ] **Step 1:** include `Engine/PostProcessVolume.h`. 멤버 `TWeakObjectPtr<class APostProcessVolume> ThemePostProcess;`
- [ ] **Step 2:** 스폰 + 설정:
```cpp
ThemePostProcess = World->SpawnActor<APostProcessVolume>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
if (APostProcessVolume* PP = ThemePostProcess.Get())
{
    PP->bUnbound = true;
    FPostProcessSettings& S = PP->Settings;
    S.bOverride_WhiteTemp = true;        S.WhiteTemp = 6800.0f;
    S.bOverride_BloomIntensity = true;   S.BloomIntensity = 0.62f;
    S.bOverride_BloomThreshold = true;   S.BloomThreshold = 1.0f;
    S.bOverride_AmbientOcclusionIntensity = true; S.AmbientOcclusionIntensity = 0.5f;
    S.bOverride_VignetteIntensity = true; S.VignetteIntensity = 0.4f;
    S.bOverride_ColorSaturation = true;  S.ColorSaturation = FVector4(1.08f,1.08f,1.08f,1.0f);
    S.bOverride_ColorContrast = true;    S.ColorContrast = FVector4(1.05f,1.05f,1.05f,1.0f);
}
```
- [ ] **Step 3:** 라이팅 튠 — `ThemeSun`(색온도 따뜻·강도↑·SourceAngle↑), `ThemeSky` Intensity↑, `ThemeFog` 밀도/색 재조정(기존 setter 사용).
- [ ] **Step 4:** 빌드 GREEN + 전체 Automation(런타임 스폰이라 테스트 무관, 회귀 0). PIE 오프스크린 스크린샷 — 색감/블룸/비네트 확인.
- [ ] **Step 5:** 커밋 `feat(visual): PostProcessVolume 그레이드 + 라이팅 튠`

### Task 14: SkyAtmosphere

**Files:** Modify `IdleProject.Build.cs`, `IdleProjectGameModeBase.cpp`

- [ ] **Step 1:** `IdleProject.Build.cs` `PublicDependencyModuleNames`에 SkyAtmosphere 관련 모듈 확인/추가(필요 시; `ASkyAtmosphere`는 Engine에 포함 — 모듈 누락 시 링크에러로 판별). 빌드로 검증.
- [ ] **Step 2:** include `Components/SkyAtmosphereComponent.h`/`Engine/SkyAtmosphere.h`. `ASkyAtmosphere` 스폰. `ThemeSun`을 atmosphere sun light로 동작하게(DirectionalLight의 `bUsedAsAtmosphereSunLight=true` 또는 `SetAtmosphereSunLight`). 기존 `ThemeSkySphere`(절차 스카이)는 비활성/숨김 판단 — 우선 숨기고 PIE 비교.
- [ ] **Step 3:** 빌드 GREEN + Automation. PIE 스크린샷 — 대기 산란 하늘 확인.
- [ ] **Step 4:** 커밋 `feat(visual): SkyAtmosphere 하늘`

### Task 15: 툰 아웃라인 머티리얼

**Files:** Create `Commandlets/GenerateToonOutlineCommandlet.cpp`(또는 Task 3 커맨드릿 확장); Modify `IdleProjectGameModeBase.cpp`

- [ ] **Step 1:** 포스트프로세스 머티리얼 `M_ToonOutline` 절차 생성(커맨드릿, `UMaterialFactoryNew` + `MaterialDomain=MD_PostProcess`, SceneTexture(Depth/Normal) 엣지 검출 노드 그래프 — 기존 `GenerateMapThemeMaterialCommandlet`의 `UMaterialEditingLibrary` 노드 생성 패턴 따름). 복잡하면 깊이 기반 단순 외곽선부터.
- [ ] **Step 2:** 커맨드릿 실행 → `/Game/UI/Chrome/M_ToonOutline.uasset` 또는 `/Game/Render/M_ToonOutline`. LFS 커밋.
- [ ] **Step 3:** `IdleProjectGameModeBase`에서 lazy 로드 후 `ThemePostProcess->Settings.WeightedBlendables`에 추가(에셋 무효 시 미적용 폴백).
- [ ] **Step 4:** 빌드 + Automation + PIE 스크린샷(외곽선 확인) → 커밋 `feat(visual): 툰 아웃라인 포스트프로세스`

---

## Phase 5 — 검증

### Task 16: PIE 종합 시각 검증 + 최종 회귀

- [ ] **Step 1:** 전체 게이트 `ue-automation.ps1` (표준 jumbo + 전체 Automation) GREEN, 카운트 = 기존 + 신규 chrome 2테스트.
- [ ] **Step 2:** PIE 오프스크린(헤드리스 §4) — 데스크톱/모바일 × 대표 카테고리 패널, 포스트프로세스 색감·블룸·SkyAtmosphere·툰 아웃라인. ([[project-hud-nav-shell]] QA 방식: env로 카테고리/모드 강제+모달 비표시, 캡처 후 되돌림.)
- [ ] **Step 3:** QA 스크린샷 `docs/qa/2026-05-31-visual-foundation/`(LFS 제외 일반 blob — PR 코멘트 인라인 렌더, [[project-hud-nav-shell]] 교훈) 커밋.
- [ ] **Step 4:** ODR/데드코드 점검 + 커밋 `chore(visual): 최종 회귀 + QA`

---

## Self-Review (작성자 점검)

- **Spec 커버리지:** §2 토큰→T1. §3 커맨드릿/툴킷→T2·T3·T4. §4 패널·셸·상시 리스타일→T5·T6~T12. §5 포스트프로세스/라이팅/스카이/툰→T13·T14·T15. §6 단계→Phase 매핑. §8 테스트→T1·T2 단위+각 PIE+T16 게이트. **갭 없음.**
- **플레이스홀더:** 순수 로직(T1·T2)·커맨드릿(T3)·헬퍼(T4)·포스트프로세스(T13)는 실제 코드. UE API 시그니처(FCanvasTileItem/FPostProcessSettings/SkyAtmosphere/UMaterialEditingLibrary)는 "기존 사용처/선례 파일 확인 후 맞춘다"로 명시(실측 의존 불가피, 픽셀/노드 검증은 PIE).
- **타입 일관성:** `EPanelStyle`/`EBtnStyle`/`FUiRect`/`FNineSliceCell`, `ComputeNineSlice`/`PanelTextureName`/`ButtonTextureName`/`ComputeStarRects`, 헬퍼 `DrawPanelChrome`/`DrawGoldButton`/`DrawRarityStars`/`DrawNineSlice`/`ResolveChrome` — T2 정의와 T4~ 사용 일치. 텍스처명 `T_GenshinPanel(Slate)`/`T_GenshinButton*`/`T_SoftShadow`/`T_GoldStar` 커맨드릿(T3)↔로더(T4) 일치.
