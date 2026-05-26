# M6 Kill Reward Scaling V1 QA Scenario

## Scope

- Client reward formula parity with server `reward.ts` and `stage.ts`.
- Spawn-time `GlobalStageIndex` reward context.
- EXP, gold, boss bonus, pet gold bonus ordering, and equipment drop level.

## Scenario 1: Stage 1-1 Baseline Rewards

Given the player is fighting a normal monster spawned on chapter 1 stage 1
When the monster dies
Then the player receives 12 EXP
And the spawned gold drop amount is based on `10 + RandRange(0, 5)`
And equipment drops, when they occur, are rolled at monster level 1

## Scenario 2: Stage Reward Multiplier Parity

Given a monster was spawned with `GlobalStageIndex=1`
When reward formulas are evaluated for `baseGold=10`
Then client and server both resolve normal gold to 11
And boss gold with BossBonus 8 resolves to 92

## Scenario 3: Spawn-Time Stage Context

Given a monster is spawned on chapter 1 stage 4
And the player advances stage before killing that monster
When the original monster dies
Then EXP, gold, and equipment drop level use the stage 1-4 spawn index
And they do not use the later current stage index

## Scenario 4: Boss Reward Bonus

Given the chapter 1 stage 5 boss is spawned with `GlobalStageIndex=4`
When the boss dies
Then EXP resolves to 154 from `round(12 * 1.6 * 8)`
And gold resolves from the `10-15` base range to `128-192`
And pet gold bonus is applied after the boss-scaled base gold is computed

## Regression Checks

- Run UE Automation `IdleProject.GameCore.RewardFormula.*`.
- Run UE Automation `IdleProject.Combat.Monster.RewardStageContext`.
- Run server Vitest for `reward.test.ts`, `stage.test.ts`, and balance sim tests.
- Re-run `npm run balance:sim` and verify the report keeps median rebirth time
  in the 5-10h target band.
