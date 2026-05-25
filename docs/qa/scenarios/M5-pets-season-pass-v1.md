# M5 Pets + Season Pass V1 QA Scenarios

## Scope

PR #22 adds two default-owned pets and a free season pass track. QA focuses on
pet equip bonus parity, season token progression, tier reward claims, edge
guards, and regression coverage for quest/offline/balance-adjacent flows.

## Scenario 1: Pet Equip Bonus Parity

Given the server PetDB mirror defines `dog` as `gold +20%` and `bird` as
`drop +15%`
When the client initializes the default pet service and equips each pet
Then the UE definitions expose the same pet ids, bonus types, and percentages,
and only one equipped pet bonus is active at a time.

Automation: `server/src/core/data/pets-season.contract.test.ts`,
`server/src/modules/pet/pet.service.test.ts`,
`client/Source/IdleProject/Tests/PetSeasonTests.cpp`
(`IdleProject.GameCore.PetService.DefinitionParity`,
`IdleProject.GameCore.PetService.EquipBonus`)

## Scenario 2: Season Token Progression and Claims

Given season 1 has a free track with 10 tiers and tier 1 requires 10 tokens
When a completed quest is claimed through `UIdleGameInstance`
Then the client grants +10 season tokens, reaches tier 1, and claiming tier 1
adds the configured gold reward once.

Automation: `server/src/core/data/pets-season.contract.test.ts`,
`server/src/modules/season/season.service.test.ts`,
`client/Source/IdleProject/Tests/PetSeasonTests.cpp`
(`IdleProject.GameCore.SeasonService.TierClaim`,
`IdleProject.GameCore.IdleGameInstance.PetSeasonHooks`)

## Scenario 3: Edge Guards

Given a player has the V1 default pet list and a season state
When the player tries to equip an unknown or unowned pet, add zero season
tokens, claim a locked tier, claim the same tier twice, or claim with a missing
character
Then the operation is rejected without changing the active pet, token balance,
claimed tier list, or reward totals.

Automation: `server/src/modules/pet/pet.service.test.ts`,
`server/src/modules/season/season.service.test.ts`,
`client/Source/IdleProject/Tests/PetSeasonTests.cpp`

## Scenario 4: Regression Coverage

Given previous milestones already cover quest completion, offline rewards,
inventory drops, and class parity
When pet and season services are initialized in the same game session
Then quest rewards still claim normally, pet gold/drop modifiers stay bounded,
offline reward formulas remain unchanged, and the season HUD reflects claimed,
claimable, and locked states without duplicating tier actions.

Automation: full `npm test`, UE Automation smoke for
`IdleProject.GameCore.*` and `IdleProject.UI.HUD.PetSeasonViewModels`.

## Manual Evidence

- Screenshot: pet panel with dog equipped and bird available.
- Screenshot: season panel at 10 tokens with tier 1 claimable.
- Log/stdout: quest claim grants +10 season tokens and tier 1 claim succeeds.
- Log/stdout: duplicate tier claim returns an already-claimed result.

## Edge Cases

- Offline progress must not auto-claim season tiers.
- Zero or negative token amounts must be ignored or rejected.
- Unknown pet ids must not fall back to the first default pet.
- Drop chance bonus must clamp to `1.0` after percentage multiplication.
- Season tier 10 remains the final V1 tier at 325 required tokens.

## Evidence Required

- Vitest stdout for `src/core/data/pets-season.contract.test.ts` or full
  `npm test`.
- UE Automation stdout for
  `IdleProject.GameCore.PetService.DefinitionParity` and
  `IdleProject.GameCore.SeasonService.DefinitionParity`.
- `npm run build`, `npm run lint`, UE `Build.bat`, and Automation stdout before
  PR handoff.
