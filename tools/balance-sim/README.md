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

Each run samples class, active play share, equipment multiplier, quest EXP
variance, and offline efficiency. The summary reports p10, median, p90, and
min/max hours to reach Lv100 and clear the boss.

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

- p10: 6.074h
- median: 6.529h
- p90: 6.979h
- min/max: 5.853h / 7.184h

The median is inside the 5-10h target and the full distribution is inside the
3-20h acceptable range, so this slice keeps the current EXP curve unchanged.
The sensitivity recommendation is to monitor enhancement gold pressure in the
next balance slice before changing combat or EXP coefficients.
