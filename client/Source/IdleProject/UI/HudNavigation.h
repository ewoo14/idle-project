#pragma once

#include "CoreMinimal.h"
#include "Containers/Map.h"

namespace IdleProject::UI
{
    enum class EHudCategory : uint8
    {
        None = 0,
        Combat, Growth, Rebirth, Gear, Collection, Daily, Social,
    };

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

    enum class EHudLayoutMode : uint8 { Desktop, Mobile };

    const TArray<EHudCategory>& AllHudCategories();
    const TArray<EHudPanel>& PanelsForCategory(EHudCategory Category);
    EHudCategory CategoryForPanel(EHudPanel Panel);
}
