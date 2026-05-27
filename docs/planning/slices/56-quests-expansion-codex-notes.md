# PR #56 Codex Notes - Character + Backend

## Scope

- Expanded client quest definitions from chapter 1 daily-only V1 coverage to
  chapter 1 continuation, chapter 2 main chain, daily variety, and weekly
  quests.
- Added weekly reset state alongside the existing UTC daily reset state.
- Added client objective hooks for boss defeats, rebirth, transcendence, tower
  climbs, level reach, gold spend, gear shop rolls, and pet feeding.
- Updated ko/en quest localization and the HUD quest type label for weekly
  quests.

## Automation

- `IdleProject.GameCore.QuestService.DefinitionParity` checks the canonical
  client quest definition metadata and prerequisite chain.
- `IdleProject.GameCore.QuestService.ExpandedUnlockWeeklyReset` covers weekly
  active state and week-boundary reset.
- `IdleProject.GameCore.QuestService.ChapterTwoPrerequisiteChain` verifies that
  chapter 2 stays locked until the chapter 1 finale chain is claimed.
- `IdleProject.GameCore.QuestService.WeeklySaveRoundTrip` covers weekly quest
  progress and reset-id capture/restore.
- `IdleProject.GameCore.IdleGameInstance.ExpandedQuestHooks` covers boss,
  level, shop roll, and pet feed progress hooks through `UIdleGameInstance`.

## Notes

- Weekly reset ids use `YYYY-WNN` from UTC week calculation. Existing v2 saves
  without `QuestWeeklyResetId` restore with the week derived from the saved
  daily reset date.

## Backend Scope

- Mirrored the client quest definition table in
  `server/src/core/data/quests.ts`: 12 main quests, 7 daily quests, and 4
  weekly quests.
- Expanded server quest type and objective enums to include `weekly` plus
  boss, rebirth, transcend, tower, level, gold spend, gear shop roll, and pet
  feed objective ids.
- Added lazy weekly reset support with `weekly_reset_id` persistence and
  rollback migration `0007_quest_weekly_reset`.
- Updated quest API docs to expose weekly reset semantics and the expanded
  objective ids.

## Backend Verification

- `server/src/modules/quest/quest.service.test.ts` covers server definition
  parity, schema enum expansion, daily and weekly reset windows, and expanded
  objective progress for weekly quests.
