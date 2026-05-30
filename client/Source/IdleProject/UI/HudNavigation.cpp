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
        static const TArray<EHudPanel> Combat    = { EHudPanel::Tower, EHudPanel::Dungeon, EHudPanel::WeeklyBoss };
        static const TArray<EHudPanel> Growth    = { EHudPanel::StatAlloc, EHudPanel::StatInfo, EHudPanel::Mastery };
        static const TArray<EHudPanel> Rebirth   = { EHudPanel::RebirthPanel, EHudPanel::Transcend, EHudPanel::RebirthPerk };
        static const TArray<EHudPanel> Gear      = { EHudPanel::Enhance, EHudPanel::Potential, EHudPanel::Rune, EHudPanel::RuneCodex, EHudPanel::Shop };
        static const TArray<EHudPanel> Collection = { EHudPanel::Pet, EHudPanel::Title, EHudPanel::Achievement, EHudPanel::TreasureBox };
        static const TArray<EHudPanel> Daily     = { EHudPanel::Quest, EHudPanel::Mission, EHudPanel::Attendance, EHudPanel::Consumable, EHudPanel::SeasonPass };
        static const TArray<EHudPanel> Social    = { EHudPanel::Guild, EHudPanel::Leaderboard };
        static const TArray<EHudPanel> Empty;

        switch (Category)
        {
        case EHudCategory::Combat:      return Combat;
        case EHudCategory::Growth:      return Growth;
        case EHudCategory::Rebirth:     return Rebirth;
        case EHudCategory::Gear:        return Gear;
        case EHudCategory::Collection:  return Collection;
        case EHudCategory::Daily:       return Daily;
        case EHudCategory::Social:      return Social;
        default:                        return Empty;
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

    EHudLayoutMode ResolveLayoutMode(float AspectRatio, EHudLayoutMode Current)
    {
        constexpr float LowerThreshold = 1.25f; // 이 아래 → Mobile
        constexpr float UpperThreshold = 1.35f; // 이 위 → Desktop
        if (AspectRatio <= LowerThreshold) { return EHudLayoutMode::Mobile; }
        if (AspectRatio >= UpperThreshold) { return EHudLayoutMode::Desktop; }
        return Current; // 데드존: 유지
    }
}
