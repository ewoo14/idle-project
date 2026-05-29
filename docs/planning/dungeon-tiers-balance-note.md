# PR #75 Dungeon Tiers Balance Note

<!-- markdownlint-disable MD013 MD022 MD031 MD032 MD040 MD041 -->

## Summary

PR #75 extends the PR #68 daily dungeons with infinite difficulty tiers. The
balance contract is intentionally simple:

```text
TIER_CP_FACTOR = 2.0
TierCpRequirement(type, tier) = MinimumCp(type) * 2^(tier - 1)
TierRewardMultiplier(tier) = max(1, tier)
Reward(type, cp, tier) = round(BaseReward * sqrt(max(1, cp / MinimumCp))) * tier
```

Tier 1 remains the PR #68 reward surface. Higher tiers do not change entry
count or dungeon type availability; they only ask for higher combat power and
pay a linear tier multiplier on top of the existing sublinear CP reward.

## Why CP Doubles While Rewards Grow Linearly

The tier gate doubles CP every tier so combat progression remains the primary
permission check. A player must reach 16x the minimum CP to enter tier 5:

- Gold: 100 CP to 1600 CP
- Exp: 250 CP to 4000 CP
- Essence: 500 CP to 8000 CP

Reward growth is linear by tier because the existing `sqrt(CP / MinimumCp)`
term already rewards CP. At the exact tier gate, tier 5 has
`sqrt(16) * 5 = 20x` the tier 1 run reward. This gives visible payoff for
choosing a harder dungeon without making every 2x CP jump become a 2x economy
jump. The economy pressure is therefore bounded by a linear tier label and a
sublinear CP curve rather than an exponential reward curve.

## Tier 1-5 Anchors

The table uses the exact CP requirement for each tier. Rewards are per run
before the three daily entries.

| Dungeon | Tier 1 | Tier 2 | Tier 3 | Tier 4 | Tier 5 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Gold CP | 100 | 200 | 400 | 800 | 1600 |
| Gold reward | 20,000 | 56,568 | 120,000 | 226,274 | 400,000 |
| Exp CP | 250 | 500 | 1000 | 2000 | 4000 |
| Exp reward | 20,000 | 56,568 | 120,000 | 226,274 | 400,000 |
| Essence CP | 500 | 1000 | 2000 | 4000 | 8000 |
| Essence reward | 12 | 34 | 72 | 136 | 240 |

Tier 5 daily totals at the exact tier gate:

| Dungeon | Tier | CP | 3-run daily reward | Median Lv50 income equivalent |
| --- | ---: | ---: | ---: | ---: |
| Gold | 5 | 1600 | 1,200,000 gold | 1.833h |
| Exp | 5 | 4000 | 1,200,000 exp | 1.643h |
| Essence | 5 | 8000 | 720 essence | n/a |

Gold and EXP tier 5 exceed the "below one Lv50 income hour" PR #68 review row
because tier 5 is no longer a Lv100 review-CP sample; it is an explicit 16x
minimum-CP challenge. The pressure is acceptable for PR #75 because the reward
requires higher CP, remains capped by three daily entries, and is not injected
into first-rebirth timing.

## First-Rebirth Timing Guard

`npm run balance:sim` was run after the tier pressure was added to the report.
The sampled 1000-run first-rebirth distribution stayed unchanged:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

The median remains inside the 5-10h target, and every sampled run remains
inside the 3-20h review band. Dungeon rewards are still report-only pressure in
`tools/balance-sim`; acquisition timing, unlock timing, and expected daily usage
are not injected into the first-rebirth model.

## Inflation And Stacking Guardrails

- Keep the daily entry limit shared at three entries per dungeon. Do not add a
  separate per-tier entry budget in this PR.
- Keep tier rewards single-application: base dungeon reward first, then tier
  multiplier, then source-specific bonuses such as PR #74 Abyss local mastery.
- #74 Abyss local mastery should apply once to dungeon reward output. It must
  not be applied before tier multiplication and again after tier multiplication.
- Do not tune the first-rebirth EXP curve from tier pressure until dungeon
  unlock timing and expected daily usage are explicitly modeled.
- Essence remains quantity pressure until rune-essence/hour and rune sink timing
  exist in the simulator.
- If future telemetry shows daily Gold/EXP dungeons replacing stage farming,
  prefer changing unlock timing, daily entry count, or tier availability before
  reducing the tier multiplier.

## Review Evidence

- Formula source: `server/src/core/formulas/dungeon.ts`
- Simulator source: `tools/balance-sim/index.ts`
- Generated report: `tools/balance-sim/reports/balance-sim-report.md`
- Verification command: `cd server; npm run balance:sim`
- Test command: `cd server; npm run test -- tests/balance-sim.test.ts`
