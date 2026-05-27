# M8 Save System V1 QA Scenarios

## Scope

- Local `USaveGame` progress persistence through `UIdleGameInstance`.
- Autosave feedback in the C++ Canvas HUD.
- ko/en UI CSV integrity for the saved feedback copy.
- Restart, offline reward, and server-down behavior for local-only saves.

## Scenario 1: Manual save broadcasts HUD feedback

Given `UIdleGameInstance` has valid V1 progress state.

When `SaveProgress()` writes the `IdleSave` slot successfully.

Then `OnProgressSaved` broadcasts exactly once for that save call.

And `AIdleHUD::HandleProgressSaved()` prepares the localized saved indicator.

Automation: `IdleProject.GameCore.SaveSystem.ProgressSavedBroadcast`,
`IdleProject.UI.HUD.SaveProgressFeedbackLabel`

## Scenario 2: Autosave does not block progression

Given the player earns gold, levels up, enhances gear, climbs the tower, feeds a
pet, rebirths, or transcends.

When the mutation path requests autosave.

Then the current progress is captured into `UIdleSaveGame`.

And the HUD shows the transient saved indicator after a successful write.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
manual PIE smoke for event-driven paths.

## Scenario 3: Restart restores local progress

Given the player has non-default gold, level, EXP, stat allocation, rebirth,
transcend, stage, tower, and pet state.

When the game is closed and launched again.

Then `Init()` loads `IdleSave` after service initialization.

And the restored `UIdleGameInstance`, `UStageService`, `UTowerService`, and
`UPetService` values match the saved state.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`

## Scenario 4: No save starts with defaults

Given no local `IdleSave` slot exists.

When `LoadProgress()` runs during startup.

Then the game keeps default V1 values such as level 1 and `NextExp` 150.

And no restore call corrupts initialized services.

Automation: `IdleProject.GameCore.SaveSystem.SaveGameDefaults`

## Scenario 5: Offline reward updates persist

Given the player claims offline rewards after a valid elapsed time.

When gold, EXP, and `LastSeenUnixSec` are updated.

Then the shared autosave path stores the new progress.

And a restart does not offer the same offline reward window again.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
manual PIE smoke for claim modal.

## Scenario 6: Server down keeps local save usable

Given the backend is not running.

When guest registration, pet list, season state, or offline claim requests fail.

Then local progress save and load still work through `USaveGame`.

And the HUD saved indicator remains independent from network callbacks.

Automation: `IdleProject.GameCore.SaveSystem.ProgressSavedBroadcast`,
manual PIE smoke with backend stopped.

## Scenario 7: Saved copy is localized

Given ko/en UI CSV files include `SAVE_PROGRESS_SAVED`.

When `IdleProject.Localization.CsvIntegrity` runs.

Then both languages contain the same key.

And English renders `Saved` while Korean renders `저장됨`.

Automation: `IdleProject.Localization.CsvIntegrity`,
`IdleProject.UI.HUD.SaveProgressFeedbackLabel`

## Manual Viewport Checks

- 1080p: confirm the saved indicator appears below the gold label and does not
  cover stage, tower, or shop panels.
- 1440p: confirm the saved indicator remains right-aligned and readable.
- 4K: confirm the Canvas scale keeps the indicator compact and inside the safe
  viewport area.

## Scenario 8: Malformed or missing local save is graceful

Given a V1 save payload is empty, versionless, missing from disk, or contains
out-of-range values.

When `ApplyFromSave()` or `LoadProgress()` runs.

Then invalid payloads are rejected without mutating current progress.

And accepted malformed values are clamped before reaching Stage, Tower, and Pet
services.

Automation: `IdleProject.GameCore.SaveSystem.InvalidLoadIsNoOp`,
`IdleProject.GameCore.SaveSystem.RestoreSanitizesServiceState`

## Save V2 Backlog

- Inventory: equipped items, bag contents, enhancement levels, affixes, and set
  membership.
- Skills: unlocked skills, ranks, available skill points, and any persistent
  cooldown/ultimate state required after restart.
- Quest: active quest progress, claimed rewards, daily reset anchor, and
  chapter/story quest flags.
- Season: season tokens, reached tier, claimed tier rewards, and pass reset
  metadata.

## Verification Commands

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.GameCore.SaveSystem; Automation RunTests IdleProject.UI.HUD.SaveProgressFeedbackLabel; Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
