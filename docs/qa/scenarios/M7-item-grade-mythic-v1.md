# M7 Item Grade Mythic v1 QA Scenarios

## Scope

- Mythic rarity drop boundaries, stat multiplier, enhancement cost, affix count,
  item set assignment, HUD color/localization, and Common-Legendary regression.
- Related tests: `IdleProject.Inventory.DropFormula.*`,
  `IdleProject.Inventory.Rarity.MythicHudMapping`,
  `IdleProject.Localization.CsvIntegrity`, and server `drop/enhance/setBonus`
  Vitest suites.

## Scenario 1: Mythic drop is unavailable at level 1

Given the drop formula rolls equipment rarity for monster level 1.
When the random roll traverses the full rarity table.
Then Mythic never appears.
And Common absorbs the residual probability after None, Uncommon, and Rare.

## Scenario 2: Mythic drop is available at level 100

Given the drop formula rolls equipment rarity for monster level 100.
When the random roll lands in the final 0.5% cumulative bucket.
Then the result is Mythic.
And rolls immediately below that bucket remain Legendary.

## Scenario 3: Mythic stats exceed Legendary without changing old tiers

Given generated weapons use level, variance, and rarity multiplier.
When a level 10 Mythic weapon is generated with variance 1.0.
Then attack is `45.0` from the `4.5` Mythic multiplier.
And Common, Uncommon, Rare, Epic, and Legendary multipliers remain
`1.0/1.3/1.7/2.3/3.2`.

## Scenario 4: Mythic enhancement cost uses the 32x rarity multiplier

Given a +0 Mythic item is equipped.
When the enhancement panel and server formula compute the next cost.
Then the displayed and computed cost is `3200`.
And a +0 Legendary item remains `1600`.
And max-level equipment still returns cost `0`.

## Scenario 5: Mythic rolls all affixes

Given a Mythic equipment item is generated.
When affixes are rolled with the injected random stream.
Then exactly three unique affix slots are populated.
And CritRate, AtkSpeed, and MagicAtk stay within the documented ranges.
And Common remains zero-affix.

## Scenario 6: Mythic can roll an item set

Given a Mythic equipment item is generated.
When the item set roll executes.
Then the item set is one of Warrior, Guardian, or Arcane.
And None/Common items still resolve to no set.

## Scenario 7: Mythic HUD and localization mapping

Given the HUD displays a Mythic item in English and Korean.
When rarity label and color are resolved.
Then English displays `Mythic`.
And Korean displays `신화`.
And the color uses `Theme::RarityMythicStart`.

## Regression Scenario: Existing rarity distribution remains anchored

Given the server and client rarity formulas are compared at boundary rolls.
When level 1 and level 100 thresholds are evaluated.
Then None, Common, Uncommon, Rare, Epic, and Legendary boundaries match the
pre-Mythic curve except for the intended Common residual reduction.
And server `Math.fround` arithmetic matches the client float boundary around
the Legendary-to-Mythic transition.
