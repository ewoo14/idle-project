# PR #34 Codex Character Implementation Notes

Date: 2026-05-26

## Scope

- Added client `FStatPointFormula` and `EPrimaryStat` in `client/Source/IdleProject/CharacterSystem/`.
- V1 grants 5 allocatable stat points for each level-up to level 2 or higher.
- Total allocatable points for a level are `5 * max(Level - 1, 0)`.

## Runtime Behavior

- `UIdleGameInstance` owns `AvailableStatPoints`, `AllocatedStats`, `GrantStatPoints`, `AllocateStatPoint`, `ResetStatPoints`, and `OnStatPointsChanged`.
- `LevelUp()` grants stat points before broadcasting `OnLevelUp`.
- `Rebirth()` resets available and allocated stat points along with level/EXP/gold reset state.
- `AIdleCharacter::RefreshDerivedStats()` adds allocated primary stats before calling `DeriveStats`, so STR/DEX/INT/WIS/CON/LUK allocation immediately flows into derived combat stats.

## Verification

- Build: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- Automation: `UnrealEditor-Cmd.exe ... Automation RunTests IdleProject` exited 0.
- Added automation coverage for formula grants/totals, zero-point allocation guard, allocation spend, reset refund, level-up grant, rebirth reset, and allocated STR derived-stat impact.
