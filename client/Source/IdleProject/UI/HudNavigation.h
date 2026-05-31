#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"

namespace IdleProject::UI
{
    // HUD 최상위 내비 카테고리(7개 고정).
    enum class EHudCategory : uint8
    {
        None = 0,
        Combat, Growth, Rebirth, Gear, Collection, Daily, Social,
    };

    // 카테고리별 세부 패널(총 25개).
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

    // 화면 종횡비로 결정되는 레이아웃 모드.
    enum class EHudLayoutMode : uint8 { Desktop, Mobile };

    // 등록된 모든 HUD 카테고리 배열을 반환한다.
    const TArray<EHudCategory>& AllHudCategories();
    // 주어진 카테고리에 속하는 패널 배열을 반환한다.
    const TArray<EHudPanel>& PanelsForCategory(EHudCategory Category);
    // 패널이 속한 상위 카테고리를 반환한다.
    EHudCategory CategoryForPanel(EHudPanel Panel);
    // 종횡비(SizeX/SizeY) → 레이아웃. Current를 받아 데드존(1.25~1.35)에서 깜빡임 방지.
    EHudLayoutMode ResolveLayoutMode(float AspectRatio, EHudLayoutMode Current);

    // 내비게이션 런타임 상태(비영속 — 세이브 안 함).
    struct FHudNavState
    {
        EHudCategory ActiveCategory = EHudCategory::None;
        EHudPanel ActivePanel = EHudPanel::None;
        TMap<EHudCategory, EHudPanel> LastPanelByCategory; // 카테고리별 마지막 본 패널
    };

    bool IsNavOpen(const FHudNavState& State);                       // 패널이 열려 있는가
    void SelectCategory(FHudNavState& State, EHudCategory Category); // 같은 카테고리 재선택=토글 닫힘
    void SelectPanel(FHudNavState& State, EHudPanel Panel);          // 활성 카테고리 내에서만 전환
    void CloseNav(FHudNavState& State);                             // 닫기

    // 카테고리 라벨 로컬라이즈 키("HUD_CAT_*").
    FString CategoryLocKey(EHudCategory Category);
    // 패널 라벨 로컬라이즈 키("HUD_PANEL_*").
    FString PanelLocKey(EHudPanel Panel);
}
