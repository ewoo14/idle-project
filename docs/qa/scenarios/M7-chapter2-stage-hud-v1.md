# M7 Chapter 2 Stage HUD V1 QA Scenarios

## Scope

PR #37 extends the Canvas stage HUD from Chapter 1 into Chapter 2. QA covers
chapter/stage copy, chapter transition feedback, localization key parity, and
regression checks for adjacent HUD panels.

## Scenario 1: Chapter 2 Stage Progress Is Readable

Given `UStageService` reports Chapter 2 Stage 3 with `KillsThisStage = 6`,
`KillsToAdvance = 12`, and weak element `Ice`
When `AIdleHUD::DrawHUD` builds the stage indicator view model
Then the HUD exposes `Chapter 2` separately from `Stage 2-3 6/12`, keeps the
progress ratio at `0.5`, and renders the localized weak element label.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StageChapterFeedbackViewModel`)

## Scenario 2: Chapter Entry Feedback Appears On Transition

Given the player clears the Chapter 1 boss at Stage 1-5
When `UStageService::OnStageChanged` broadcasts Chapter 2 Stage 1 with zero
stage progress
Then `AIdleHUD` stores localized chapter entry feedback such as
`Chapter 2 Enter`, draws it below the stage indicator, and fades it out without
moving the top-center stage panel.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StageChapterFeedbackViewModel`)

## Scenario 3: Chapter Clear Feedback Uses Boss Delegate

Given the player clears a chapter boss
When `UStageService::OnChapterBossDefeated` broadcasts the cleared chapter
Then `AIdleHUD` stores localized clear feedback such as `Chapter 2 Cleared`.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StageChapterFeedbackViewModel`)

## Scenario 4: Localization And Weak Elements Stay In Sync

Given Chapter 2 contains lightning, ice, fire, and holy weak element stages
When `IdleProject.Localization.CsvIntegrity` runs
Then Korean and English UI CSV files contain matching keys for chapter labels,
transition feedback, and `ELEMENT_LIGHTNING`.

Automation: `client/Source/IdleProject/Tests/LocalizationTests.cpp`
(`IdleProject.Localization.CsvIntegrity`)

## Scenario 5: Rebirth Reset Returns To Chapter 1

Given a character has reached Chapter 2 and then performs rebirth
When the rebirth path initializes stage progress
Then `UStageService` returns to Chapter 1 Stage 1 with zero progress, and the
stage HUD next reads `Chapter 1` and `Stage 1-1 0/5`.

Automation: `client/Source/IdleProject/Tests/StageServiceTests.cpp`
(`IdleProject.GameCore.StageService.BossCompletion`,
`IdleProject.GameCore.IdleGameInstance.StageServiceHooks`)

## Layout Regression Checks

- The stage panel remains top-center at 1080p, 1440p, and 4K.
- Chapter feedback is drawn below the stage panel and does not overlap the
  bottom skill HUD, right-side rebirth and quest panels, or left-side class,
  pet, and season panels.
- No new art assets are required. `.gitattributes` already tracks future
  `.uasset`, image, mesh, audio, and video files with Git LFS.
