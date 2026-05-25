# M4 Quest System V1 QA Scenario

## Scope

Character-side quest logic for PR #18:

- Local active quest model mirrors `server/src/core/data/quests.ts` quest IDs, objectives, target counts, and rewards.
- Kill, offline-claim, and gear-enhance hooks can advance matching active quests.
- Completed quests can be claimed once, apply gold/EXP through `UIdleGameInstance`, and unlock the next main quest.
- Daily quests reset lazily when the UTC date changes.
- Network quest endpoints are best-effort; local progress remains playable if server identity is not available.

## Automated Coverage

Automation target: `IdleProject.GameCore.QuestService`

- `ProgressClaimUnlock`: first main quest starts active with three daily quests, kill progress completes `main_ch1_001`, claiming returns reward values, and `main_ch1_002` unlocks.
- `DailyReset`: daily progress/completed/claimed state resets when the reset date advances.

Automation target: `IdleProject.GameCore.IdleGameInstance.QuestRewardAndHooks`

- `RecordQuestProgress(EQuestObjective::KillMonster, 5)` completes the first main quest.
- `ClaimQuest("main_ch1_001")` applies reward gold and EXP through existing progression methods.
- `ClaimOfflineRewardsAt(...)` advances the `daily_claim_offline` quest.

## Manual PIE Smoke

1. Start PIE with the server offline.
2. Kill monsters until the first main quest completes.
3. Claim the quest through any temporary debug/UI entry point once available.
4. Confirm gameplay continues without a network error blocking local progress.

## Follow-Up

- Designer/UI slice should bind the quest log to `UIdleGameInstance::GetActiveQuestStates()`.
- Backend sync should provide a character ID before enabling `UApiClient::ReportQuestProgress` and `ClaimQuestReward` calls from runtime.
