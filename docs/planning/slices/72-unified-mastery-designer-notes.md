# PR #72 Designer Notes - Unified Mastery

## HUD Layout

- Mastery panel: right-side utility stack, below the combat/tower/dungeon lane
  and above the bottom skill HUD. Use `--bg-panel` over `--bg-primary` with the
  existing 4-8px radius and 1.0-2.0 Canvas-height scale clamp.
- Header: `MASTERY_PANEL_TITLE` on the left, `MASTERY_WORLD_POWER_FORMAT` on the
  right. World Power is the permanent sum of the six track levels and should sit
  visually beside CP in the header hierarchy.
- Rows: six fixed rows in this order: Combat, Equipment, Abyss, Rune, Beast,
  Explore. Each row shows localized name, `MASTERY_TRACK_LEVEL_FORMAT`, a stable
  XP bar, `MASTERY_TRACK_XP_FORMAT`, and one current bonus summary.
- XP bar: bind to `UMasteryService::GetTrackLevelInfo(Track)`. Use
  `XpIntoLevel / XpToNext` for fill, clamp to 0-1, and format large XP values
  with thousands separators.
- Bonus summary: Combat/Equipment/Explore display core stat pressure through
  `MASTERY_BONUS_CORE_FORMAT`; Rune uses crit; Abyss uses drop; Beast can show
  gold first and EXP in tooltip or secondary text.

## Copy And Tooltips

- ko/en UI CSV keys are paired for the panel title, World Power, six track
  labels, level/XP/progress formats, five bonus formats, and two tooltips.
- `MASTERY_TOOLTIP_LOCKED` explains that linked systems grant mastery XP.
- `MASTERY_TOOLTIP_WORLD_POWER` explains persistence across rebirth and
  transcendence so players do not confuse it with resettable CP.

## Responsive And Asset Notes

- 1080p: keep panel width in the 360-460px range and row height dense enough for
  all six rows without covering skill slots.
- 1440p / 4K: scale by Canvas height only; do not introduce viewport-width font
  scaling or new hard-coded colors.
- No binary UI assets were added in this designer pass. Existing `.gitattributes`
  LFS patterns cover future `.uasset`, `.png`, `.jpg`, `.psd`, and related art
  exports.

## Character Interface Dependency

- HUD ViewModel should prefer the new BlueprintPure
  `UMasteryService::GetTrackLevelInfo(Track)` for level, total XP, XP-in-level,
  and XP-to-next in one bindable struct.
- Existing BlueprintPure getters remain available for header and summary binds:
  `GetTrackLevel`, `GetTrackTotalXp`, `GetWorldPower`, and `GetGlobalBonus`.
