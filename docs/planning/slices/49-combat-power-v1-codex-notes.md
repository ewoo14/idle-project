# PR #49 Combat Power V1 - Codex Notes

## Character Implementation

- Added `FCombatPowerFormula::ComputeCombatPower(const FDerivedStats&)`.
- Formula returns `int64`, rounds the weighted sum, and clamps negative totals
  to zero.
- Added `AIdleCharacter::GetCombatPower()` as a `BlueprintPure` accessor over
  `GetCurrentDerivedStats()`.

## Automation Coverage

- `IdleProject.Character.CombatPower.Formula`
  - weighted CP value
  - zero stat CP
  - negative weighted total clamp
- `IdleProject.Character.CombatPower.Accessors`
  - `GetCombatPower()` equals `ComputeCombatPower(GetCurrentDerivedStats())`
- `IdleProject.Character.CombatPower.GrowthSources`
  - stat allocation increases CP
  - equipment affix/set bonuses increase CP
  - enhancement increases CP
  - rebirth bonus points increase CP against the original level-one baseline
  - transcend multiplier increases CP against equivalent post-reset stats

## Verification

- RED: `Build.bat IdleProjectEditor Win64 Development` failed on missing
  `CharacterSystem/CombatPowerFormula.h`.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.Character.CombatPower; Quit"` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject; Quit"` exited 0.
