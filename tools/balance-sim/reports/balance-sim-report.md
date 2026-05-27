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

- Max level: +50
- Minimum +0 to +50 gold cost: 4292500
- Expected +0 to +50 gold cost: 22717602.46
- Median sampled Lv50 gold/hour: 654689
- Expected +0 to +50 cost at median Lv50 gold/hour: 34.7h
- Failure consumes gold only; no downgrade or destruction is modeled in V1.
- V1 enhancement is a long-tail gold sink for infinite growth;
  high-level attempts are expected to outpace sampled Lv50 income and
  keep legendary multi-slot investment open-ended.

<!-- markdownlint-disable MD013 -->

| Current | Next | Cost | Success | Expected attempts | Expected gold | Cumulative expected gold |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| +0 | +1 | 100 | 95% | 1.053 | 105.26 | 105.26 |
| +1 | +2 | 400 | 93% | 1.073 | 429.18 | 534.45 |
| +2 | +3 | 900 | 91% | 1.094 | 984.68 | 1519.13 |
| +3 | +4 | 1600 | 90% | 1.116 | 1785.71 | 3304.84 |
| +4 | +5 | 2500 | 88% | 1.139 | 2847.38 | 6152.23 |
| +5 | +6 | 3600 | 86% | 1.163 | 4186.05 | 10338.27 |
| +6 | +7 | 4900 | 84% | 1.188 | 5819.48 | 16157.75 |
| +7 | +8 | 6400 | 82% | 1.214 | 7766.99 | 23924.74 |
| +8 | +9 | 8100 | 81% | 1.241 | 10049.63 | 33974.37 |
| +9 | +10 | 10000 | 79% | 1.269 | 12690.36 | 46664.72 |
| +10 | +11 | 12100 | 77% | 1.299 | 15714.29 | 62379.01 |
| +11 | +12 | 14400 | 75% | 1.33 | 19148.94 | 81527.95 |
| +12 | +13 | 16900 | 73% | 1.362 | 23024.52 | 104552.47 |
| +13 | +14 | 19600 | 72% | 1.397 | 27374.3 | 131926.77 |
| +14 | +15 | 22500 | 70% | 1.433 | 32234.96 | 164161.73 |
| +15 | +16 | 25600 | 68% | 1.471 | 37647.06 | 201808.78 |
| +16 | +17 | 28900 | 66% | 1.511 | 43655.59 | 245464.37 |
| +17 | +18 | 32400 | 64% | 1.553 | 50310.56 | 295774.93 |
| +18 | +19 | 36100 | 63% | 1.597 | 57667.73 | 353442.66 |
| +19 | +20 | 40000 | 61% | 1.645 | 65789.48 | 419232.14 |
| +20 | +21 | 44100 | 59% | 1.695 | 74745.77 | 493977.91 |
| +21 | +22 | 48400 | 57% | 1.748 | 84615.38 | 578593.29 |
| +22 | +23 | 52900 | 55% | 1.805 | 95487.36 | 674080.65 |
| +23 | +24 | 57600 | 54% | 1.866 | 107462.68 | 781543.33 |
| +24 | +25 | 62500 | 52% | 1.931 | 120656.37 | 902199.7 |
| +25 | +26 | 67600 | 50% | 2 | 135200 | 1037399.7 |
| +26 | +27 | 72900 | 48% | 2.075 | 151244.82 | 1188644.52 |
| +27 | +28 | 78400 | 46% | 2.155 | 168965.52 | 1357610.04 |
| +28 | +29 | 84100 | 45% | 2.242 | 188565.02 | 1546175.06 |
| +29 | +30 | 90000 | 43% | 2.336 | 210280.37 | 1756455.43 |
| +30 | +31 | 96100 | 41% | 2.439 | 234390.25 | 1990845.68 |
| +31 | +32 | 102400 | 39% | 2.551 | 261224.5 | 2252070.17 |
| +32 | +33 | 108900 | 37% | 2.674 | 291176.46 | 2543246.63 |
| +33 | +34 | 115600 | 36% | 2.809 | 324719.1 | 2867965.73 |
| +34 | +35 | 122500 | 34% | 2.959 | 362426.04 | 3230391.76 |
| +35 | +36 | 129600 | 32% | 3.125 | 405000.01 | 3635391.77 |
| +36 | +37 | 136900 | 30% | 3.311 | 453311.28 | 4088703.05 |
| +37 | +38 | 144400 | 28% | 3.521 | 508450.69 | 4597153.74 |
| +38 | +39 | 152100 | 27% | 3.759 | 571804.51 | 5168958.25 |
| +39 | +40 | 160000 | 25% | 4.032 | 645161.3 | 5814119.55 |
| +40 | +41 | 168100 | 23% | 4.348 | 730869.55 | 6544989.1 |
| +41 | +42 | 176400 | 21% | 4.717 | 832075.48 | 7377064.58 |
| +42 | +43 | 184900 | 19% | 5.155 | 953092.76 | 8330157.34 |
| +43 | +44 | 193600 | 18% | 5.682 | 1100000.01 | 9430157.34 |
| +44 | +45 | 202500 | 16% | 6.329 | 1281645.51 | 10711802.85 |
| +45 | +46 | 211600 | 14% | 7.143 | 1511428.56 | 12223231.42 |
| +46 | +47 | 220900 | 12% | 8.197 | 1810655.72 | 14033887.13 |
| +47 | +48 | 230400 | 10% | 9.615 | 2215384.57 | 16249271.7 |
| +48 | +49 | 240100 | 9% | 11.628 | 2791860.37 | 19041132.07 |
| +49 | +50 | 250000 | 7% | 14.706 | 3676470.39 | 22717602.46 |

<!-- markdownlint-enable MD013 -->

## Rarity Enhancement Pressure

- Rarity scales cost only; success rates, failure behavior, and stat payoff
  stay unchanged.
- Eight Legendary slots: 2,907,853,115.20 expected gold,
  4441.579h at sampled median Lv50 gold/hour.
- Eight Mythic slots: 5,815,706,230.40 expected gold,
  8883.159h at sampled median Lv50 gold/hour.

<!-- markdownlint-disable MD013 -->

| Rarity | Multiplier | Minimum +0 to +50 gold | Expected +0 to +50 gold | Hours at median Lv50 gold/hour |
| --- | ---: | ---: | ---: | ---: |
| Common | 1 | 4292500 | 22717602.46 | 34.7h |
| Rare | 4 | 17170000 | 90870409.85 | 138.799h |
| Epic | 8 | 34340000 | 181740819.7 | 277.599h |
| Legendary | 16 | 68680000 | 363481639.4 | 555.197h |
| Mythic | 32 | 137360000 | 726963278.8 | 1110.395h |

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

## Achievement Multiplier Pressure

- Soft cap starts at 100 points.
- Soft-cap bonus budget: 50 effective points.
- 100 points stays at x2 before the soft-cap slope decays toward x2.5.
- Composite reference: transcend 10 (x3.5)
  and tower floor 100 (x1.2).
- Achievement tiers remain infinite for collection depth, but stat growth
  is bounded enough that transcend and tower stay the primary prestige
  multipliers.

<!-- markdownlint-disable MD013 -->

| Points | Legacy x | Soft-capped x | Composite x |
| ---: | ---: | ---: | ---: |
| 0 | 1 | 1 | 4.2 |
| 3 | 1.03 | 1.03 | 4.326 |
| 100 | 2 | 2 | 8.4 |
| 125 | 2.25 | 2.197 | 9.226 |
| 250 | 3.5 | 2.475 | 10.395 |
| 500 | 6 | 2.5 | 10.499 |

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
- server/src/core/formulas/achievement.ts
