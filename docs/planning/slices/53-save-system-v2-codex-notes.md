# PR #53 Codex character implementation notes

## Scope

- Upgraded `UIdleSaveGame` default `SaveVersion` to 2 while preserving all V1
  core, Stage, Tower, and Pet fields.
- Added V2 payloads for:
  - Inventory items and equipped slot indexes.
  - Skill ranks and unspent skill points.
  - Quest active-state entries and daily reset date.
  - Season id, tokens, and claimed tiers.
- Added capture and restore APIs to `UInventoryComponent`, `USkillComponent`,
  `UQuestService`, and `USeasonService`.
- Extended `UIdleGameInstance::CaptureToSave` and `ApplyFromSave` to include
  V2 systems. Version 1 saves still load with default Inventory/Skill/Quest/
  Season state instead of failing.
- Restore paths sanitize malformed payloads: invalid equipment indexes,
  invalid item rows, over-cap enhancement, unknown skill ids, negative skill
  points, unknown quest ids, over-target quest progress, mismatched season ids,
  and unknown season tiers.

## Automation Coverage

- `IdleProject.GameCore.SaveSystem.SaveGameDefaults`
- `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`
- `IdleProject.GameCore.SaveSystem.InventoryStateRoundTrip`
- `IdleProject.GameCore.SaveSystem.SkillRankStateRoundTrip`
- `IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip`
- `IdleProject.GameCore.SaveSystem.MalformedV2PayloadIsSanitized`
- Existing V1 regression tests remain:
  `InvalidLoadIsNoOp`, `RestoreSanitizesServiceState`,
  `ProgressSavedBroadcast`.

## Verification

- RED: `Build.bat IdleProjectEditor Win64 Development` failed after adding the
  new tests because V2 fields and capture/restore APIs did not exist.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: `UnrealEditor-Cmd.exe` with
  `Automation RunTests IdleProject.GameCore.SaveSystem; Quit` exited 0 and
  found 9 SaveSystem tests, all completed with `Result={Success}`.
- GREEN: full `Automation RunTests IdleProject; Quit` exited 0 and found 143
  automation tests.

## Additional QA Fix

- Full automation exposed an existing flaky test in
  `IdleProject.Inventory.ItemFactory.GuaranteedDropForLevel`: it compared one
  random low-level drop against one random high-level drop, which is invalid
  after rarity, affix, and set randomness. The assertion now verifies that both
  guaranteed drops are playable and uses `FDropFormula::ComputeItemBonus` for
  deterministic level-scaling coverage.

## Notes For TM

- Inventory and Skill capture from `UIdleGameInstance` depends on the current
  player pawn. If no pawn or component exists, the save safely skips those
  component payloads.
- Quest restore initializes default quests first, overlays known saved entries,
  and GameInstance then applies daily reset against the current UTC date.
- Season restore ignores payloads from a different `CurrentSeasonId`; this is
  the intended safe default for future season rotation.
