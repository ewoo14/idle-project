# M4 Quest System V1 QA Scenario

## Scope

Character-side quest logic for PR #18:

- Local active quest model mirrors `server/src/core/data/quests.ts` quest IDs, objectives, target counts, and rewards.
- Kill, offline-claim, and gear-enhance hooks can advance matching active quests.
- Completed quests can be claimed once, apply gold/EXP through `UIdleGameInstance`, and unlock the next main quest.
- Daily quests reset lazily when the UTC date changes.
- Network quest endpoints are best-effort; local progress remains playable if server identity is not available.

## Automated Coverage

### Given / When / Then

- Given server `questDefinitions` is the canonical quest table, When `QuestService.list()` exposes unlocked quests, Then quest ID, objective, target count, rewards, prerequisite, and chapter map fields match the canonical table.
- Given local `UQuestService` initializes default quests, When automation reads its definition snapshot, Then every quest definition matches `server/src/core/data/quests.ts` field-for-field for gameplay data.
- Given kill, offline reward, and gear enhance events occur, When objective progress is recorded, Then only matching active unclaimed quests advance and progress clamps at each target.
- Given a quest is completed or claimed, When duplicate progress or duplicate claim attempts occur, Then rewards are not granted twice and claimed quests do not gain additional progress.
- Given daily quests contain progress from a previous UTC date, When quest state is listed or reset, Then progress, completed, and claimed flags reset to zero/false with the new reset date.

Automation target: `IdleProject.GameCore.QuestService`

- `ProgressClaimUnlock`: first main quest starts active with three daily quests, kill progress completes `main_ch1_001`, claiming returns reward values, and `main_ch1_002` unlocks.
- `DefinitionParity`: local quest definitions match the server quest table for quest ID, type, objective, target count, reward gold, reward EXP, prerequisite, and chapter map.
- `DailyReset`: daily progress/completed/claimed state resets when the reset date advances.

Automation target: `IdleProject.GameCore.IdleGameInstance.QuestRewardAndHooks`

- `RecordQuestProgress(EQuestObjective::KillMonster, 5)` completes the first main quest.
- `ClaimQuest("main_ch1_001")` applies reward gold and EXP through existing progression methods.
- `ClaimOfflineRewardsAt(...)` advances the `daily_claim_offline` quest.
- `RecordGearEnhanced()` advances and completes the `daily_enhance_gear` quest after three hooks.

Vitest target: `server/src/modules/quest/quest.service.test.ts`

- Objective progress covers `kill_monster`, `claim_offline`, and `enhance` events across active quests.
- Claimed quests are returned unchanged and are not persisted again during duplicate progress.
- Listed quest metadata is compared against `server/src/core/data/quests.ts` instead of a copied fixture.

## Manual PIE Smoke

1. Start PIE with the server offline.
2. Kill monsters until the first main quest completes.
3. Claim the quest through any temporary debug/UI entry point once available.
4. Confirm gameplay continues without a network error blocking local progress.

## Follow-Up

- Designer/UI slice should bind the quest log to `UIdleGameInstance::GetActiveQuestStates()`.
- Backend sync should provide a character ID before enabling `UApiClient::ReportQuestProgress` and `ClaimQuestReward` calls from runtime.
