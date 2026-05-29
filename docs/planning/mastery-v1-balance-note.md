# Mastery V1 Balance Note

Date: 2026-05-29

This note covers PR #72 Task 10 for Unified Mastery. It records the V1 XP curve,
global bonus coefficients, simulator interpretation, and inflation guardrails
for the six persistent mastery tracks.

## Formula Summary

Mastery uses six permanent tracks: Combat, Equipment, Abyss, Rune, Beast, and
Explore. Track XP survives rebirth and transcend resets.

```text
XpToNext(level) = floor(100 * 1.15^level)
WorldPower = sum(trackLevels)

CoreStatMultiplier = 1 + 0.02 * ln(1 + Combat + Equipment + Explore)
CritRateAdd        = 0.01 * ln(1 + Rune)
DropRateAdd        = 0.01 * ln(1 + Abyss)
GoldFindPct        = 0.02 * ln(1 + Beast)
ExpBoostPct        = 0.02 * ln(1 + Beast)
```

The coefficients intentionally use logarithmic growth. Mastery has no hard cap,
but late levels add less marginal power than early levels. This supports
infinite growth without letting a permanent layer replace rebirth, transcend,
tower, equipment depth, rune depth, or achievement progression.

## XP Curve Anchors

`Cumulative XP` is the total XP needed for one track to reach that level from
level 0. `All tracks XP` is the same target applied to all six tracks.

| Review point | Level | XP to next | Cumulative XP | All tracks XP | World Power |
| --- | ---: | ---: | ---: | ---: | ---: |
| Early | 5 | 201 | 672 | 4,032 | 30 |
| Mid | 30 | 6,621 | 43,461 | 260,766 | 180 |
| End | 100 | 117,431,345 | 782,874,918 | 4,697,249,508 | 600 |

The level-100 row is intentionally expensive. V1 mastery is a permanent
background ladder, not a first-rebirth target. The first-rebirth simulator still
uses the current character level curve and remains targeted at level 100 plus
boss clear in 5-10h median, with 3-20h as the review guardrail.

## Bonus Anchors

These rows assume every track is at the same review level. Core uses three
tracks, so the level-100 review has `Combat + Equipment + Explore = 300`.

| Review point | Track levels | Core stat x | Core stat gain | Crit add | Drop add | Gold gain | EXP gain |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Early | 5 each | 1.055 | +5.545% | +1.792pp | +1.792pp | +3.584% | +3.584% |
| Mid | 30 each | 1.090 | +9.022% | +3.434pp | +3.434pp | +6.868% | +6.868% |
| End | 100 each | 1.114 | +11.414% | +4.615pp | +4.615pp | +9.230% | +9.230% |

At the headline endgame review point, the core multiplier is:

```text
1 + 0.02 * ln(1 + 300) = 1.114142...
```

That is about +11.4% to the shared core-stat lane at 300 combined core-track
levels. The value is visible, but it is much smaller than a mature transcend,
tower, rune, or equipment-depth layer. This is the intended V1 pressure.

## Stacking Rules

Mastery enters the stat pipeline as a separate multiplier after existing
permanent and account-wide layers:

```text
Final core stats =
  composed base/equipment/rebirth/stat-allocation stats
  * TranscendMultiplier
  * TowerMultiplier
  * AchievementMultiplier
  * MasteryCoreMultiplier
```

Crit, drop, gold, and EXP mastery bonuses are separate additive lanes. They do
not modify the transcend, tower, or achievement coefficients themselves.

This prevents double counting. For example, a player with 100 Combat, 100
Equipment, and 100 Explore gets about x1.114 mastery core pressure. That value
is multiplied once into the final stat path; it is not also folded into
achievement points, tower milestones, or transcend count.

## Simulator Interpretation

The current `tools/balance-sim` first-rebirth model is unchanged by this note.
Seed `23`, 1000 runs, still reports:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

This remains inside the 5-10h median target and the 3-20h acceptable band.
Mastery is not injected into the sampled first-rebirth run until acquisition
timing is implemented in the simulator. The V1 review treats mastery as a
long-tail permanent pressure layer and checks coefficient safety directly from
the shared formula.

## Inflation Guardrails

- Keep the mastery XP curve independent from `LevelCurveDB.csv`; do not tune
  first-rebirth pacing with mastery XP.
- Keep logarithmic coefficients for V1. If the level-100 all-track row exceeds
  about +15% core pressure, review the coefficient before changing other
  systems.
- Do not add per-track local bonuses in PR #72. Track-specific local effects
  are a V1.5 scope item because they can directly alter enhancement, rune,
  dungeon, and quest economies.
- Re-run `cd server; npm run test -- mastery` after changing formula constants.
- Re-run `cd server; npm run balance:sim` and review
  `tools/balance-sim/reports/balance-sim-report.md` if mastery acquisition is
  ever injected into first-rebirth timing.

