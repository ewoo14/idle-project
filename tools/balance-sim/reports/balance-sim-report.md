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
| 1-1 | 0 | 1 | 1 | 12 | 8 | 96 | 64 |
| 1-2 | 1 | 1.15 | 1.15 | 14 | 9 | 110 | 74 |
| 1-3 | 2 | 1.3 | 1.3 | 16 | 10 | 125 | 83 |
| 1-4 | 3 | 1.45 | 1.45 | 17 | 12 | 139 | 93 |
| 1-5 | 4 | 1.6 | 1.6 | 19 | 13 | 154 | 102 |

<!-- markdownlint-enable MD013 -->

## Formula Sources

- server/src/core/formulas/level.ts
- server/src/core/formulas/combat.ts
- server/src/core/formulas/stats.ts
- server/src/core/formulas/offline.ts
- server/src/core/formulas/reward.ts
- server/src/core/formulas/stage.ts
