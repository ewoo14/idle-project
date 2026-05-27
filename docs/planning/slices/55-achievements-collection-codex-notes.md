# PR #55 Codex Notes - character/designer/balance

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
- Balance pass changed the permanent achievement stat multiplier from fully
  linear growth to a soft-capped curve:
  - `0-100` points remain linear at `1 + points * 0.01`.
  - Points above `100` use an exponential decay bonus budget of `50` effective
    points.
  - The multiplier therefore preserves early anchors (`0 => x1.00`,
    `3 => x1.03`, `100 => x2.00`) and approaches `x2.50` instead of growing
    without bound.
- `tools/balance-sim` now reports Achievement Multiplier Pressure, comparing
  legacy linear growth with the soft-capped curve under a reference
  `TranscendCount=10` and `TowerHighestFloor=100` composition.

## Notes

- Achievement stat multiplier is neutral at zero points, linear through 100
  points, then soft-capped so achievement collection remains an infinite tier
  system without overtaking transcend/tower as the primary prestige layers.
- The initial catalog has 22 definitions across 9 categories.
- Metric semantics are per definition: cumulative metrics add progress, maximum
  metrics keep the highest observed value.
- Existing progression tests were updated because level/rebirth/transcend setup
  now legitimately creates achievement points that affect final derived stats.
- Designer HUD uses existing `docs/planning/ui-tokens.json` colors through
  `UIThemeTokens` and keeps the panel in the right-side Canvas stack.
- Balance report anchor: at 250 total achievement points the old linear curve
  would be `x3.50`; the soft-capped curve is `x2.475`, and the reference
  transcend/tower/achievement composite is `x10.395` instead of unbounded
  linear growth. At 500 points, achievement is effectively `x2.50`.

## Verification

- `Build.bat IdleProjectEditor Win64 Development`: passed on UE 5.7.
- UE Automation `IdleProject`: passed, exit code 0.
- UE Automation `IdleProject.UI.HUD.AchievementPanelViewModel`: passed, exit
  code 0.
- UE Automation `IdleProject.Localization.CsvIntegrity`: passed, exit code 0.
- `npm run build`: passed.
- `npm test`: 35 files passed, 397 tests passed, 1 skipped.
- `npm run lint`: passed.
- Balance targeted RED/GREEN:
  - `npm test -- src/core/formulas/achievement.test.ts`: failed before formula
    implementation because the soft-cap constants were undefined and 125
    points still returned the old `x2.25`; passed after implementation.
  - `npm test -- tests/balance-sim.test.ts`: failed before simulator support
    because `achievementPressure` was absent; passed after implementation.
- `npm run balance:sim`: regenerated
  `tools/balance-sim/reports/balance-sim-report.md`; median first rebirth
  remains `5.324h`, with p10/p90 `4.9h/5.758h`.
