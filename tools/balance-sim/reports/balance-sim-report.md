# Balance Simulator V1

## Distribution

- Runs: 1000
- Target: Lv100 + boss clear
- p10: 4.919h
- median: 5.328h
- p90: 5.751h
- min/max: 4.564h / 6.144h
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

## Item Drop Rarity Pressure

- Source: `server/src/core/formulas/drop.ts`.
- Level 1 total probability: 100%
- Level 100 total probability: 100%
- Unique and Transcendent are interpolation tiers: Unique stays below
  Epic drop pressure, and Transcendent stays below Legendary drop pressure.
- This table reports rarity pressure only; the sampled first-rebirth
  distribution still uses the existing equipmentMultiplier abstraction.

<!-- markdownlint-disable MD013 -->

| Rarity | Stat x | Lv1 chance | Lv100 chance | Affixes |
| --- | ---: | ---: | ---: | ---: |
| Common | 1 | 70% | 56.8% | 0 |
| Rare | 1.7 | 28% | 30% | 1 |
| Epic | 2.3 | 0% | 6% | 2 |
| Unique | 2.75 | 0% | 2.5% | 2 |
| Legendary | 3.2 | 0% | 1.5% | 2-3 |
| Transcendent | 3.85 | 0% | 0.7% | 2-3 |
| Mythic | 4.5 | 0% | 0.5% | 3 |

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
- Eight Mythic slots: 11,631,412,460.88 expected gold,
  17766.317h at sampled median Lv50 gold/hour.

<!-- markdownlint-disable MD013 -->

| Rarity | Multiplier | Minimum +0 to +50 gold | Expected +0 to +50 gold | Hours at median Lv50 gold/hour |
| --- | ---: | ---: | ---: | ---: |
| Common | 1 | 4292500 | 22717602.46 | 34.7h |
| Rare | 2 | 8585000 | 45435204.93 | 69.4h |
| Epic | 4 | 17170000 | 90870409.85 | 138.799h |
| Unique | 8 | 34340000 | 181740819.7 | 277.599h |
| Legendary | 16 | 68680000 | 363481639.4 | 555.197h |
| Transcendent | 32 | 137360000 | 726963278.8 | 1110.395h |
| Mythic | 64 | 274720000 | 1453926557.61 | 2220.79h |

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

## Rune Growth Pressure

- Rune slots: 6
- Core rune growth is intentionally uncapped; the table shows one rune's
  stat bonus and the resulting same-lane 6-slot multiplier.
- Combat rows apply six matching Mythic +50 core runes to the shared Lv100
  review loadout. CP and DPS are reported as pressure signals, not new
  tuning gates.
- Util rows show the first Mythic enhance level that reaches the per-rune
  cap. Six capped utility runes are possible and therefore create very
  large specialized economy multipliers.

<!-- markdownlint-disable MD013 -->

| Rarity | Enhance | Single rune bonus | 6-slot multiplier |
| --- | ---: | ---: | ---: |
| Common | +0 | 2% | x1.12 |
| Common | +10 | 6% | x1.36 |
| Common | +50 | 22% | x2.32 |
| Common | +100 | 42% | x3.52 |
| Epic | +0 | 8% | x1.48 |
| Epic | +10 | 22% | x2.32 |
| Epic | +50 | 78% | x5.68 |
| Epic | +100 | 148% | x9.88 |
| Legendary | +0 | 12% | x1.72 |
| Legendary | +10 | 32% | x2.92 |
| Legendary | +50 | 112% | x7.72 |
| Legendary | +100 | 212% | x13.72 |
| Mythic | +0 | 18% | x2.08 |
| Mythic | +10 | 48% | x3.88 |
| Mythic | +50 | 168% | x11.08 |
| Mythic | +100 | 318% | x20.08 |

| Rune set | Class | Lv | Base CP | Rune CP | CP x | Base DPS | Rune DPS | DPS x |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 6x Mythic +50 PhysAtk | Warrior | 100 | 5444 | 25604 | x4.703 | 4295 | 53071 | x12.356 |
| 6x Mythic +50 MagicAtk | Mage | 100 | 5519 | 26526 | x4.806 | 4787 | 58561 | x12.233 |

## Class Mastery Rune Pressure

- ClassMastery uses the seventh dedicated rune slot. The table applies one
  Mythic +50 class rune to each Lv100 review loadout and keeps the regular
  six core slots out of the comparison.
- Damage-role rows are checked against the PR #60 +/-15% DPS band after the
  class rune is applied. Two-stat Warrior, Cleric, and Paladin rows are
  treated as CP/survival compensation, not an extra DPS lane.

<!-- markdownlint-disable MD013 -->

| Class | Role | Mastery stats | Base CP | Rune CP | CP x | Base DPS | Rune DPS | DPS x |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Warrior | dps | PhysAtk, PhysDef | 5444 | 9732 | x1.788 | 4295 | 12425 | x2.893 |
| Mage | dps | MagicAtk | 5519 | 9020 | x1.634 | 4787 | 13749 | x2.872 |
| Archer | dps | PhysAtk | 5197 | 8184 | x1.575 | 4451 | 13081 | x2.939 |
| Thief | dps | PhysAtk | 5145 | 8097 | x1.574 | 4669 | 13686 | x2.931 |
| Cleric | healer | MagicAtk, Hp | 5566 | 9452 | x1.698 | 3378 | 9852 | x2.917 |
| Paladin | tank | PhysDef, Hp | 5805 | 7733 | x1.332 | 3644 | 3644 | x1 |
| Berserker | dps | PhysAtk | 5316 | 8716 | x1.64 | 4629 | 13378 | x2.89 |
| Summoner | dps | MagicAtk | 5681 | 9221 | x1.623 | 4249 | 12306 | x2.896 |

<!-- markdownlint-enable MD013 -->

## Rune Set Bonus Pressure

- Set bonuses apply only to the six regular rune slots; the ClassMastery
  slot remains outside set counting.
- Rows compare 2/4/6 same-set tiers on the shared Lv100 Warrior review
  loadout. Utility economy lanes are listed as pressure signals, while
  only combat-facing lanes change CP or DPS in this table.
- Rune set acquisition is not injected into the sampled first-rebirth run;
  the 1000-run median remains the PR #61 baseline guard.

<!-- markdownlint-disable MD013 -->

| Rune set | Count | Bonus | Bonus lanes | Base CP | Set CP | CP x | Base DPS | Set DPS | DPS x |
| --- | ---: | ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Offense | 2 | 5% | PhysAtk, MagicAtk | 5444 | 5632 | x1.035 | 4295 | 4537 | x1.056 |
| Offense | 4 | 12% | PhysAtk, MagicAtk | 5444 | 5896 | x1.083 | 4295 | 4876 | x1.135 |
| Offense | 6 | 25% | PhysAtk, MagicAtk | 5444 | 6386 | x1.173 | 4295 | 5505 | x1.282 |
| Bastion | 2 | 5% | PhysDef, MagicDef | 5444 | 5484 | x1.007 | 4295 | 4295 | x1 |
| Bastion | 4 | 12% | PhysDef, MagicDef | 5444 | 5538 | x1.017 | 4295 | 4295 | x1 |
| Bastion | 6 | 25% | PhysDef, MagicDef | 5444 | 5642 | x1.036 | 4295 | 4295 | x1 |
| Vitality | 2 | 5% | Hp, OfflineEff | 5444 | 5462 | x1.003 | 4295 | 4295 | x1 |
| Vitality | 4 | 12% | Hp, OfflineEff | 5444 | 5488 | x1.008 | 4295 | 4295 | x1 |
| Vitality | 6 | 25% | Hp, OfflineEff | 5444 | 5536 | x1.017 | 4295 | 4295 | x1 |
| Fortune | 2 | 5% | GoldFind, ExpBoost, CritDamage | 5444 | 5449 | x1.001 | 4295 | 4314 | x1.004 |
| Fortune | 4 | 12% | GoldFind, ExpBoost, CritDamage | 5444 | 5456 | x1.002 | 4295 | 4341 | x1.011 |
| Fortune | 6 | 25% | GoldFind, ExpBoost, CritDamage | 5444 | 5469 | x1.005 | 4295 | 4391 | x1.022 |

<!-- markdownlint-enable MD013 -->

| Util rune | Rarity | Cap enhance | Single rune value | 6-slot total | Effective multiplier |
| --- | --- | ---: | ---: | ---: | ---: |
| CritDamage | Mythic | +177 | 100% | 600% | x2 |
| GoldFind | Mythic | +377 | 200% | 1200% | x3 |
| ExpBoost | Mythic | +377 | 200% | 1200% | x3 |
| OfflineEff | Mythic | +127 | 50% | 300% | x1.5 |

<!-- markdownlint-enable MD013 -->

## Rune Codex Collection Pressure

- Total cells: 63
- Per-cell core bonus: +0.4%
- All-cell core bonus: +25.2%
- All row-completion bonuses: +41%
- Core category completion bonus: +5%
- Util category cap extension: +10%
- Full codex core bonus: +71.2%
- Base median first-rebirth time: 5.328h
- Projected full-codex median if applied directly to first rebirth DPS: 3.112h (-41.6%).
- Not injected into the sampled first-rebirth run. The 1000-run median
  remains the baseline guard until rune acquisition/drop rates are wired
  into the simulation model.

## Class Balance Snapshot

- Effective DPS uses the class attack route through `computeClassDamage`,
  review defense, attack speed, crit expectation, and active skill
  `damageCoeff / cooldown` pressure from `skills.ts`.
- DPS classes target +/-15% around each level's DPS median.
- Paladin and Cleric are role exceptions: tank/healer utility may sit below
  the DPS band while preserving survival/support compensation.

### Lv50

<!-- markdownlint-disable MD013 -->

| Class | Role | HP | Effective ATK | Effective DPS | DPS delta | CP |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Warrior | dps | 1884 | 1010 | 2000 | -1% | 2928 |
| Mage | dps | 1467 | 1054 | 2295 | 13% | 2980 |
| Archer | dps | 1501 | 893 | 1895 | -7% | 2804 |
| Thief | dps | 1467 | 882 | 2027 | 0% | 2787 |
| Cleric | healer | 1526 | 1016 | 1599 | 0% | 3005 |
| Paladin | tank | 2184 | 989 | 1687 | 0% | 3118 |
| Berserker | dps | 1658 | 1024 | 2143 | 6% | 2871 |
| Summoner | dps | 1550 | 1067 | 2030 | 0% | 3065 |

<!-- markdownlint-enable MD013 -->

### Lv100

<!-- markdownlint-disable MD013 -->

| Class | Role | HP | Effective ATK | Effective DPS | DPS delta | CP |
| --- | --- | ---: | ---: | ---: | ---: | ---: |
| Warrior | dps | 3684 | 2000 | 4295 | -7% | 5444 |
| Mage | dps | 2892 | 2084 | 4787 | 3% | 5519 |
| Archer | dps | 2951 | 1778 | 4451 | -4% | 5197 |
| Thief | dps | 2892 | 1757 | 4669 | 1% | 5145 |
| Cleric | healer | 3001 | 2013 | 3378 | 0% | 5566 |
| Paladin | tank | 4259 | 1959 | 3644 | 0% | 5805 |
| Berserker | dps | 3258 | 2024 | 4629 | 0% | 5316 |
| Summoner | dps | 3060 | 2107 | 4249 | -8% | 5681 |

<!-- markdownlint-enable MD013 -->

## Formula Sources

- server/src/core/formulas/level.ts
- server/src/core/formulas/combat.ts
- server/src/core/formulas/stats.ts
- server/src/core/formulas/offline.ts
- server/src/core/formulas/reward.ts
- server/src/core/formulas/stage.ts
- server/src/core/formulas/drop.ts
- server/src/core/formulas/enhance.ts
- server/src/core/formulas/petLevel.ts
- server/src/core/formulas/achievement.ts
- server/src/core/formulas/rune.ts
- server/src/core/formulas/classRune.ts
- server/src/core/formulas/runeSet.ts
- server/src/core/formulas/runeCodex.ts
