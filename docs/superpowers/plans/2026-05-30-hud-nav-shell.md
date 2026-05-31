# HUD 내비게이션 셸 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** ~28개 패널을 매 프레임 모두 그리던 디버그형 HUD를, 종횡비로 PC/모바일을 자동 분기하는 적응형 내비게이션 셸(7 카테고리 2단, 활성 패널 1개)로 재구성한다.

**Architecture:** 순수 로직(enum·레지스트리·레이아웃 분기·내비 상태머신)을 신규 `UI/HudNavigation.{h,cpp}` 모듈로 분리해 Automation으로 단위 테스트한다. `AIdleHUD`는 이 모듈의 `FHudNavState`를 멤버로 들고, `DrawHUD`는 (1)상시 요소 → (2)내비 셸 → (3)활성 패널 1개 → (4)모달 순으로 그린다. 기존 `Draw*Panel` 내부 로직은 보존하고 고정 앵커만 도킹/오버레이 영역(`PanelRegion*` 멤버)으로 인자화한다.

**Tech Stack:** Unreal Engine 5.7.4, C++17, immediate-mode `AHUD`/Canvas, UE Automation(`IMPLEMENT_SIMPLE_AUTOMATION_TEST`), 로컬라이즈 `IdleProject::Localization::UI`.

**클라 전용 — 서버/세이브/parity 무관. SaveVer 29 무변경.** 선행 스펙: `docs/superpowers/specs/2026-05-30-hud-nav-shell-design.md`. 브랜치 `feat/hud-nav-shell`.

---

## File Structure

| 파일 | 책임 | 작업 |
| --- | --- | --- |
| `client/Source/IdleProject/UI/HudNavigation.h` | 내비 enum·상태 struct·free 함수 선언(순수, `IdleProject::UI` 네임스페이스) | Create |
| `client/Source/IdleProject/UI/HudNavigation.cpp` | 카테고리/패널 레지스트리 테이블 + 레이아웃 분기 + 상태머신 + 로컬키 매핑 | Create |
| `client/Source/IdleProject/Tests/HudNavigationTests.cpp` | 위 로직 Automation 단위 테스트 | Create |
| `client/Source/IdleProject/UI/IdleHUD.h` | `FHudNavState`/`EHudLayoutMode`/`PanelRegion*` 멤버 + 셸 메서드 선언 | Modify |
| `client/Source/IdleProject/UI/IdleHUD.cpp` | `DrawHUD` 재구성, `DrawNavShell`/`DrawActivePanel` 구현, 패널 앵커 이주, 히트박스 라우팅 | Modify |
| `client/Content/Localization/Game/ko/UI.csv` · `en/UI.csv` | 카테고리/패널 라벨 키 | Modify |
| `client/Source/IdleProject/Tests/LocalizationTests.cpp` | 신규 로컬키 존재 회귀 | Modify |

> **공통 빌드/게이트 (모든 커밋 전후):** `tools/ci/ue-automation.ps1` = 표준 jumbo 빌드 + 전체 IdleProject Automation. 단일 테스트만 빠르게 돌릴 땐 에디터 커맨드릿(아래 각 태스크에 명시). ODR 주의: 신규 익명 헬퍼는 동명 grep 후 추가([[reference-ue-headless-verify]]).

---

## Phase 1 — 내비게이션 로직 모듈 (순수·TDD)

### Task 1: HudNavigation 헤더 + enum/레지스트리 골격

**Files:**
- Create: `client/Source/IdleProject/UI/HudNavigation.h`
- Create: `client/Source/IdleProject/UI/HudNavigation.cpp`
- Test: `client/Source/IdleProject/Tests/HudNavigationTests.cpp`

- [ ] **Step 1: 실패 테스트 작성** — `HudNavigationTests.cpp`

```cpp
#include "Misc/AutomationTest.h"
#include "UI/HudNavigation.h"

#if WITH_DEV_AUTOMATION_TESTS

using namespace IdleProject::UI;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavCategoryRegistryTest,
    "IdleProject.UI.HUD.NavCategoryRegistry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavCategoryRegistryTest::RunTest(const FString& Parameters)
{
    // 카테고리 7개, 정해진 순서.
    const TArray<EHudCategory>& Cats = AllHudCategories();
    TestEqual(TEXT("7 categories"), Cats.Num(), 7);
    TestEqual(TEXT("first is Combat"), Cats[0], EHudCategory::Combat);
    TestEqual(TEXT("last is Social"), Cats[6], EHudCategory::Social);

    // 카테고리별 패널 수.
    TestEqual(TEXT("Combat has 3"), PanelsForCategory(EHudCategory::Combat).Num(), 3);
    TestEqual(TEXT("Gear has 5"), PanelsForCategory(EHudCategory::Gear).Num(), 5);
    TestEqual(TEXT("Daily has 5"), PanelsForCategory(EHudCategory::Daily).Num(), 5);
    TestEqual(TEXT("Social has 2"), PanelsForCategory(EHudCategory::Social).Num(), 2);

    // 역매핑: 모든 패널이 자기 카테고리로 되돌아옴.
    for (const EHudCategory Cat : Cats)
    {
        for (const EHudPanel Panel : PanelsForCategory(Cat))
        {
            TestEqual(TEXT("panel maps back to its category"), CategoryForPanel(Panel), Cat);
        }
    }
    // 전체 패널 25개(상시 요소 제외).
    int32 Total = 0;
    for (const EHudCategory Cat : Cats) { Total += PanelsForCategory(Cat).Num(); }
    TestEqual(TEXT("25 panels total"), Total, 25);
    return true;
}

#endif
```

- [ ] **Step 2: 테스트 실패 확인** — 컴파일 실패(`HudNavigation.h` 없음).
Run(표준 게이트 스크립트 — 엔진 경로 기본값 `C:\Program Files\Epic Games\UE_5.7`):
```powershell
pwsh tools/ci/ue-automation.ps1 -Filter IdleProject.UI.HUD.NavCategoryRegistry
```
Expected: 컴파일 에러(헤더 미존재).

- [ ] **Step 3: 헤더 작성** — `HudNavigation.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"

namespace IdleProject::UI
{
    // 1단 카테고리(레일/탭바 순서대로).
    enum class EHudCategory : uint8
    {
        None = 0,
        Combat,     // 타워/던전/주간보스
        Growth,     // 스탯분배/스탯정보/마스터리
        Rebirth,    // 환생/초월/환생퍼크
        Gear,       // 강화/잠재/룬/룬코덱/상점
        Collection, // 펫/칭호/업적/보물상자
        Daily,      // 퀘스트/미션/출석/소비/시즌패스
        Social,     // 길드/리더보드
    };

    // 2단 패널(서브탭).
    enum class EHudPanel : uint8
    {
        None = 0,
        Tower, Dungeon, WeeklyBoss,
        StatAlloc, StatInfo, Mastery,
        RebirthPanel, Transcend, RebirthPerk,
        Enhance, Potential, Rune, RuneCodex, Shop,
        Pet, Title, Achievement, TreasureBox,
        Quest, Mission, Attendance, Consumable, SeasonPass,
        Guild, Leaderboard,
    };

    // 레이아웃 모드.
    enum class EHudLayoutMode : uint8 { Desktop, Mobile };

    const TArray<EHudCategory>& AllHudCategories();
    const TArray<EHudPanel>& PanelsForCategory(EHudCategory Category);
    EHudCategory CategoryForPanel(EHudPanel Panel);
}
```

- [ ] **Step 4: 레지스트리 구현** — `HudNavigation.cpp`

```cpp
#include "UI/HudNavigation.h"

namespace IdleProject::UI
{
    const TArray<EHudCategory>& AllHudCategories()
    {
        static const TArray<EHudCategory> Cats = {
            EHudCategory::Combat, EHudCategory::Growth, EHudCategory::Rebirth,
            EHudCategory::Gear, EHudCategory::Collection, EHudCategory::Daily,
            EHudCategory::Social,
        };
        return Cats;
    }

    const TArray<EHudPanel>& PanelsForCategory(EHudCategory Category)
    {
        static const TArray<EHudPanel> Combat = { EHudPanel::Tower, EHudPanel::Dungeon, EHudPanel::WeeklyBoss };
        static const TArray<EHudPanel> Growth = { EHudPanel::StatAlloc, EHudPanel::StatInfo, EHudPanel::Mastery };
        static const TArray<EHudPanel> Rebirth = { EHudPanel::RebirthPanel, EHudPanel::Transcend, EHudPanel::RebirthPerk };
        static const TArray<EHudPanel> Gear = { EHudPanel::Enhance, EHudPanel::Potential, EHudPanel::Rune, EHudPanel::RuneCodex, EHudPanel::Shop };
        static const TArray<EHudPanel> Collection = { EHudPanel::Pet, EHudPanel::Title, EHudPanel::Achievement, EHudPanel::TreasureBox };
        static const TArray<EHudPanel> Daily = { EHudPanel::Quest, EHudPanel::Mission, EHudPanel::Attendance, EHudPanel::Consumable, EHudPanel::SeasonPass };
        static const TArray<EHudPanel> Social = { EHudPanel::Guild, EHudPanel::Leaderboard };
        static const TArray<EHudPanel> Empty;

        switch (Category)
        {
        case EHudCategory::Combat:     return Combat;
        case EHudCategory::Growth:     return Growth;
        case EHudCategory::Rebirth:    return Rebirth;
        case EHudCategory::Gear:       return Gear;
        case EHudCategory::Collection: return Collection;
        case EHudCategory::Daily:      return Daily;
        case EHudCategory::Social:     return Social;
        default:                       return Empty;
        }
    }

    EHudCategory CategoryForPanel(EHudPanel Panel)
    {
        for (const EHudCategory Cat : AllHudCategories())
        {
            if (PanelsForCategory(Cat).Contains(Panel))
            {
                return Cat;
            }
        }
        return EHudCategory::None;
    }
}
```

- [ ] **Step 5: 빌드 모듈에 신규 파일 인식 확인** — `IdleProject.Build.cs`는 디렉터리 전체를 컴파일하므로 별도 등록 불필요. 표준 빌드만.
Run: `pwsh tools/ci/ue-automation.ps1 -Filter IdleProject.UI.HUD.NavCategoryRegistry` (`-Filter` 기본값 "IdleProject" 전체, 좁히면 빠른 점검).
Expected: PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Source/IdleProject/UI/HudNavigation.h client/Source/IdleProject/UI/HudNavigation.cpp client/Source/IdleProject/Tests/HudNavigationTests.cpp
git commit -m "feat(hud): 내비 카테고리/패널 레지스트리 + 역매핑"
```

---

### Task 2: 레이아웃 분기(종횡비 + 히스테리시스)

**Files:**
- Modify: `client/Source/IdleProject/UI/HudNavigation.h`
- Modify: `client/Source/IdleProject/UI/HudNavigation.cpp`
- Modify: `client/Source/IdleProject/Tests/HudNavigationTests.cpp`

- [ ] **Step 1: 실패 테스트 추가** — `HudNavigationTests.cpp` 에 새 테스트 추가

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavLayoutModeTest,
    "IdleProject.UI.HUD.NavLayoutMode",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavLayoutModeTest::RunTest(const FString& Parameters)
{
    // 명확히 가로 → Desktop.
    TestEqual(TEXT("wide -> Desktop"), ResolveLayoutMode(1.78f, EHudLayoutMode::Mobile), EHudLayoutMode::Desktop);
    // 명확히 세로 → Mobile.
    TestEqual(TEXT("tall -> Mobile"), ResolveLayoutMode(0.56f, EHudLayoutMode::Desktop), EHudLayoutMode::Mobile);
    // 히스테리시스 데드존(1.25~1.35): 현재 모드 유지.
    TestEqual(TEXT("deadzone keeps Desktop"), ResolveLayoutMode(1.30f, EHudLayoutMode::Desktop), EHudLayoutMode::Desktop);
    TestEqual(TEXT("deadzone keeps Mobile"), ResolveLayoutMode(1.30f, EHudLayoutMode::Mobile), EHudLayoutMode::Mobile);
    // 경계 바깥에서만 전환.
    TestEqual(TEXT("below 1.25 from Desktop -> Mobile"), ResolveLayoutMode(1.24f, EHudLayoutMode::Desktop), EHudLayoutMode::Mobile);
    TestEqual(TEXT("above 1.35 from Mobile -> Desktop"), ResolveLayoutMode(1.36f, EHudLayoutMode::Mobile), EHudLayoutMode::Desktop);
    return true;
}
```

- [ ] **Step 2: 실패 확인** — `ResolveLayoutMode` 미선언 컴파일 에러.

- [ ] **Step 3: 선언 추가** — `HudNavigation.h` 의 `CategoryForPanel` 선언 아래에:
```cpp
    // 종횡비(SizeX/SizeY) → 레이아웃. Current를 받아 데드존(1.25~1.35)에서 깜빡임 방지.
    EHudLayoutMode ResolveLayoutMode(float AspectRatio, EHudLayoutMode Current);
```

- [ ] **Step 4: 구현 추가** — `HudNavigation.cpp` 끝의 네임스페이스 안에:
```cpp
    EHudLayoutMode ResolveLayoutMode(float AspectRatio, EHudLayoutMode Current)
    {
        constexpr float LowerThreshold = 1.25f; // 이 아래 → Mobile
        constexpr float UpperThreshold = 1.35f; // 이 위 → Desktop
        if (AspectRatio <= LowerThreshold) { return EHudLayoutMode::Mobile; }
        if (AspectRatio >= UpperThreshold) { return EHudLayoutMode::Desktop; }
        return Current; // 데드존: 유지
    }
```

- [ ] **Step 5: 통과 확인** — Run: `IdleProject.UI.HUD.NavLayoutMode`. Expected: PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Source/IdleProject/UI/HudNavigation.h client/Source/IdleProject/UI/HudNavigation.cpp client/Source/IdleProject/Tests/HudNavigationTests.cpp
git commit -m "feat(hud): 종횡비 레이아웃 분기(히스테리시스)"
```

---

### Task 3: 내비 상태머신(FHudNavState 전이)

**Files:**
- Modify: `client/Source/IdleProject/UI/HudNavigation.h`
- Modify: `client/Source/IdleProject/UI/HudNavigation.cpp`
- Modify: `client/Source/IdleProject/Tests/HudNavigationTests.cpp`

- [ ] **Step 1: 실패 테스트 추가**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavStateMachineTest,
    "IdleProject.UI.HUD.NavStateMachine",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavStateMachineTest::RunTest(const FString& Parameters)
{
    FHudNavState S;
    // 기본: 닫힘.
    TestFalse(TEXT("default closed"), IsNavOpen(S));
    TestEqual(TEXT("default category None"), S.ActiveCategory, EHudCategory::None);

    // 카테고리 선택 → 그 카테고리 첫 패널 활성.
    SelectCategory(S, EHudCategory::Gear);
    TestTrue(TEXT("open after select"), IsNavOpen(S));
    TestEqual(TEXT("Gear active"), S.ActiveCategory, EHudCategory::Gear);
    TestEqual(TEXT("first panel = Enhance"), S.ActivePanel, EHudPanel::Enhance);

    // 서브탭으로 패널 전환 → 기억됨.
    SelectPanel(S, EHudPanel::Rune);
    TestEqual(TEXT("now Rune"), S.ActivePanel, EHudPanel::Rune);

    // 다른 카테고리 갔다가 → 돌아오면 마지막 본 패널 복원.
    SelectCategory(S, EHudCategory::Social);
    TestEqual(TEXT("Social first = Guild"), S.ActivePanel, EHudPanel::Guild);
    SelectCategory(S, EHudCategory::Gear);
    TestEqual(TEXT("Gear restores Rune"), S.ActivePanel, EHudPanel::Rune);

    // 같은 카테고리 재선택 → 토글 닫힘.
    SelectCategory(S, EHudCategory::Gear);
    TestFalse(TEXT("re-select closes"), IsNavOpen(S));
    TestEqual(TEXT("category cleared"), S.ActiveCategory, EHudCategory::None);

    // 명시적 닫기.
    SelectCategory(S, EHudCategory::Daily);
    CloseNav(S);
    TestFalse(TEXT("closed by CloseNav"), IsNavOpen(S));

    // 활성 카테고리 아닌 패널 선택은 무시(방어).
    SelectCategory(S, EHudCategory::Combat);
    SelectPanel(S, EHudPanel::Shop); // Shop은 Gear → 무시
    TestEqual(TEXT("cross-category panel ignored"), S.ActivePanel, EHudPanel::Tower);
    return true;
}
```

- [ ] **Step 2: 실패 확인** — `FHudNavState`/전이 함수 미선언.

- [ ] **Step 3: 선언 추가** — `HudNavigation.h`:
```cpp
    struct FHudNavState
    {
        EHudCategory ActiveCategory = EHudCategory::None;
        EHudPanel ActivePanel = EHudPanel::None;
        TMap<EHudCategory, EHudPanel> LastPanelByCategory; // 비영속(런타임)
    };

    bool IsNavOpen(const FHudNavState& State);
    void SelectCategory(FHudNavState& State, EHudCategory Category); // 같은 카테고리 재선택=토글 닫힘
    void SelectPanel(FHudNavState& State, EHudPanel Panel);         // 활성 카테고리 내에서만
    void CloseNav(FHudNavState& State);
```

- [ ] **Step 4: 구현 추가** — `HudNavigation.cpp`:
```cpp
    bool IsNavOpen(const FHudNavState& State)
    {
        return State.ActiveCategory != EHudCategory::None && State.ActivePanel != EHudPanel::None;
    }

    void SelectCategory(FHudNavState& State, EHudCategory Category)
    {
        if (State.ActiveCategory == Category) { CloseNav(State); return; } // 토글
        const TArray<EHudPanel>& Panels = PanelsForCategory(Category);
        if (Panels.Num() == 0) { CloseNav(State); return; }
        const EHudPanel* Last = State.LastPanelByCategory.Find(Category);
        const EHudPanel Target = (Last && Panels.Contains(*Last)) ? *Last : Panels[0];
        State.ActiveCategory = Category;
        State.ActivePanel = Target;
        State.LastPanelByCategory.Add(Category, Target);
    }

    void SelectPanel(FHudNavState& State, EHudPanel Panel)
    {
        if (CategoryForPanel(Panel) != State.ActiveCategory) { return; } // 교차 카테고리 무시
        State.ActivePanel = Panel;
        State.LastPanelByCategory.Add(State.ActiveCategory, Panel);
    }

    void CloseNav(FHudNavState& State)
    {
        State.ActiveCategory = EHudCategory::None;
        State.ActivePanel = EHudPanel::None;
    }
```

- [ ] **Step 5: 통과 확인** — Run: `IdleProject.UI.HUD.NavStateMachine`. Expected: PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Source/IdleProject/UI/HudNavigation.h client/Source/IdleProject/UI/HudNavigation.cpp client/Source/IdleProject/Tests/HudNavigationTests.cpp
git commit -m "feat(hud): 내비 상태머신(카테고리 토글·패널 기억·교차 가드)"
```

---

### Task 4: 로컬라이즈 키 매핑

**Files:**
- Modify: `client/Source/IdleProject/UI/HudNavigation.h`
- Modify: `client/Source/IdleProject/UI/HudNavigation.cpp`
- Modify: `client/Source/IdleProject/Tests/HudNavigationTests.cpp`

- [ ] **Step 1: 실패 테스트 추가**

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FHudNavLocKeyTest,
    "IdleProject.UI.HUD.NavLocKey",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHudNavLocKeyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("combat key"), CategoryLocKey(EHudCategory::Combat), FString(TEXT("HUD_CAT_COMBAT")));
    TestEqual(TEXT("social key"), CategoryLocKey(EHudCategory::Social), FString(TEXT("HUD_CAT_SOCIAL")));
    TestEqual(TEXT("tower key"), PanelLocKey(EHudPanel::Tower), FString(TEXT("HUD_PANEL_TOWER")));
    TestEqual(TEXT("runecodex key"), PanelLocKey(EHudPanel::RuneCodex), FString(TEXT("HUD_PANEL_RUNECODEX")));
    // 모든 카테고리/패널이 빈 키가 아님.
    for (const EHudCategory Cat : AllHudCategories())
    {
        TestFalse(TEXT("category key non-empty"), CategoryLocKey(Cat).IsEmpty());
        for (const EHudPanel P : PanelsForCategory(Cat))
        {
            TestFalse(TEXT("panel key non-empty"), PanelLocKey(P).IsEmpty());
        }
    }
    return true;
}
```

- [ ] **Step 2: 실패 확인** — 미선언 컴파일 에러.

- [ ] **Step 3: 선언 추가** — `HudNavigation.h`:
```cpp
    FString CategoryLocKey(EHudCategory Category); // "HUD_CAT_*"
    FString PanelLocKey(EHudPanel Panel);          // "HUD_PANEL_*"
```

- [ ] **Step 4: 구현 추가** — `HudNavigation.cpp`:
```cpp
    FString CategoryLocKey(EHudCategory Category)
    {
        switch (Category)
        {
        case EHudCategory::Combat:     return TEXT("HUD_CAT_COMBAT");
        case EHudCategory::Growth:     return TEXT("HUD_CAT_GROWTH");
        case EHudCategory::Rebirth:    return TEXT("HUD_CAT_REBIRTH");
        case EHudCategory::Gear:       return TEXT("HUD_CAT_GEAR");
        case EHudCategory::Collection: return TEXT("HUD_CAT_COLLECTION");
        case EHudCategory::Daily:      return TEXT("HUD_CAT_DAILY");
        case EHudCategory::Social:     return TEXT("HUD_CAT_SOCIAL");
        default:                       return FString();
        }
    }

    FString PanelLocKey(EHudPanel Panel)
    {
        switch (Panel)
        {
        case EHudPanel::Tower:        return TEXT("HUD_PANEL_TOWER");
        case EHudPanel::Dungeon:      return TEXT("HUD_PANEL_DUNGEON");
        case EHudPanel::WeeklyBoss:   return TEXT("HUD_PANEL_WEEKLYBOSS");
        case EHudPanel::StatAlloc:    return TEXT("HUD_PANEL_STATALLOC");
        case EHudPanel::StatInfo:     return TEXT("HUD_PANEL_STATINFO");
        case EHudPanel::Mastery:      return TEXT("HUD_PANEL_MASTERY");
        case EHudPanel::RebirthPanel: return TEXT("HUD_PANEL_REBIRTH");
        case EHudPanel::Transcend:    return TEXT("HUD_PANEL_TRANSCEND");
        case EHudPanel::RebirthPerk:  return TEXT("HUD_PANEL_REBIRTHPERK");
        case EHudPanel::Enhance:      return TEXT("HUD_PANEL_ENHANCE");
        case EHudPanel::Potential:    return TEXT("HUD_PANEL_POTENTIAL");
        case EHudPanel::Rune:         return TEXT("HUD_PANEL_RUNE");
        case EHudPanel::RuneCodex:    return TEXT("HUD_PANEL_RUNECODEX");
        case EHudPanel::Shop:         return TEXT("HUD_PANEL_SHOP");
        case EHudPanel::Pet:          return TEXT("HUD_PANEL_PET");
        case EHudPanel::Title:        return TEXT("HUD_PANEL_TITLE");
        case EHudPanel::Achievement:  return TEXT("HUD_PANEL_ACHIEVEMENT");
        case EHudPanel::TreasureBox:  return TEXT("HUD_PANEL_TREASUREBOX");
        case EHudPanel::Quest:        return TEXT("HUD_PANEL_QUEST");
        case EHudPanel::Mission:      return TEXT("HUD_PANEL_MISSION");
        case EHudPanel::Attendance:   return TEXT("HUD_PANEL_ATTENDANCE");
        case EHudPanel::Consumable:   return TEXT("HUD_PANEL_CONSUMABLE");
        case EHudPanel::SeasonPass:   return TEXT("HUD_PANEL_SEASONPASS");
        case EHudPanel::Guild:        return TEXT("HUD_PANEL_GUILD");
        case EHudPanel::Leaderboard:  return TEXT("HUD_PANEL_LEADERBOARD");
        default:                      return FString();
        }
    }
```

- [ ] **Step 5: 통과 확인** — Run: `IdleProject.UI.HUD.NavLocKey`. Expected: PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Source/IdleProject/UI/HudNavigation.h client/Source/IdleProject/UI/HudNavigation.cpp client/Source/IdleProject/Tests/HudNavigationTests.cpp
git commit -m "feat(hud): 카테고리/패널 로컬라이즈 키 매핑"
```

---

### Task 5: 로컬라이즈 CSV 항목 + 회귀

**Files:**
- Modify: `client/Content/Localization/Game/ko/UI.csv`
- Modify: `client/Content/Localization/Game/en/UI.csv`
- Modify: `client/Source/IdleProject/Tests/LocalizationTests.cpp`

- [ ] **Step 1: 실패 테스트 추가** — `LocalizationTests.cpp` (기존 패턴 따라; 한 테스트에 키 존재 검증 추가). 기존 파일에서 `IdleProject::Localization::UI(TEXT(...))` 가 빈 텍스트가 아닌지 확인하는 패턴을 그대로 사용:
```cpp
// (기존 로컬 테스트 RunTest 본문에 추가)
const TArray<FString> NavKeys = {
    TEXT("HUD_CAT_COMBAT"), TEXT("HUD_CAT_GROWTH"), TEXT("HUD_CAT_REBIRTH"),
    TEXT("HUD_CAT_GEAR"), TEXT("HUD_CAT_COLLECTION"), TEXT("HUD_CAT_DAILY"), TEXT("HUD_CAT_SOCIAL"),
    TEXT("HUD_PANEL_TOWER"), TEXT("HUD_PANEL_DUNGEON"), TEXT("HUD_PANEL_WEEKLYBOSS"),
    TEXT("HUD_PANEL_STATALLOC"), TEXT("HUD_PANEL_STATINFO"), TEXT("HUD_PANEL_MASTERY"),
    TEXT("HUD_PANEL_REBIRTH"), TEXT("HUD_PANEL_TRANSCEND"), TEXT("HUD_PANEL_REBIRTHPERK"),
    TEXT("HUD_PANEL_ENHANCE"), TEXT("HUD_PANEL_POTENTIAL"), TEXT("HUD_PANEL_RUNE"),
    TEXT("HUD_PANEL_RUNECODEX"), TEXT("HUD_PANEL_SHOP"), TEXT("HUD_PANEL_PET"),
    TEXT("HUD_PANEL_TITLE"), TEXT("HUD_PANEL_ACHIEVEMENT"), TEXT("HUD_PANEL_TREASUREBOX"),
    TEXT("HUD_PANEL_QUEST"), TEXT("HUD_PANEL_MISSION"), TEXT("HUD_PANEL_ATTENDANCE"),
    TEXT("HUD_PANEL_CONSUMABLE"), TEXT("HUD_PANEL_SEASONPASS"),
    TEXT("HUD_PANEL_GUILD"), TEXT("HUD_PANEL_LEADERBOARD"),
};
for (const FString& Key : NavKeys)
{
    TestFalse(*FString::Printf(TEXT("loc key resolves: %s"), *Key),
        IdleProject::Localization::UI(*Key).IsEmpty());
}
```
> 정확한 호출 형태는 `LocalizationTests.cpp` 기존 코드를 따를 것(이미 `IdleProject::Localization::UI` 사용 중).

- [ ] **Step 2: 실패 확인** — CSV에 키 없어 빈 텍스트 → FAIL.

- [ ] **Step 3: ko/UI.csv 에 행 추가** (기존 `UI,KEY,"값","주석"` 형식, 적당한 위치):
```
UI,HUD_CAT_COMBAT,"전투","내비"
UI,HUD_CAT_GROWTH,"성장","내비"
UI,HUD_CAT_REBIRTH,"환생","내비"
UI,HUD_CAT_GEAR,"장비","내비"
UI,HUD_CAT_COLLECTION,"수집","내비"
UI,HUD_CAT_DAILY,"일일","내비"
UI,HUD_CAT_SOCIAL,"소셜","내비"
UI,HUD_PANEL_TOWER,"무한의 탑","내비"
UI,HUD_PANEL_DUNGEON,"던전","내비"
UI,HUD_PANEL_WEEKLYBOSS,"주간 보스","내비"
UI,HUD_PANEL_STATALLOC,"스탯 분배","내비"
UI,HUD_PANEL_STATINFO,"스탯 정보","내비"
UI,HUD_PANEL_MASTERY,"마스터리","내비"
UI,HUD_PANEL_REBIRTH,"환생","내비"
UI,HUD_PANEL_TRANSCEND,"초월","내비"
UI,HUD_PANEL_REBIRTHPERK,"환생 특성","내비"
UI,HUD_PANEL_ENHANCE,"강화","내비"
UI,HUD_PANEL_POTENTIAL,"잠재능력","내비"
UI,HUD_PANEL_RUNE,"룬","내비"
UI,HUD_PANEL_RUNECODEX,"룬 도감","내비"
UI,HUD_PANEL_SHOP,"상점","내비"
UI,HUD_PANEL_PET,"펫","내비"
UI,HUD_PANEL_TITLE,"칭호","내비"
UI,HUD_PANEL_ACHIEVEMENT,"업적","내비"
UI,HUD_PANEL_TREASUREBOX,"보물상자","내비"
UI,HUD_PANEL_QUEST,"퀘스트","내비"
UI,HUD_PANEL_MISSION,"미션","내비"
UI,HUD_PANEL_ATTENDANCE,"출석","내비"
UI,HUD_PANEL_CONSUMABLE,"소비","내비"
UI,HUD_PANEL_SEASONPASS,"시즌 패스","내비"
UI,HUD_PANEL_GUILD,"길드","내비"
UI,HUD_PANEL_LEADERBOARD,"리더보드","내비"
```

- [ ] **Step 4: en/UI.csv 에 동일 키, 영문 값 추가**:
```
UI,HUD_CAT_COMBAT,"Combat","nav"
UI,HUD_CAT_GROWTH,"Growth","nav"
UI,HUD_CAT_REBIRTH,"Rebirth","nav"
UI,HUD_CAT_GEAR,"Gear","nav"
UI,HUD_CAT_COLLECTION,"Collection","nav"
UI,HUD_CAT_DAILY,"Daily","nav"
UI,HUD_CAT_SOCIAL,"Social","nav"
UI,HUD_PANEL_TOWER,"Tower","nav"
UI,HUD_PANEL_DUNGEON,"Dungeon","nav"
UI,HUD_PANEL_WEEKLYBOSS,"Weekly Boss","nav"
UI,HUD_PANEL_STATALLOC,"Stats","nav"
UI,HUD_PANEL_STATINFO,"Stat Info","nav"
UI,HUD_PANEL_MASTERY,"Mastery","nav"
UI,HUD_PANEL_REBIRTH,"Rebirth","nav"
UI,HUD_PANEL_TRANSCEND,"Transcend","nav"
UI,HUD_PANEL_REBIRTHPERK,"Perks","nav"
UI,HUD_PANEL_ENHANCE,"Enhance","nav"
UI,HUD_PANEL_POTENTIAL,"Potential","nav"
UI,HUD_PANEL_RUNE,"Runes","nav"
UI,HUD_PANEL_RUNECODEX,"Rune Codex","nav"
UI,HUD_PANEL_SHOP,"Shop","nav"
UI,HUD_PANEL_PET,"Pets","nav"
UI,HUD_PANEL_TITLE,"Titles","nav"
UI,HUD_PANEL_ACHIEVEMENT,"Achievements","nav"
UI,HUD_PANEL_TREASUREBOX,"Treasure","nav"
UI,HUD_PANEL_QUEST,"Quests","nav"
UI,HUD_PANEL_MISSION,"Missions","nav"
UI,HUD_PANEL_ATTENDANCE,"Attendance","nav"
UI,HUD_PANEL_CONSUMABLE,"Consumables","nav"
UI,HUD_PANEL_SEASONPASS,"Season Pass","nav"
UI,HUD_PANEL_GUILD,"Guild","nav"
UI,HUD_PANEL_LEADERBOARD,"Leaderboard","nav"
```

- [ ] **Step 5: 통과 확인** — Run: `IdleProject.UI.Localization` (기존 로컬 테스트 이름). Expected: PASS.

- [ ] **Step 6: 커밋**
```bash
git add client/Content/Localization/Game/ko/UI.csv client/Content/Localization/Game/en/UI.csv client/Source/IdleProject/Tests/LocalizationTests.cpp
git commit -m "feat(hud): 내비 카테고리/패널 라벨 로컬라이즈(ko·en)"
```

---

## Phase 2 — 내비 셸 골격 + 활성 패널 디스패치

### Task 6: AIdleHUD에 내비 상태/영역 멤버 + 메서드 선언

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h`

- [ ] **Step 1: include 추가** — `IdleHUD.h` 상단 include 블록에:
```cpp
#include "UI/HudNavigation.h"
```

- [ ] **Step 2: 멤버 추가** — `AIdleHUD` private 멤버 영역(`bQuestLogVisible` 인근, IdleHUD.h:1383 부근)에:
```cpp
// ── 내비게이션 셸 상태(런타임 전용·비영속) ──────────────────
IdleProject::UI::FHudNavState HudNav;
IdleProject::UI::EHudLayoutMode HudLayoutMode = IdleProject::UI::EHudLayoutMode::Desktop;
// 활성 패널이 그려질 도킹/오버레이 영역(DrawActivePanel이 매 프레임 설정).
float PanelRegionX = 0.0f;
float PanelRegionY = 0.0f;
float PanelRegionW = 0.0f;
float PanelRegionH = 0.0f;
```

- [ ] **Step 3: 메서드 선언 추가** — `AIdleHUD` private 메서드 선언부(`DrawHUD` 인근)에:
```cpp
void DrawNavShell();          // 종횡비 분기 → 레일(Desktop) 또는 탭바(Mobile) + 서브탭, 영역 산출
void DrawCategoryRail();      // Desktop: 좌측 세로 카테고리 레일
void DrawCategoryTabBar();    // Mobile: 하단 카테고리 탭바
void DrawPanelSubTabs();      // 활성 카테고리의 패널 서브탭(공통)
void DrawActivePanel();       // ActivePanel 하나만 PanelRegion에 그림
FName NavCategoryHitBox(IdleProject::UI::EHudCategory Category) const;
FName NavPanelHitBox(IdleProject::UI::EHudPanel Panel) const;
```

- [ ] **Step 4: 빌드 확인(선언만, 미사용)** — Run: `pwsh tools/ci/ue-automation.ps1` (또는 표준 jumbo 빌드). Expected: 컴파일 GREEN(미정의 메서드는 아직 호출 안 하므로 OK; 단 선언만 있고 정의 없으면 링크 에러 → 다음 태스크에서 정의. **이 태스크는 멤버/include까지만, 메서드 선언은 Task 7과 함께 커밋**).
> 주의: 링크 에러 방지를 위해 Step 3 선언과 Task 7 정의를 **한 커밋**으로 묶는다. 이 태스크 단독 커밋은 Step 1~2(멤버/include)만.

- [ ] **Step 5: 커밋(멤버/include만)**
```bash
git add client/Source/IdleProject/UI/IdleHUD.h
git commit -m "feat(hud): 내비 셸 상태/영역 멤버 추가"
```

---

### Task 7: 셸 렌더링 + 활성 패널 디스패치(빈 영역) + 히트박스

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h` (Task 6 Step 3 선언 포함)
- Modify: `client/Source/IdleProject/UI/IdleHUD.cpp`

> **참고:** 이 단계에서는 `DrawActivePanel`이 **영역만 산출하고 패널 본문은 아직 기존 고정 앵커로 그림**(이주는 Phase 3). 목표는 셸(레일/탭바/서브탭)이 뜨고, 클릭으로 `HudNav` 상태가 바뀌는 것까지.

- [ ] **Step 1: DrawHUD 재구성** — `IdleHUD.cpp:4512` `DrawHUD()` 본문을 아래로 교체. 기존 개별 `Draw*Panel()` 직접 호출 라인(4527~4551)을 **DrawActivePanel()로 대체**하고, 상시 요소만 남긴다:
```cpp
void AIdleHUD::DrawHUD()
{
    Super::DrawHUD();
    const UWorld* World = GetWorld();
    BindPlayerCombat();

    // 레이아웃 모드 갱신(종횡비 + 히스테리시스).
    if (Canvas && Canvas->SizeY > 0.0f)
    {
        const float Aspect = Canvas->SizeX / Canvas->SizeY;
        HudLayoutMode = IdleProject::UI::ResolveLayoutMode(Aspect, HudLayoutMode);
    }

    // (1) 상시 요소 — 셸과 무관하게 항상.
    const USkillComponent* PlayerSkills = ResolvePlayerSkills();
    if (World && PlayerSkills) { DrawSkillHud(*PlayerSkills, World->GetTimeSeconds()); }
    DrawStageIndicator();
    DrawBossBar();

    // (2) 내비 셸(레일/탭바 + 서브탭) — 영역 산출.
    DrawNavShell();

    // (3) 활성 패널 1개.
    DrawActivePanel();

    // (4) 모달/오버레이(최우선).
    DrawClassSelectionPanel();   // 최초 1회 클래스 선택
    DrawQuestLog();              // 기존 토글(bQuestLogVisible) 유지 — Phase 3에서 Quest 패널로 흡수
    DrawOfflineRewardModal();
    if (World)
    {
        DrawBossSpecialWarning(World->GetTimeSeconds());
        DrawFloatingDamageTexts(World->GetTimeSeconds());
        DrawStatusIndicators(World->GetTimeSeconds());
        DrawProgressSavedIndicator(World->GetTimeSeconds());
        DrawCloudSyncIndicator(World->GetTimeSeconds());
    }
}
```
> **중요:** 이 시점엔 `DrawActivePanel`이 아직 패널 본문을 옛 앵커로 그리므로, 위에서 빠진 패널들(Shop/Rune/...)을 임시로 `DrawActivePanel` 안의 switch에서 호출한다(Step 3). QuestLog는 기존 토글 유지(Phase 3 Task 17에서 통합).

- [ ] **Step 2: 셸 렌더링 구현** — `IdleHUD.cpp` 적당한 위치(예 `DrawHUD` 아래)에 추가:
```cpp
FName AIdleHUD::NavCategoryHitBox(IdleProject::UI::EHudCategory Category) const
{
    return FName(*FString::Printf(TEXT("NavCat_%d"), (int32)Category));
}
FName AIdleHUD::NavPanelHitBox(IdleProject::UI::EHudPanel Panel) const
{
    return FName(*FString::Printf(TEXT("NavPanel_%d"), (int32)Panel));
}

void AIdleHUD::DrawNavShell()
{
    using namespace IdleProject::UI;
    if (!Canvas) { return; }
    const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);

    if (HudLayoutMode == EHudLayoutMode::Desktop)
    {
        DrawCategoryRail();
        // 우측 도킹 영역 산출.
        const float RailW = 64.0f * Scale;
        PanelRegionW = FMath::Clamp(Canvas->SizeX * 0.34f, 380.0f * Scale, 560.0f * Scale);
        PanelRegionX = Canvas->SizeX - PanelRegionW - 16.0f * Scale;
        PanelRegionY = 92.0f * Scale;
        PanelRegionH = Canvas->SizeY - PanelRegionY - 16.0f * Scale;
        (void)RailW;
    }
    else // Mobile
    {
        DrawCategoryTabBar();
        // 중앙 오버레이 영역.
        const float TabBarH = 84.0f * Scale;
        PanelRegionW = FMath::Min(Canvas->SizeX - 24.0f * Scale, 560.0f * Scale);
        PanelRegionX = (Canvas->SizeX - PanelRegionW) * 0.5f;
        PanelRegionY = 96.0f * Scale;
        PanelRegionH = Canvas->SizeY - PanelRegionY - TabBarH - 12.0f * Scale;
    }

    if (IsNavOpen(HudNav))
    {
        DrawPanelSubTabs(); // 서브탭은 PanelRegion 상단을 차지 → 영역 보정
    }
}

void AIdleHUD::DrawCategoryRail()
{
    using namespace IdleProject::UI;
    const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
    const float W = 64.0f * Scale, H = 52.0f * Scale;
    float Y = 92.0f * Scale;
    for (const EHudCategory Cat : AllHudCategories())
    {
        const bool bActive = (HudNav.ActiveCategory == Cat);
        DrawRect(bActive ? Theme::AccentBlue : Theme::BgPanel.CopyWithNewOpacity(0.85f), 12.0f * Scale, Y, W, H);
        DrawText(Localization::UI(*CategoryLocKey(Cat)).ToString(),
            bActive ? Theme::TextPrimary : Theme::TextMuted,
            16.0f * Scale, Y + 16.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.7f * Scale);
        AddHitBox(FVector2D(12.0f * Scale, Y), FVector2D(W, H), NavCategoryHitBox(Cat), true);
        Y += H + 6.0f * Scale;
    }
}

void AIdleHUD::DrawCategoryTabBar()
{
    using namespace IdleProject::UI;
    const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
    const TArray<EHudCategory>& Cats = AllHudCategories();
    const float BarH = 84.0f * Scale;
    const float Y = Canvas->SizeY - BarH;
    const float W = Canvas->SizeX / Cats.Num();
    DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.95f), 0.0f, Y, Canvas->SizeX, BarH);
    for (int32 i = 0; i < Cats.Num(); ++i)
    {
        const bool bActive = (HudNav.ActiveCategory == Cats[i]);
        const float X = i * W;
        if (bActive) { DrawRect(Theme::AccentBlue.CopyWithNewOpacity(0.4f), X, Y, W, BarH); }
        DrawText(Localization::UI(*CategoryLocKey(Cats[i])).ToString(),
            bActive ? Theme::TextPrimary : Theme::TextMuted,
            X + 8.0f * Scale, Y + 30.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.7f * Scale);
        AddHitBox(FVector2D(X, Y), FVector2D(W, BarH), NavCategoryHitBox(Cats[i]), true);
    }
}

void AIdleHUD::DrawPanelSubTabs()
{
    using namespace IdleProject::UI;
    const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
    const TArray<EHudPanel>& Panels = PanelsForCategory(HudNav.ActiveCategory);
    const float TabH = 30.0f * Scale;
    float X = PanelRegionX;
    float Y = PanelRegionY;
    const float MaxX = PanelRegionX + PanelRegionW;
    for (const EHudPanel P : Panels)
    {
        const FString Label = Localization::UI(*PanelLocKey(P)).ToString();
        const float TabW = 86.0f * Scale;
        if (X + TabW > MaxX) { X = PanelRegionX; Y += TabH + 4.0f * Scale; } // 줄바꿈(5탭)
        const bool bActive = (HudNav.ActivePanel == P);
        DrawRect(bActive ? Theme::AccentBlue : Theme::BgPanel.CopyWithNewOpacity(0.7f), X, Y, TabW, TabH);
        DrawText(Label, bActive ? Theme::TextPrimary : Theme::TextMuted, X + 6.0f * Scale, Y + 6.0f * Scale,
            GEngine ? GEngine->GetSmallFont() : nullptr, 0.62f * Scale);
        AddHitBox(FVector2D(X, Y), FVector2D(TabW, TabH), NavPanelHitBox(P), true);
        X += TabW + 4.0f * Scale;
    }
    // 서브탭이 차지한 높이만큼 패널 본문 영역을 아래로 민다.
    const float Used = (Y + TabH) - PanelRegionY;
    PanelRegionY += Used + 8.0f * Scale;
    PanelRegionH -= Used + 8.0f * Scale;
}
```
> 토큰명은 `UIThemeTokens.h` 실제 토큰: `BgPrimary`/`BgPanel`/`TextPrimary`/`TextMuted`/`AccentGold`/`AccentBlue`/`AccentRed`. (`TextStrong`은 없음 → `TextPrimary` 사용.)

- [ ] **Step 3: DrawActivePanel(임시 디스패치)** — 이 단계에선 기존 Draw 함수를 그대로 호출(앵커 이주 전):
```cpp
void AIdleHUD::DrawActivePanel()
{
    using namespace IdleProject::UI;
    if (!IsNavOpen(HudNav)) { return; }
    switch (HudNav.ActivePanel)
    {
    case EHudPanel::Tower:        DrawTowerPanel(); break;
    case EHudPanel::Dungeon:      DrawDungeonPanel(); break;
    case EHudPanel::WeeklyBoss:   DrawWeeklyBossPanel(); break;
    case EHudPanel::StatAlloc:    DrawStatAllocationPanel(); break;
    case EHudPanel::StatInfo:     DrawStatInfoPanel(); break;
    case EHudPanel::Mastery:      DrawMasteryPanel(); break;
    case EHudPanel::RebirthPanel: DrawRebirthPanel(); break;
    case EHudPanel::Transcend:    DrawTranscendPanel(); break;
    case EHudPanel::RebirthPerk:  DrawRebirthPerkPanel(); break;
    case EHudPanel::Enhance:      DrawEnhancePanel(); break;
    case EHudPanel::Potential:    DrawPotentialPanel(); break;
    case EHudPanel::Rune:         DrawRunePanel(); break;
    case EHudPanel::RuneCodex:    DrawRuneCodexPanel(); break;
    case EHudPanel::Shop:         DrawShopPanel(); break;
    case EHudPanel::Pet:          DrawPetPanel(); break;
    case EHudPanel::Title:        DrawTitlePanel(); break;
    case EHudPanel::Achievement:  DrawAchievementPanel(); break;
    case EHudPanel::TreasureBox:  DrawTreasureBoxPanel(); break;
    case EHudPanel::Quest:        DrawQuestLog(); break;
    case EHudPanel::Mission:      DrawMissionPanel(); break;
    case EHudPanel::Attendance:   DrawAttendancePanel(); break;
    case EHudPanel::Consumable:   DrawConsumablePanel(); break;
    case EHudPanel::SeasonPass:   DrawSeasonPassPanel(); break;
    case EHudPanel::Guild:        DrawGuildPanel(); break;
    case EHudPanel::Leaderboard:  DrawLeaderboardPanel(); break;
    default: break;
    }
}
```
> QuestLog는 (3)활성패널과 (4)모달 양쪽에서 호출되면 이중 그림 → Step1 (4)에서 `DrawQuestLog()` 줄을 제거하고 Quest는 활성 패널로만 그린다. (4)에는 OfflineReward/ClassSelection/BossWarning만 남긴다.

- [ ] **Step 4: 히트박스 라우팅** — `NotifyHitBoxClick`(IdleHUD.cpp:4564) 시작부에 prefix 분기 추가:
```cpp
{
    const FString Name = BoxName.ToString();
    if (Name.StartsWith(TEXT("NavCat_")))
    {
        const int32 Id = FCString::Atoi(*Name.RightChop(7));
        IdleProject::UI::SelectCategory(HudNav, (IdleProject::UI::EHudCategory)Id);
        return;
    }
    if (Name.StartsWith(TEXT("NavPanel_")))
    {
        const int32 Id = FCString::Atoi(*Name.RightChop(9));
        IdleProject::UI::SelectPanel(HudNav, (IdleProject::UI::EHudPanel)Id);
        return;
    }
}
```

- [ ] **Step 5: 빌드 + 회귀** — Run: `pwsh tools/ci/ue-automation.ps1`. Expected: 컴파일 GREEN + 전체 Automation PASS(기존 ViewModel 테스트 무변경).

- [ ] **Step 6: PIE 수동 확인(Desktop)** — 에디터 PIE 실행 → 좌측 레일 7개 표시, 카테고리 클릭 시 해당 패널이 우측(아직 옛 앵커 위치)에 1개만 뜸, 재클릭 시 닫힘, 서브탭 전환 동작. 스크린샷 1장 저장.

- [ ] **Step 7: 커밋**
```bash
git add client/Source/IdleProject/UI/IdleHUD.h client/Source/IdleProject/UI/IdleHUD.cpp
git commit -m "feat(hud): 내비 셸 렌더링 + 활성 패널 디스패치 + 히트박스 라우팅"
```

---

## Phase 3 — 패널 앵커 → 도킹 영역 이주

> **공통 이주 레시피(모든 패널 동일):** 각 `Draw*Panel()` 함수 상단의 위치 계산을 도킹/오버레이 영역으로 바꾼다.
> 1. `const float X = <기존 앵커식>;` → `const float X = PanelRegionX;`
> 2. `const float Y = <기존 앵커식>;` → `const float Y = PanelRegionY;`
> 3. `PanelWidth` 계산에서 화면 비례 폭을 영역 폭으로 클램프: `PanelWidth = FMath::Min(PanelWidth, PanelRegionW);` (계산 직후 1줄 추가)
> 4. 패널이 `Canvas->SizeX`/고정 좌표로 **우측/중앙 정렬**하던 내부 요소가 있으면 `X` 기준 상대 좌표로 바뀌므로 PIE에서 넘침 확인.
> 5. **특수 패널 주의:** `DrawStatInfoPanel`은 `ToggleY` 의존 → ToggleY를 `PanelRegionY`로. `DrawRuneCodexPanel`은 `RunePanelX` 의존(룬 패널 옆에 붙던 도감) → 단독 패널이 되므로 `X = PanelRegionX`로 단순화. `DrawStatAllocationPanel`은 `Canvas->SizeX - 700.0f` 고정 → `PanelRegionX`로.
>
> 각 패널 이주 후 표준 빌드 + 해당 카테고리 패널 PIE 확인. 카테고리 단위로 커밋.

### Task 8: Combat 카테고리 패널 이주(타워/던전/주간보스)

**Files:** Modify `client/Source/IdleProject/UI/IdleHUD.cpp`

대상 앵커(레시피 적용):
- `DrawTowerPanel`(L7974): `X=(SizeX-PanelWidth)*0.5f` `Y=214*Scale` → 영역.
- `DrawDungeonPanel`(L8069): `X=(SizeX-PanelWidth)*0.5f` `Y=428*Scale` → 영역.
- `DrawWeeklyBossPanel`(L8178): `X=(SizeX-PanelWidth)*0.5f` `Y=650*Scale` → 영역.

- [ ] **Step 1:** 세 함수에 이주 레시피 1~3 적용.
- [ ] **Step 2:** 표준 빌드 GREEN + 전체 Automation PASS. Run: `pwsh tools/ci/ue-automation.ps1`.
- [ ] **Step 3:** PIE — '전투' 카테고리 → 타워/던전/주간보스 서브탭이 도킹 영역에 정상 표시. 스크린샷.
- [ ] **Step 4: 커밋** `git commit -m "refactor(hud): 전투 카테고리 패널 도킹 영역 이주"`

### Task 9: Growth 카테고리(스탯분배/스탯정보/마스터리)
**Files:** Modify `IdleHUD.cpp`
- `DrawStatAllocationPanel`(L7045): `X=SizeX-700*Scale` `Y=92*Scale` → 영역.
- `DrawStatInfoPanel`(L7322): `X=SizeX-PanelWidth-28` `Y=ToggleY+38` → 영역(ToggleY→PanelRegionY, 특수 5).
- `DrawMasteryPanel`(L9660): `X=SizeX-PanelWidth-28` `Y=454*Scale` → 영역.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE → Step 4 커밋 `refactor(hud): 성장 카테고리 패널 도킹 영역 이주`

### Task 10: Rebirth 카테고리(환생/초월/환생퍼크)
**Files:** Modify `IdleHUD.cpp`
- `DrawRebirthPanel`(L7804): `Y=304` → 영역.
- `DrawTranscendPanel`(L7885): `Y=522` → 영역.
- `DrawRebirthPerkPanel`(L7168): `X=28` `Y=454` → 영역.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE → Step 4 커밋 `refactor(hud): 환생 카테고리 패널 도킹 영역 이주`

### Task 11: Gear 카테고리(강화/잠재/룬/룬코덱/상점, 서브탭 5)
**Files:** Modify `IdleHUD.cpp`
- `DrawEnhancePanel`(L6766): `Y=628` → 영역.
- `DrawPotentialPanel`(L6898): `Y=238` → 영역.
- `DrawRunePanel`(L6121): `Y=282` → 영역.
- `DrawRuneCodexPanel`(L6250): `X=Max(28, RunePanelX-PanelWidth-12)` → 영역(특수 5: 단독화).
- `DrawShopPanel`(L5730): `Y=92` → 영역.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE(서브탭 5개 줄바꿈 확인) → Step 4 커밋 `refactor(hud): 장비 카테고리 패널 도킹 영역 이주`

### Task 12: Collection 카테고리(펫/칭호/업적/보물상자)
**Files:** Modify `IdleHUD.cpp`
- `DrawPetPanel`(L9716): `X=28` `Y=300` → 영역.
- `DrawTitlePanel`(L9926): `Y=28` → 영역.
- `DrawAchievementPanel`(L9594): `Y=722` → 영역.
- `DrawTreasureBoxPanel`(L8458): `Y=120` → 영역.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE → Step 4 커밋 `refactor(hud): 수집 카테고리 패널 도킹 영역 이주`

### Task 13: Daily 카테고리(퀘스트/미션/출석/소비/시즌패스, 서브탭 5)
**Files:** Modify `IdleHUD.cpp`
- `DrawMissionPanel`(L10065): `X=28` `Y=360` → 영역.
- `DrawAttendancePanel`(L8340): `X=28` `Y=120` → 영역.
- `DrawConsumablePanel`(L5863): `Y=360` → 영역.
- `DrawSeasonPassPanel`(L10204): `X=28` `Y=724` → 영역.
- `DrawQuestLog`(L7680): `Y=92` → 영역. **+ 기존 `bQuestLogVisible` 토글 제거**: Quest는 이제 활성 패널로만 표시. `DrawHUD` (4)모달에서 `DrawQuestLog()` 호출 제거(Task 7 Step3 주석 반영 확인), 기존 QuestLog 여는 핫박스/입력이 있으면 `SelectCategory(HudNav, Daily)+SelectPanel(Quest)`로 대체하거나 제거.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE → Step 4 커밋 `refactor(hud): 일일 카테고리 패널 도킹 영역 이주 + 퀘스트 토글 통합`

### Task 14: Social 카테고리(길드/리더보드)
**Files:** Modify `IdleHUD.cpp`
- `DrawGuildPanel`(L8551): `X=28` `Y=120` → 영역. (길드 패널은 내부 탭/스크롤이 큼 → PanelRegionH 안에서 스크롤/클램프 확인)
- `DrawLeaderboardPanel`(L5981): `Y=688` → 영역.
- [ ] Step 1 이주 → Step 2 빌드/Automation → Step 3 PIE(길드 큰 패널 영역 내 수용) → Step 4 커밋 `refactor(hud): 소셜 카테고리 패널 도킹 영역 이주`

---

## Phase 4 — 모바일 오버레이 마감 + 회귀

### Task 15: 모바일(A) 오버레이 디밍 + 닫기 X

**Files:** Modify `client/Source/IdleProject/UI/IdleHUD.cpp`, `IdleHUD.h`

- [ ] **Step 1:** `DrawActivePanel` 진입 시 Mobile 모드면 패널 뒤 전체 디밍 + 닫기 X 히트박스:
```cpp
void AIdleHUD::DrawActivePanel()
{
    using namespace IdleProject::UI;
    if (!IsNavOpen(HudNav)) { return; }
    if (HudLayoutMode == EHudLayoutMode::Mobile && Canvas)
    {
        DrawRect(FLinearColor(0,0,0,0.45f), 0, 0, Canvas->SizeX, Canvas->SizeY); // 디밍
        const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
        const float S = 34.0f * Scale;
        const float CX = PanelRegionX + PanelRegionW - S, CY = PanelRegionY - S - 4.0f * Scale;
        DrawRect(Theme::BgPanel, CX, CY, S, S);
        DrawText(TEXT("X"), Theme::TextPrimary, CX + 11.0f * Scale, CY + 7.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f * Scale);
        AddHitBox(FVector2D(CX, CY), FVector2D(S, S), FName(TEXT("NavClose")), true);
    }
    // ... 기존 switch(Step Task7-3) ...
}
```
> 디밍은 서브탭/패널보다 **먼저** 그려야 하므로 `DrawNavShell`의 `DrawPanelSubTabs` 호출 순서를 `DrawActivePanel` 안으로 옮기거나, 디밍을 `DrawActivePanel` 최상단·서브탭을 그 뒤로 조정. 실제 순서: 디밍 → 서브탭 → 패널 본문. (필요 시 `DrawPanelSubTabs()` 호출을 `DrawActivePanel`로 이동.)

- [ ] **Step 2:** `NotifyHitBoxClick` 에 닫기 라우팅:
```cpp
if (BoxName == FName(TEXT("NavClose"))) { IdleProject::UI::CloseNav(HudNav); return; }
```

- [ ] **Step 3:** 빌드 + 전체 Automation. Run: `pwsh tools/ci/ue-automation.ps1`. Expected: GREEN.

- [ ] **Step 4: PIE 수동(Mobile 종횡비)** — 에디터 New Editor Window를 세로 해상도(예 720x1280)로 → 하단 탭바, 카테고리 탭 → 중앙 오버레이 + 디밍 + X 닫기, 전투 씬 뒤로 비침 확인. 스크린샷.

- [ ] **Step 5: 커밋** `git commit -m "feat(hud): 모바일 오버레이 디밍 + 닫기 버튼"`

### Task 16: 최종 회귀 + 데드 코드 정리

**Files:** Modify `client/Source/IdleProject/UI/IdleHUD.cpp` (정리), 필요 시 `IdleHUD.h`

- [ ] **Step 1:** 더 이상 직접 호출되지 않는 경로 점검: 기존 `bStatInfoVisible` 토글 등 옛 가시성 멤버가 새 흐름과 충돌하지 않는지 grep. 충돌/데드면 제거하거나 새 흐름에 맞춤.
- [ ] **Step 2:** ODR 점검 — 새로 추가한 익명 헬퍼/함수명 동명 충돌 grep([[reference-ue-headless-verify]] §1-b).
- [ ] **Step 3:** 전체 게이트 — Run: `pwsh tools/ci/ue-automation.ps1`. Expected: 표준 jumbo 빌드 GREEN + 전체 IdleProject Automation **N/0**(증가분 = 신규 HudNavigation 4테스트 + Localization 회귀).
- [ ] **Step 4:** PIE 종합 — Desktop/Mobile 각각 7카테고리 25패널 진입·전환·닫기, 상시 요소(스킬/스테이지/보스/궁극기/플로팅/상태/동기화) 항상 노출, 모달 우선순위(오프라인 보상이 셸 위), 전투 씬 가시성. 스크린샷 모음.
- [ ] **Step 5: 커밋** `git commit -m "chore(hud): 옛 가시성 토글 정리 + 최종 회귀"`

---

## Self-Review (작성자 점검 완료)

- **Spec 커버리지:** §3 적응형 분기→Task2/Task7(Step1 종횡비). §4 7카테고리 2단→Task1 레지스트리+Task5 라벨+Task7 서브탭. §5 상시 요소→Task7 DrawHUD (1). §6 상호작용(토글/기억/닫기/기본닫힘)→Task3 상태머신+Task7 라우팅+Task15 닫기. §7 구현(enum/레지스트리/디스패치/앵커 인자화)→Task1·6·7·8~14. §8 세이브 무변경→전 태스크 클라 전용·세이브 무접근. §9 리스크(점진 이주/히스테리시스/PIE)→Phase3 카테고리 단위+Task2 데드존+각 PIE step. §10 테스트→Phase1 단위+각 Task PIE+Task16 게이트. **갭 없음.**
- **플레이스홀더 스캔:** 코드 단계 모두 실제 코드. PIE 픽셀 검증은 본질상 수동(스크린샷)으로 명시 — 코드 스텁 아님.
- **타입 일관성:** `EHudCategory`/`EHudPanel`/`EHudLayoutMode`/`FHudNavState`, 함수 `AllHudCategories`/`PanelsForCategory`/`CategoryForPanel`/`ResolveLayoutMode`/`SelectCategory`/`SelectPanel`/`CloseNav`/`IsNavOpen`/`CategoryLocKey`/`PanelLocKey` — Phase1 정의와 Phase2~4 사용 시그니처 일치. enum 멤버명 `RebirthPanel`(패널)·`EHudCategory::Rebirth`(카테고리) 구분 일관.
