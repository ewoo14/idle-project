# PR #76 Designer Notes - Leaderboard HUD

## HUD Layout

- Add a compact right-side leaderboard panel for seasonal `Power` and `Rebirth`
  rankings.
- Header shows `LEADERBOARD_PANEL_TITLE` and `LEADERBOARD_SEASON_FORMAT`.
- Power/Rebirth tabs switch the displayed `ELeaderboardKind`; refresh calls
  `UIdleGameInstance::RefreshLeaderboard(SelectedKind)`.
- Top-N rows show rank, character id, and score. The current character row is
  highlighted when `ULeaderboardService::GetMyEntry(Kind).CharacterId` matches a
  top-N row.
- A separate `My Rank` row always reads from
  `ULeaderboardService::GetMyEntry(Kind)`, so players outside top-N still see
  their own rank when the backend returns it.

## States

- Loading state uses `LEADERBOARD_LOADING`.
- Offline state uses `LEADERBOARD_OFFLINE` and the existing red warning accent.
- Empty top-N cache uses `LEADERBOARD_EMPTY`.
- The panel is read-only except for tabs and refresh. It does not mutate save
  data and does not add reward claiming.

## Tokens And Responsive Rules

- Use existing theme tokens only: `BgPanel`, `BgPrimary`, `TextPrimary`,
  `TextMuted`, `AccentGold`, `AccentBlue`, and `AccentRed`.
- Keep Canvas-height scale clamped to `1.0-2.0` for 1080p, 1440p, and 4K.
- Width is clamped to a compact right-side range and the list is capped to
  eight visible rows to avoid overlap with bottom skill HUD and existing utility
  panels.

## Localization And Assets

- ko/en `UI.csv` keys added:
  `LEADERBOARD_PANEL_TITLE`, `LEADERBOARD_TAB_POWER`,
  `LEADERBOARD_TAB_REBIRTH`, `LEADERBOARD_SEASON_FORMAT`,
  `LEADERBOARD_MY_RANK`, `LEADERBOARD_SCORE_FORMAT`, `LEADERBOARD_EMPTY`,
  `LEADERBOARD_LOADING`, `LEADERBOARD_OFFLINE`, and `ACTION_REFRESH`.
- `IdleProject.Localization.CsvIntegrity` covers key parity.
- No binary art assets were added. Existing `.gitattributes` LFS patterns cover
  future `.uasset` and `.png` exports.

## Verification

- `IdleProject.UI.HUD.LeaderboardPanelViewModel` covers tab labels, season
  formatting, top-N rows, score formatting, my-rank highlight, loading state,
  offline state, and refresh copy.
- `IdleProject.Leaderboard` remains the service-level JSON parsing regression
  suite.
