# Consumables V1 Balance Note

Date: 2026-05-29

This note covers PR #73 Task 7 for consumable items and timed buffs. It records
the V1 buff values, duration, acquisition assumptions, shop price basis,
simulator interpretation, and inflation guardrails.

## Formula Summary

V1 adds six fixed consumable types. `FConsumableFormula` and
`server/src/core/formulas/consumable.ts` are the formula sources and use the
same enum order from `AttackTonic=0` through `WisdomBooster=5`.

All six consumables last 1800 seconds, or 30 minutes. Unknown types return
0 percent and 0 duration.

| Consumable | Effect lane | Value | Duration | Pressure |
| --- | --- | ---: | ---: | --- |
| AttackTonic | PhysAtk / MagicAtk | +30% | 1800s | combat |
| GuardTonic | HP / PhysDef / MagicDef | +30% | 1800s | survival |
| AllStatElixir | core stats | +20% | 1800s | combat |
| FortuneScroll | DropRateAdd | +30pp | 1800s | economy |
| GoldFeast | gold gain | +50% | 1800s | economy |
| WisdomBooster | EXP gain | +50% | 1800s | economy |

The values are intentionally simple and readable. Attack and guard tonics sit
at the same +30% headline value, while AllStatElixir is lower at +20% because it
touches every core stat lane. GoldFeast and WisdomBooster are stronger at +50%,
but they are limited to one economy lane each. FortuneScroll is a flat drop-rate
addition and must remain clamped by the existing drop path.

## Acquisition And Price Basis

V1 uses three acquisition hooks:

- Gold shop purchase through `UIdleGameInstance::TryBuyConsumable`.
- Dungeon or content rewards that can add stock through `UBuffService`.
- Quest rewards that can add stock through the same stock path.

The current shop implementation prices one consumable at the same cost as the
PR #38 gear-roll shop item:

```text
ConsumableCost = GetGearRollCost(GlobalStageIndex)
               = round(300 * computeRewardMultiplier(GlobalStageIndex))
```

Current anchors from the shop formula:

| Progress point | Global stage index | Cost |
| --- | ---: | ---: |
| baseline | 0 or 1 | 300 gold |
| early elite | 5 | 480 gold |
| chapter 1 boss | 10 | 705 gold |
| chapter 4 boss | 40 | 2055 gold |

This keeps consumables as a light discretionary gold sink rather than a premium
or mandatory progression system. The cost scales with the same stage multiplier
used by the gear shop, so the player sees one consistent shop economy instead
of a separate consumable price curve.

## Simulator Interpretation

`tools/balance-sim` now reports consumable pressure by importing
`server/src/core/formulas/consumable.ts`. The current sampled first-rebirth run
does not inject consumable ownership, stock limits, or usage cadence yet.

Seed `23`, 1000 runs, current output:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

This remains inside the 5-10h median target and the 3-20h acceptable band. The
unchanged median is expected because consumables are report-only pressure until
acquisition timing and expected uptime are modeled.

The generated report includes:

```text
## Consumable Timed Buff Pressure
AttackTonic   PhysAtk/MagicAtk       30%  1800s  combat   no
GuardTonic    Hp/PhysDef/MagicDef    30%  1800s  survival no
AllStatElixir Core stats             20%  1800s  combat   no
FortuneScroll DropRateAdd            30%  1800s  economy  no
GoldFeast     Gold                   50%  1800s  economy  no
WisdomBooster Exp                    50%  1800s  economy  no
```

## Stacking Rules

Stat buffs are temporary multipliers applied after the existing permanent and
account-wide progression layers. They do not change the coefficients of
transcend, tower, achievement, mastery, equipment, pet, rune, or trait systems.

Economy buffs each have one application lane:

- GoldFeast: gold gain path only.
- WisdomBooster: EXP gain path only.
- FortuneScroll: drop chance path only.

The implementation must keep each active buff single-application by type. Using
the same consumable again refreshes the end time instead of creating a second
stack. Different consumable types can run at the same time.

## Inflation Guardrails

- Do not tune `LevelCurveDB.csv` or first-rebirth EXP pacing from consumables
  until acquisition timing and expected uptime are modeled in `tools/balance-sim`.
- Keep AllStatElixir below AttackTonic and GuardTonic because it can affect
  multiple derived lanes at once.
- Keep GoldFeast and WisdomBooster in separate application paths so they cannot
  double count with mastery, rune, pet, or achievement economy bonuses.
- Keep FortuneScroll as drop-rate add pressure, not a reward multiplier.
- If consumables become a high-uptime system, model stock acquisition and
  expected uptime before increasing the V1 values.
- Re-run `cd server; npm run balance:sim` and review
  `tools/balance-sim/reports/balance-sim-report.md` after changing any
  consumable percentage, duration, shop price, or acquisition hook.
