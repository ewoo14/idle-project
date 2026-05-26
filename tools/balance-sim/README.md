# Balance Simulator V1

Runs a deterministic 1000-sample rebirth timing simulation for the first reset
target: Lv100 plus one boss clear.

## Usage

From `server/`:

```bash
npm run balance:sim
```

The command writes:

- `tools/balance-sim/reports/balance-sim-report.md`
- `tools/balance-sim/reports/balance-sim-report.json`

Only the markdown report is committed as PR evidence. The JSON report contains
all per-run samples and is ignored to avoid committing large generated output;
rerun `npm run balance:sim` whenever detailed samples are needed locally.

## Model

The simulator imports the production TypeScript formulas instead of duplicating
the rule source:

- `server/src/core/formulas/level.ts`
- `server/src/core/formulas/combat.ts`
- `server/src/core/formulas/stats.ts`
- `server/src/core/formulas/offline.ts`
- `server/src/core/formulas/reward.ts`
- `server/src/core/formulas/stage.ts`
- `server/src/core/formulas/enhance.ts`

Each run samples class, active play share, equipment multiplier, quest EXP
variance, and offline efficiency. The summary reports p10, median, p90, and
min/max hours to reach Lv100 and clear the boss.

PR #32 adds kill reward scaling to the model. Active kill EXP and gold now call
`computeKillExp` / `computeKillGold`, and the same global stage index ramp
drives monster HP and kill rewards. The Chapter 1 comparison table in the
generated report is the review artifact for reward-vs-HP scaling.

PR #33 adds enhancement spend pressure to the report. The simulator reads
`MAX_ENHANCE_LEVEL`, `getEnhanceCost`, and `getEnhanceSuccessRate` from the
shared formula source, then reports minimum all-success gold cost, expected
gold cost using `cost / successRate`, and expected hours at sampled median Lv50
gold/hour.

## V1 Interpretation

The V1 pass is a distribution check, not a curve rewrite. It validates the
current production formulas against the rebirth target in
`docs/planning/05-balance-philosophy.md`:

- target band: median first rebirth in 5-10h
- acceptable guardrail: all sampled first rebirths in 3-20h
- sampled variables: class, active/idle mix, early equipment power, quest EXP
  variance, and offline efficiency
- output policy: commit the concise `.md` report and regenerate the ignored
  `.json` detail report only when reviewer investigation needs raw samples

## Current Result

Seed `23`, 1000 runs:

- p10: 4.9h
- median: 5.324h
- p90: 5.758h
- min/max: 4.582h / 6.128h

The median is inside the 5-10h target and the full distribution is inside the
3-20h acceptable range, so this slice keeps the current EXP curve unchanged.
BossBonus remains 8x and the stage reward multiplier remains
`1 + globalStageIndex * 0.15`. The sensitivity recommendation is to monitor
enhancement gold pressure in the next balance slice before changing combat or
EXP coefficients.

Enhancement V1 pressure, using the current +0 to +5 formula:

- minimum all-success cost: 5,500 gold
- expected cost: 11,020.66 gold
- expected cost at median sampled Lv50 gold/hour: 0.017h

This makes enhancement a light early gold sink in the current model rather than
a blocker for the first rebirth target.
