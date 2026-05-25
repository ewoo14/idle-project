# Balance Simulator V1

## Distribution

- Runs: 1000
- Target: Lv100 + boss clear
- p10: 6.074h
- median: 6.529h
- p90: 6.979h
- min/max: 5.855h / 7.41h
- status: inside-target (target 5-10h, acceptable 3-20h)

## Sensitivity

- EXP curve: keep the current level.ts curve; do not apply the 1.85-2.0 exponent increase yet.
- gold per hour: keep active gold tuning stable and watch enhancement spend in the next slice.
- offline efficiency: current 70-80% sampling keeps idle rewards below active progress.

## Formula Sources

- server/src/core/formulas/level.ts
- server/src/core/formulas/combat.ts
- server/src/core/formulas/stats.ts
- server/src/core/formulas/offline.ts

