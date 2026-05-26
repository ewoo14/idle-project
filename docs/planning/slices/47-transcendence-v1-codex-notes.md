# PR #47 Transcendence V1 Codex Character Notes

## Scope

- Added client-side `FTranscendFormula` with threshold 5 and `1 + count * 0.25` stat multiplier.
- Added `UIdleGameInstance` transcend state, gate, reset action, preview/current multiplier helpers, and `OnTranscend`.
- Applied transcend multiplier in `AIdleCharacter::RefreshDerivedStats()` after passive stat application and before combat initialization.
- Multiplied only `Hp`, `PhysAtk`, `MagicAtk`, `PhysDef`, and `MagicDef`; rate-style stats remain unchanged.

## Automation Coverage

- `IdleProject.GameCore.Transcend.Formula`
- `IdleProject.GameCore.Transcend.GateAndPreview`
- `IdleProject.GameCore.Transcend.ResetAndCount`
- `IdleProject.Character.Stats.TranscendMultiplier`

## Verification

- RED: `Build.bat IdleProjectEditor Win64 Development` failed before implementation on missing `GameCore/TranscendFormula.h` and missing `UIdleGameInstance::Transcend`.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.GameCore.Transcend; Automation RunTests IdleProject.Character.Stats.TranscendMultiplier; Quit"` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject; Quit"` exited 0; log recorded the new transcend tests as `Result={Success}`.
