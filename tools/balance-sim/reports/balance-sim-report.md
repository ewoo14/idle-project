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

<!-- markdownlint-disable MD013 -->

| Current | Next | Cost | Success | Expected attempts | Expected gold | Cumulative expected gold |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| +0 | +1 | 100 | 95% | 1.053 | 105.26 | 105.26 |
| +1 | +2 | 400 | 85% | 1.176 | 470.59 | 575.85 |
| +2 | +3 | 900 | 70% | 1.429 | 1285.71 | 1861.57 |
| +3 | +4 | 1600 | 55% | 1.818 | 2909.09 | 4770.66 |
| +4 | +5 | 2500 | 40% | 2.5 | 6250 | 11020.66 |

<!-- markdownlint-enable MD013 -->

## Formula Sources

- server/src/core/formulas/level.ts
- server/src/core/formulas/combat.ts
- server/src/core/formulas/stats.ts
- server/src/core/formulas/offline.ts
- server/src/core/formulas/reward.ts
- server/src/core/formulas/stage.ts
- server/src/core/formulas/enhance.ts
