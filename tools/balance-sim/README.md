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
- `server/src/core/formulas/achievement.ts`

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

PR #39 extends that pressure check with rarity-scaled scenarios for Common,
Rare, Epic, and Legendary. PR #44 expands the detail table to +0 through +50.
PR #45 adds Mythic to the rarity summary and keeps eight-slot Legendary/Mythic
estimates for high-rarity long-tail sink pressure.

PR #42 adds pet feed pressure. The simulator imports `petLevel.ts`, reports the
Lv0 to Lv10 feed-cost path, and compares the dog gold-bonus payback against the
sampled median Lv50 gold/hour.

PR #55 adds achievement multiplier pressure. The simulator imports
`achievement.ts`, compares the legacy linear multiplier with the soft-capped
multiplier, and reports the composite impact against the PR #47 transcend and
PR #51 tower milestone reference multipliers.

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

Enhancement V1 pressure, using the current +0 to +50 formula:

- minimum all-success cost: 4,292,500 gold
- expected cost: 22,717,602.91 gold
- expected cost at median sampled Lv50 gold/hour: 34.7h
- Legendary expected cost: 363,481,639.40 gold, or 555.197h at the sampled median
  Lv50 gold/hour
- eight Legendary slots: 2,907,853,115.20 expected gold, or 4441.579h at the sampled
  median Lv50 gold/hour
- Mythic expected cost: 726,963,278.80 gold, or 1110.395h at the sampled median
  Lv50 gold/hour
- eight Mythic slots: 5,815,706,230.40 expected gold, or 8883.159h at the sampled
  median Lv50 gold/hour

This makes Common enhancement a long-tail gold sink in the current model while
making high-rarity enhancement an open-ended midgame/endgame pressure target.

Pet growth V1 pressure:

- total Lv0 to Lv10 feed cost: 192,500 gold
- dog gold bonus: 20% to 40% (+20 percentage points)
- bird drop bonus: 15% to 30% (+15 percentage points)
- dog payback at median sampled Lv50 gold/hour: 1.47h

This makes dog feeding a recoupable gold investment rather than a first-rebirth
progression wall. Bird feeding should be reviewed through loot-quality
telemetry because its drop bonus does not directly convert to gold/hour.
