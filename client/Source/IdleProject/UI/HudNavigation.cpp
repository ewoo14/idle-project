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

    bool IsNavOpen(const FHudNavState& State)
    {
        return State.ActiveCategory != EHudCategory::None && State.ActivePanel != EHudPanel::None;
    }

    void SelectCategory(FHudNavState& State, EHudCategory Category)
    {
        if (State.ActiveCategory == Category) { CloseNav(State); return; } // 토글 닫기
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
}
