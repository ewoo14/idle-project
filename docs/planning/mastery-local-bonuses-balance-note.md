# Mastery Local Bonuses Balance Note

Date: 2026-05-29

This note covers PR #74 Task 5 for Mastery local bonuses V1.5. It records the
six local mastery bonus lanes, formula anchors, simulator interpretation, and
inflation guardrails.

## Formula Summary

PR #72 made mastery a permanent account progression layer with shared XP,
world power, and global bonuses. PR #74 keeps that XP curve unchanged and adds
one local effect per track:

```text
LocalBonus(track, level) = 0.01 * ln(1 + level)
EquipmentLocalBonus     = min(0.50, 0.01 * ln(1 + level))
```

The TypeScript source is `server/src/core/formulas/mastery.ts::localBonus`.
The client mirror is `FMasteryFormula::GetLocalBonus`. Both use the same track
order: Combat, Equipment, Abyss, Rune, Beast, Explore.

| Track | Local lane | Application intent |
| --- | --- | --- |
| Combat | Kill reward | Gold and EXP from combat rewards |
| Equipment | Enhancement gold discount | Cost multiplier `1 - bonus`, capped at 50% |
| Abyss | Dungeon reward | Gold, EXP, and resource rewards from dungeon clears |
| Rune | Rune codex additive value | Adds to rune codex effect value |
| Beast | Pet bonus | Multiplies pet bonus output |
| Explore | Quest reward | Gold and EXP from quest claims |

## Coefficient Rationale

The coefficient is deliberately lower than PR #72 global mastery economy rows.
PR #72 uses `0.02 * ln(1 + Beast)` for gold and EXP gain; local bonuses use
`0.01 * ln(1 + level)` because they affect six separate systems and can stack
with global mastery, consumables, pets, runes, achievements, tower, and
transcend layers.

The logarithmic curve gives readable early progress while keeping long-tail
growth shallow. Level 100 is still only about +4.6% per local lane:

```text
0.01 * ln(101) = 0.046151... = 4.615%
```

Equipment is the only capped lane because it reduces a sink instead of adding
reward output. The 50% cap is unreachable in practical V1.5 review levels
because it would require `ln(1 + level) >= 50`, but the cap is still explicit
to prevent future infinite-growth cost elimination.

## Bonus Anchors

| Review point | Level | Combat | Equipment discount | Abyss | Rune | Beast | Explore |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Early | 5 | +1.792% | -1.792% cost | +1.792% | +1.792% | +1.792% | +1.792% |
| Mid | 30 | +3.434% | -3.434% cost | +3.434% | +3.434% | +3.434% | +3.434% |
| End | 100 | +4.615% | -4.615% cost | +4.615% | +4.615% | +4.615% | +4.615% |

The end row is intentionally modest. A player with level 100 in all six tracks
gets multiple visible local nudges, but no single system receives enough power
to replace its native tuning curve. This keeps mastery as a background ladder,
not a shortcut around equipment depth, dungeon limits, rune collection, pet
growth, or quest pacing.

## PR #72 And Consumable Stacking

PR #72 global mastery bonuses remain separate:

- Core stat pressure uses Combat + Equipment + Explore together.
- Crit, drop, gold, and EXP global mastery lanes are additive or multiplier
  lanes outside the local bonus table.
- Mastery XP remains separate from `LevelCurveDB.csv`.

PR #73 consumables are temporary 30-minute buffs. PR #74 local mastery bonuses
are permanent but smaller and must stay single-application by lane. Gold,
EXP, drop, pet, rune, and quest paths should apply one local mastery bonus at
the ownership point for that system, not inside a shared reward helper that can
be called twice.

## Simulator Interpretation

`tools/balance-sim` imports `localBonus` from
`server/src/core/formulas/mastery.ts` and reports local-bonus pressure as a
review table. Local mastery acquisition is not injected into the sampled
first-rebirth run yet.

Seed `23`, 1000 runs, current `npm run balance:sim` output:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

This remains inside the 5-10h median target and the 3-20h acceptable band.
The unchanged median is expected because PR #74 reports coefficient pressure
only until mastery acquisition timing and expected per-track levels are modeled
explicitly.

## Inflation Guardrails

- Keep `0.01` as the V1.5 local coefficient unless simulator acquisition data
  shows local bonuses are under-visible after realistic playtime.
- Do not tune `LevelCurveDB.csv`, `MonsterDB.csv`, or first-rebirth EXP pacing
  from local mastery until acquisition timing is modeled in `tools/balance-sim`.
- Keep Equipment as a discount value and preserve the 50% cap. If future
  levels make the cap reachable, tune enhancement protection sourcing and
  rarity costs before raising the cap.
- Keep each local bonus on one application path. Avoid applying Combat through
  both monster reward generation and a generic reward-claim helper.
- If future sampled median drops below 5h after mastery acquisition is wired,
  tune mastery XP income or unlock timing before reducing the local coefficient.
- Re-run `cd server; npm run test -- tests/balance-sim.test.ts` and
  `cd server; npm run balance:sim` after changing local-bonus constants or
  injecting mastery acquisition into the sampled run.
