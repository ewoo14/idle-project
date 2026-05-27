# PR #55 Codex Notes - character/designer

## Scope

- Added client achievement formula/catalog and `UAchievementService`.
- Added server `achievement.ts` formula mirror and parity tests.
- Wired achievement progress into existing gameplay hooks:
  monster kills, enhancement attempts, gear rolls, item collection, gold earn/spend,
  level reached, rebirth, transcend, tower floor, pet feed, quest claims, season
  rewards, and offline claims.
- Persisted achievement metric values and unlocked tiers through `UIdleSaveGame`
  `clientSave` state.
- Extended final character stat composition to:
  `Transcend * TowerMilestone * Achievement`.
- Added Canvas HUD achievement panel with category progress rows, total points,
  stat multiplier, next-threshold labels, and unlock feedback toast.
- Added ko/en UI localization keys for the achievement panel, category labels,
  and `Achievement Unlocked: <name> (Tier N)` feedback.

## Notes

- Achievement stat multiplier is `1 + TotalPoints * 0.01`, neutral at zero
  points.
- The initial catalog has 22 definitions across 9 categories.
- Metric semantics are per definition: cumulative metrics add progress, maximum
  metrics keep the highest observed value.
- Existing progression tests were updated because level/rebirth/transcend setup
  now legitimately creates achievement points that affect final derived stats.
- Designer HUD uses existing `docs/planning/ui-tokens.json` colors through
  `UIThemeTokens` and keeps the panel in the right-side Canvas stack.

## Verification

- `Build.bat IdleProjectEditor Win64 Development`: passed on UE 5.7.
- UE Automation `IdleProject`: passed, exit code 0.
- UE Automation `IdleProject.UI.HUD.AchievementPanelViewModel`: passed, exit
  code 0.
- UE Automation `IdleProject.Localization.CsvIntegrity`: passed, exit code 0.
- `npm run build`: passed.
- `npm test`: 35 files passed, 396 tests passed, 1 skipped.
- `npm run lint`: passed.
