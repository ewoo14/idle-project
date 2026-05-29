# M11 Dungeon Tiers V1 QA Scenarios

## Coverage

- Normal: exact CP boundary, tier reward scaling, selected tier run.
- Boundary: CP below gate, `minCp`, `4 * minCp`, tier 1 default compatibility.
- Regression: shared daily entries, client/server parity, Abyss mastery bonus single application.

## Scenario 1: Tier Gate Rejects One-Below CP

Given an Essence dungeon tier 3 requirement of 2000 CP.

When the player tries tier 3 at 1999 CP.

Then the run fails with zero reward.

And the Essence daily entry count remains 3.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `server/src/core/formulas/dungeon.parity.test.ts`

## Scenario 2: Exact Requirement Succeeds And Scales Reward

Given an Essence dungeon tier 3 requirement of 2000 CP.

When the player tries tier 3 at exactly 2000 CP.

Then the run succeeds.

And the Essence reward is `tier one reward * 3`.

And one shared Essence daily entry is consumed.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`

## Scenario 3: Tier 1 Preserves Legacy PR #68 Rewards

Given a caller requests EXP dungeon reward with the legacy two-argument formula.

When the caller requests the same CP with explicit tier 1.

Then both rewards are identical.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `server/src/core/formulas/dungeon.parity.test.ts`

## Scenario 4: Max Accessible Tier Boundaries Are Stable

Given each dungeon type has a minimum CP.

When CP is below minimum, equal to minimum, and equal to four times minimum.

Then max accessible tier is 0, 1, and 3 respectively.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`

## Scenario 5: Abyss Local Bonus Does Not Double Apply

Given the player has Abyss mastery level 1.

When the player clears Gold dungeon tier 3.

Then the tier 3 base reward is multiplied once by `1 + AbyssLocalBonus`.

Automation: `IdleProject.GameCore.Dungeon.AbyssTierBonus`
