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
- `server/src/core/formulas/dungeon.ts`
- `server/src/core/formulas/drop.ts`
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

PR #65 expands rarity reporting to seven active item tiers. The simulator
imports `drop.ts`, reports Lv1/Lv100 item drop rarity pressure, confirms Lv100
probability sums to 100%, and keeps Unique below Epic and Transcendent below
Legendary drop pressure. The same PR updates enhancement scenarios to
Common/Rare/Epic/Unique/Legendary/Transcendent/Mythic with Mythic at 64x.

PR #42 adds pet feed pressure. The simulator imports `petLevel.ts`, reports the
Lv0 to Lv10 feed-cost path, and compares the dog gold-bonus payback against the
sampled median Lv50 gold/hour.

PR #55 adds achievement multiplier pressure. The simulator imports
`achievement.ts`, compares the legacy linear multiplier with the soft-capped
multiplier, and reports the composite impact against the PR #47 transcend and
PR #51 tower milestone reference multipliers.

PR #66 expands reward-scaling evidence from the chapter 1 sample to all 30
stages across chapters 1-3. The report now includes encounter type, weak
element, normal reward, 3x elite reward, and 8x boss reward for every stage,
plus a Dark element pressure table that verifies the Holy/Dark 1.5x matchup.

PR #68 adds dungeon reward pressure. The simulator imports `dungeon.ts`, reports
the three-run daily reward for Gold, Exp, and Essence dungeons at minimum and
Lv100-review CP anchors, and compares Gold/EXP rows against sampled Lv50
income. Dungeon rewards are not injected into the sampled first-rebirth run.

PR #69 adds expanded pet bonus pressure. The simulator imports `petBonus.ts`,
reports the 10-pet catalog at Lv0 and Lv10, and compares stat-facing pet
bonuses against Lv100 CP/DPS review loadouts. Pet bonuses are not injected into
the sampled first-rebirth run until acquisition timing is modeled.

PR #74 adds mastery local bonus pressure. The simulator imports `mastery.ts`,
reports Combat, Equipment, Abyss, Rune, Beast, and Explore local bonus anchors
at levels 5, 30, and 100, and keeps local mastery out of the sampled
first-rebirth run until acquisition timing is modeled.

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
- median: 5.328h
- p90: 5.751h
- min/max: 4.564h / 6.144h

The median is inside the 5-10h target and the full distribution is inside the
3-20h acceptable range, so this slice keeps the current EXP curve unchanged.
BossBonus remains 8x and the stage reward multiplier remains
`1 + max(0, globalStageIndex - 1) * 0.15`. EliteBonus is 3x and is lower than
the boss reward spike. The sensitivity recommendation is to monitor enhancement
gold pressure in the next balance slice before changing combat or EXP
coefficients.

PR #66 stage evidence:

- stage table coverage: 30 rows, chapter 1-3, global indexes 1-30
- elite stages: 1-5, 2-5, 3-5 at 3x reward
- boss stages: 1-10, 2-10, 3-10 at 8x reward
- Dark weakness coverage: 9 of 30 stages, with chapter 3 carrying 5 Dark-weak
  stages
- Dark matchup pressure: Holy->Dark x1.5, Dark->Holy x1.5, Dark->Dark x1.5,
  Dark->Fire x1.0 neutral

Enhancement V1 pressure, using the current +0 to +50 formula:

- minimum all-success cost: 4,292,500 gold
- expected cost: 22,717,602.46 gold
- expected cost at median sampled Lv50 gold/hour: 34.7h
- Legendary expected cost: 363,481,639.40 gold, or 555.197h at the sampled median
  Lv50 gold/hour
- eight Legendary slots: 2,907,853,115.20 expected gold, or 4441.579h at the sampled
  median Lv50 gold/hour
- Mythic expected cost: 1,453,926,557.61 gold, or 2220.79h at the sampled median
  Lv50 gold/hour
- eight Mythic slots: 11,631,412,460.88 expected gold, or 17766.317h at the sampled
  median Lv50 gold/hour

Item drop rarity pressure at Lv100:

- total probability: 100%
- Common/Rare/Epic/Unique/Legendary/Transcendent/Mythic:
  56.8% / 30% / 6% / 2.5% / 1.5% / 0.7% / 0.5%
- Unique remains below Epic, and Transcendent remains below Legendary.

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

Pet expansion V1 pressure:

- catalog: dog, bird, cat, wolf, owl, bear, turtle, fox, rabbit, dragon
- Lv10 effective bonuses double Lv0 values through the shared pet-level formula
- Wolf/Owl stat pets add about x1.11 DPS on their matching Lv100 review class
- Bear/Turtle are CP/survival pressure and do not change DPS in the review row
- Dragon AllStat reports x1.067 CP and x1.09 DPS on the Lv100 Warrior row
- no pet bonus is injected into the sampled first-rebirth timing model

Dungeon V1 pressure:

- daily entry limit: 3 per dungeon
- reward curve: `round(BaseReward * sqrt(max(1, CP / MinimumCP)))`
- Gold CP 100: 20,000/run, 60,000/day, about 0.092h of sampled Lv50 gold income
- Gold CP 5,500: 148,324/run, 444,972/day, about 0.68h of sampled Lv50 gold income
- Exp CP 5,500: 93,808/run, 281,424/day, about 0.385h of sampled Lv50 EXP income
- Essence CP 5,500: 40/run, 120/day, reported as quantity until essence/hour exists
