# PR #75 Dungeon Tiers QA Notes

## Scope

- Dungeon tier CP gates and reward scaling for Gold, EXP, and Essence.
- Tier 1 legacy compatibility with the PR #68 two-argument reward call.
- Shared daily entries across tiers for the same dungeon type.
- Client/server formula parity for `GetTierCpRequirement`, `GetMaxAccessibleTier`, and tiered rewards.
- PR #74 Abyss mastery local bonus applied once after tier reward multiplication.

## Scenario 1: Tier CP Gate Blocks Inaccessible Runs

Given a dungeon type and a requested tier where `CombatPower < GetTierCpRequirement(type, tier)`.

When the player calls `TryRunDungeon(type, CombatPower, TodayUtc, tier)`.

Then the run fails.

And the reward is `0` for gold, exp, and essence.

And the dungeon daily entry count is not consumed.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `server/src/core/formulas/dungeon.parity.test.ts`

## Scenario 2: Tier Requirement Boundary Allows Exact CP

Given `CombatPower == GetTierCpRequirement(type, tier)`.

When the player runs that dungeon tier.

Then the run succeeds.

And exactly one shared daily entry is consumed.

And the reward equals the tier-one formula reward multiplied by `tierRewardMultiplier(tier)`.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `IdleProject.GameCore.Dungeon.Formula`, `server/src/core/formulas/dungeon.test.ts`

## Scenario 3: Tier 1 Keeps PR #68 Legacy Reward Compatibility

Given an existing caller uses `getDungeonReward(type, cp)` or `FDungeonFormula::GetRewardForCp(type, cp)`.

When the same reward is requested explicitly as tier 1.

Then both calls return identical rewards.

And tier 1 uses the same CP gate as the original minimum CP gate.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `IdleProject.GameCore.Dungeon.Formula`, `server/src/core/formulas/dungeon.parity.test.ts`

## Scenario 4: Max Accessible Tier Uses Boundary Rules

Given a dungeon type with `minCp`.

When CP is `minCp - 1`.

Then `GetMaxAccessibleTier(type, cp)` returns `0`.

When CP is `minCp`.

Then it returns `1`.

When CP is `4 * minCp`.

Then it returns `3`.

Automation: `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`, `server/src/core/formulas/dungeon.parity.test.ts`

## Scenario 5: Daily Entries Are Shared Across Tiers

Given a dungeon type has three daily entries.

When the player runs tier 1 and tier 3 of the same type on the same UTC date.

Then both runs consume from the same three-entry pool.

And failed tier-gate attempts do not consume entries.

Automation: `IdleProject.GameCore.Dungeon.ServiceRunLimitReset`, `IdleProject.GameCore.Dungeon.ServiceTierGateAndEntry`

## Scenario 6: Abyss Mastery Bonus Is Applied Once

Given the player has Abyss mastery local bonus from PR #74.

When the player clears an accessible tier 3 dungeon.

Then the base reward is first multiplied by the tier multiplier.

And the Abyss local bonus is applied once to that tiered reward.

And the local bonus is not applied again around the tier multiplier.

Automation: `IdleProject.GameCore.Dungeon.AbyssTierBonus`

## Scenario 7: Client And Server Formulas Stay In Parity

Given the server mirrors `FDungeonFormula`.

When type, CP, and tier boundary anchors are evaluated.

Then `getTierCpRequirement`, `getMaxAccessibleTier`, and `getDungeonReward(type, cp, tier)` match the C++ anchors using fround-compatible scaling.

Automation: `server/src/core/formulas/dungeon.parity.test.ts`
