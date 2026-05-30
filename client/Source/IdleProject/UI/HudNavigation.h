#pragma once

#include "CoreMinimal.h"

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
}
