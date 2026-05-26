# M6 Stage Progression HUD V1 QA Scenarios

## Scope

PR #31 adds the Chapter 1 stage loop and a Canvas HUD stage indicator. QA covers
stage progress text, boss stage emphasis, weak element display, and regression
checks for the existing combat, respawn, rebirth, quest, pet, and season HUD
panels.

## Scenario 1: Stage Progress Indicator Uses StageService State

Given `UStageService` reports Chapter 1 Stage 3 with `KillsThisStage = 7`,
`KillsToAdvance = 10`, and weak element `Ice`
When `AIdleHUD::DrawHUD` builds the stage indicator view model
Then the HUD exposes `Stage 1-3 7/10`, progress ratio `0.7`, and a localized
weak element label.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StageIndicatorViewModel`)

## Scenario 2: Boss Stage Receives Visual Emphasis

Given `UStageService` reports Stage 1-5 with `bBossStage = true`,
`KillsThisStage = 0`, `KillsToAdvance = 1`, and weak element `Fire`
When the stage indicator view model is built
Then the HUD exposes the localized boss badge, uses the gold state border, and
uses the danger token for the fire weakness label.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StageIndicatorViewModel`)

## Scenario 3: Stage Advancement Refreshes HUD Data

Given the player defeats the required number of monsters on a normal stage
When `UStageService::RecordKill(false)` advances to the next stage
Then `GetCurrentStageInfo()` returns the new stage, resets
`KillsThisStage` to zero, and the next `DrawHUD` call reads the updated info
through `UIdleGameInstance::GetStageService()`.

Automation: `client/Source/IdleProject/Tests/StageServiceTests.cpp`
(`IdleProject.GameCore.StageService.Progression`)

## Scenario 4: Boss Kill Does Not Cross-Wire Rebirth HUD

Given the player is on Stage 1-5
When a non-boss kill is recorded
Then stage progress does not advance and the rebirth boss gate remains locked.

Given the Stage 1-5 boss kill is recorded
When `MarkChapter1BossDefeated()` is invoked through the game instance
Then the stage service records the chapter boss clear and the existing rebirth
panel uses its own `CanRebirth` rules without changing unrelated HUD panels.

Automation: `client/Source/IdleProject/Tests/StageServiceTests.cpp`
(`IdleProject.GameCore.StageService.BossCompletion`,
`IdleProject.GameCore.IdleGameInstance.StageServiceHooks`)

## Edge Cases

- `KillsToAdvance = 0` displays as `current/0` and produces progress ratio
  `0.0` rather than division by zero.
- Kill progress above the target is displayed as reported and clamped to a full
  progress bar.
- `None` weak element still produces a stable localized label and muted color.
- Missing `UIdleGameInstance` or `UStageService` skips only the stage indicator.
- Stage indicator layout stays top-center and does not overlap bottom skill
  slots, right-side rebirth and quest panels, or left-side class, pet, and
  season panels at 1080p, 1440p, and 4K.

## Regression Checks

- Existing skill HUD, rebirth panel, quest log, pet panel, season pass panel,
  floating damage text, and status indicators still draw from their existing
  view models.
- Stage HUD copy exists in both `client/Content/Localization/Game/ko/UI.csv`
  and `client/Content/Localization/Game/en/UI.csv`.
- No new binary art assets are required for this slice. Existing `.gitattributes`
  LFS rules still cover future `.uasset`, `.png`, `.fbx`, audio, and video
  files.

## Evidence Required

- UE editor target `Build.bat` stdout.
- UE Automation stdout for `Automation RunTests IdleProject` or the specific
  stage and HUD tests listed above.
- Markdown lint stdout for this file and `docs/qa/scenarios/README.md`.
