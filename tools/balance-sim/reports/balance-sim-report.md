# Balance Simulator V1

## Distribution

- Runs: 1000
- Target: Lv100 + boss clear
- p10: 4.9h
- median: 5.324h
- p90: 5.758h
- min/max: 4.582h / 6.128h
- status: inside-target (target 5-10h, acceptable 3-20h)

## Sensitivity

- EXP curve: keep current level.ts curve; do not raise exponent yet.
- gold per hour: keep active tuning stable; watch enhancement spend.
- offline efficiency: 70-80% keeps idle below active progress.

## Reward Scaling

- Boss bonus: 8x
- Normal kill rewards use `computeKillExp` / `computeKillGold`.
- Source: `server/src/core/formulas/reward.ts`.
- Monster HP and reward multipliers both reuse the stage index ramp.
- Result: 1-1 through 1-5 keep reward-per-HP pressure stable before boss bonuses.

<!-- markdownlint-disable MD013 -->

| Stage | idx | HP x | Reward x | Normal EXP | Normal Gold | Boss EXP | Boss Gold |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1-1 | 0 | 1 | 1 | 12 | 10-15 | 96 | 80-120 |
| 1-2 | 1 | 1.15 | 1.15 | 14 | 11-17 | 110 | 92-138 |
| 1-3 | 2 | 1.3 | 1.3 | 16 | 13-19 | 125 | 104-156 |
| 1-4 | 3 | 1.45 | 1.45 | 17 | 15-22 | 139 | 116-174 |
| 1-5 | 4 | 1.6 | 1.6 | 19 | 16-24 | 154 | 128-192 |

<!-- markdownlint-enable MD013 -->

## Enhancement Spend Pressure

- Max level: +5
- Minimum +0 to +5 gold cost: 5500
- Expected +0 to +5 gold cost: 11020.66
- Median sampled Lv50 gold/hour: 654689
- Expected +0 to +5 cost at median Lv50 gold/hour: 0.017h
- Failure consumes gold only; no downgrade or destruction is modeled in V1.
- V1 enhancement is a light early sink, not a Lv50 progression blocker;
  use this report as a pressure baseline before raising costs or adding
  material sinks.

<!-- markdownlint-disable MD013 -->

| Current | Next | Cost | Success | Expected attempts | Expected gold | Cumulative expected gold |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| +0 | +1 | 100 | 95% | 1.053 | 105.26 | 105.26 |
| +1 | +2 | 400 | 85% | 1.176 | 470.59 | 575.85 |
| +2 | +3 | 900 | 70% | 1.429 | 1285.71 | 1861.57 |
| +3 | +4 | 1600 | 55% | 1.818 | 2909.09 | 4770.66 |
| +4 | +5 | 2500 | 40% | 2.5 | 6250 | 11020.66 |

<!-- markdownlint-enable MD013 -->

## Rarity Enhancement Pressure

- Rarity scales cost only; success rates, failure behavior, and stat payoff
  stay unchanged.
- Eight Legendary slots: 1,410,644.08 expected gold,
  2.155h at sampled median Lv50 gold/hour.

<!-- markdownlint-disable MD013 -->

| Rarity | Multiplier | Minimum +0 to +5 gold | Expected +0 to +5 gold | Hours at median Lv50 gold/hour |
| --- | ---: | ---: | ---: | ---: |
| Common | 1 | 5500 | 11020.66 | 0.017h |
| Rare | 4 | 22000 | 44082.63 | 0.067h |
| Epic | 8 | 44000 | 88165.25 | 0.135h |
| Legendary | 16 | 88000 | 176330.51 | 0.269h |

<!-- markdownlint-enable MD013 -->

## Pet Feed Gold Pressure

- Max pet level: 10
- Total Lv0 to Lv10 feed cost: 192500
- Dog gold bonus: 20% to 40% (+20 percentage points).
- Bird drop bonus: 15% to 30% (+15 percentage points).
- Median sampled Lv50 gold/hour: 654689
- Incremental dog gold at median: 130937.8/h.
- Dog payback at median Lv50 gold/hour: 1.47h.
- This treats the Lv10 dog upgrade as an income investment against the
  existing PR #32 sampled blended gold rate. It does not grant combat power,
  so the sink competes with enhancement/shop gold without shortening
  first-rebirth EXP pacing directly.

<!-- markdownlint-disable MD013 -->

| Current | Next | Feed cost | Cumulative cost | Dog gold after feed | Bird drop after feed |
| ---: | ---: | ---: | ---: | ---: | ---: |
| Lv0 | Lv1 | 500 | 500 | 22% | 16.5% |
| Lv1 | Lv2 | 2000 | 2500 | 24% | 18% |
| Lv2 | Lv3 | 4500 | 7000 | 26% | 19.5% |
| Lv3 | Lv4 | 8000 | 15000 | 28% | 21% |
| Lv4 | Lv5 | 12500 | 27500 | 30% | 22.5% |
| Lv5 | Lv6 | 18000 | 45500 | 32% | 24% |
| Lv6 | Lv7 | 24500 | 70000 | 34% | 25.5% |
| Lv7 | Lv8 | 32000 | 102000 | 36% | 27% |
| Lv8 | Lv9 | 40500 | 142500 | 38% | 28.5% |
| Lv9 | Lv10 | 50000 | 192500 | 40% | 30% |

<!-- markdownlint-enable MD013 -->

## Formula Sources

- server/src/core/formulas/level.ts
- server/src/core/formulas/combat.ts
- server/src/core/formulas/stats.ts
- server/src/core/formulas/offline.ts
- server/src/core/formulas/reward.ts
- server/src/core/formulas/stage.ts
- server/src/core/formulas/enhance.ts
- server/src/core/formulas/petLevel.ts
