# PR #52 Codex character implementation notes

## Scope

- Added `UIdleSaveGame` as the local `USaveGame` container for V1 progress:
  gold, level, exp, rebirth, transcend, stat allocation, chapter boss flag,
  last-seen timestamp, stage progress, tower floor, equipped pet, and pet
  levels.
- Added `UIdleGameInstance::SaveProgress`, `LoadProgress`, `CaptureToSave`,
  and `ApplyFromSave`. `Init` now loads after service initialization, and
  `Shutdown` saves before existing language/last-seen config persistence.
- Added restore APIs for `UStageService`, `UTowerService`, and `UPetService`.
- Added autosave calls for major local progression mutations. `AddExp`
  suppresses nested level-up autosaves and saves once after the exp mutation.
- Added `OnProgressSaved` and a localized HUD saved indicator (`저장됨` /
  `Saved`) that binds and unbinds with the HUD lifecycle.

## Automation Coverage

- `IdleProject.GameCore.SaveSystem.SaveGameDefaults`
- `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`
- `IdleProject.GameCore.SaveSystem.ProgressSavedBroadcast`
- `IdleProject.UI.HUD.SaveProgressFeedbackLabel`
- `IdleProject.Localization.CsvIntegrity`

## Verification

- RED: `Build.bat IdleProjectEditor Win64 Development` failed before
  implementation because `GameCore/IdleSaveGame.h` was missing.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe` with
  `Automation RunTests IdleProject.GameCore.SaveSystem; Quit` exited 0.
- GREEN: targeted save/HUD/localization Automation exited 0:
  `IdleProject.GameCore.SaveSystem.ProgressSavedBroadcast`,
  `IdleProject.UI.HUD.SaveProgressFeedbackLabel`,
  `IdleProject.Localization.CsvIntegrity`.
