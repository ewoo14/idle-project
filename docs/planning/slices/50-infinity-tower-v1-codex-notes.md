# PR #50 Infinity Tower V1 Codex Notes

## Character Scope

- Added client-side `FTowerFormula` with floor requirement, clear check, and
  linear gold reward helpers.
- Added `UTowerService` with `HighestFloor`, capped batch climbing
  (`MaxClimbPerCall = 100`), next-floor requirement lookup, and
  `OnTowerClimbed`.
- Added `UIdleGameInstance` tower service ownership and `ClimbTower()` reward
  application through the current `AIdleCharacter::GetCombatPower()` path.

## Automation Coverage

- `IdleProject.GameCore.Tower.Formula`
- `IdleProject.GameCore.Tower.ServiceClimb`
- `IdleProject.GameCore.Tower.MaxClimbCap`
- `IdleProject.GameCore.IdleGameInstance.TowerHooks`

## Verification

- RED: `Build.bat IdleProjectEditor Win64 Development` failed before
  implementation on missing `GameCore/TowerFormula.h`.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.GameCore.Tower; Quit"` exited 0 and logged 3 successful Tower tests.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.GameCore.IdleGameInstance.TowerHooks; Quit"` exited 0 and logged the TowerHooks test as `Result={Success}`.

## Notes

- This character slice does not add HUD controls or the server TypeScript mirror;
  those remain the designer/backend follow-up tracks from the PR #50 plan.
