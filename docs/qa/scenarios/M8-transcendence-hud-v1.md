# M8 Transcendence HUD V1 QA Scenarios

## Scenario 1: Transcendence Is Locked Below Five Rebirths

Given the player has `RebirthCount` 4.
When the HUD builds the transcendence panel.
Then the status is locked, the requirement shows `Rebirth 4 / 5`, and the
`TranscendAction` hit box is not added.

Automation: `IdleProject.UI.HUD.TranscendPanelViewModel`

## Scenario 2: Transcendence Is Ready At Five Rebirths

Given the player has `RebirthCount` 5.
When the HUD builds the transcendence panel.
Then the status is ready, the requirement shows `Rebirth 5 / 5`, the current
multiplier is visible, the next multiplier preview is visible, and the
`TranscendAction` hit box is enabled.

Automation: `IdleProject.UI.HUD.TranscendPanelViewModel`,
`IdleProject.GameCore.Transcend.GateAndPreview`

## Scenario 3: Clicking Transcend Resets Deep Prestige State

Given the player has reached the transcend threshold.
When the player clicks `TranscendAction`.
Then `Transcend()` succeeds, `TranscendCount` increments, `RebirthCount` and
`RebirthBonusPoints` reset to zero, level and stat allocations reset, and the
HUD refreshes derived stats through the player character.

Automation: `IdleProject.GameCore.Transcend.ResetAndCount`

## Scenario 4: Transcend Feedback Uses Localized Copy

Given the player successfully transcends.
When `OnTranscend` broadcasts.
Then the HUD shows localized feedback such as `Transcend! x1.25 applied` or
`초월! x1.25 적용` and removes the delegate during `EndPlay`.

Automation: manual PIE smoke plus `IdleProject.Localization.CsvIntegrity`

## Scenario 5: Localization Keys Stay In Sync

Given ko/en UI CSV files include the transcendence panel keys.
When `IdleProject.Localization.CsvIntegrity` runs.
Then the key sets match and no empty or duplicate rows are reported.

Automation: `IdleProject.Localization.CsvIntegrity`
