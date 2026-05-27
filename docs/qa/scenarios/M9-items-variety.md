# M9 Items Variety QA Scenario

## Given
- A character can receive generated equipment from monster drops or shop rolls.
- Existing Common/None behavior from PR #40/#43 must remain unchanged.

## When
- Generate repeated level 60 guaranteed drops with a fixed RNG stream.
- Equip 2-piece and 4-piece sets for Warrior, Guardian, Arcane, Assassin, Hunter, Holy, and Berserker.
- Restore a save payload containing `BaseItemId`, expanded set enum, and expanded affix fields.

## Then
- Weapon drops expose at least six base ids and deterministic `DisplayName` / `ItemId` values for the same RNG seed.
- Common equipment has no affixes and no set membership.
- Rare+ equipment can roll all seven set families.
- Mythic equipment rolls exactly three unique affixes from the expanded pool.
- Equipment bonus and PowerScore include PhysDef, MagicDef, Hp, and CritDmg affixes.
- Save restore preserves old item fields and the new base/set/affix fields.

## Automation
- Client: `IdleProject.Inventory.ItemFactory.BaseCatalogDeterministic`
- Client: `IdleProject.Inventory.DropFormula.ExpandedAffixesCoverage`
- Client: `IdleProject.GameCore.SaveSystem.ExpandedItemFieldsRoundTrip`
- Server: `drop.test.ts`, `setBonus.test.ts`, `equipment.test.ts`
