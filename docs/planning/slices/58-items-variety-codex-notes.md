# PR #58 Codex Notes - Character

## Change Summary

- `FItemInstance` includes `BaseItemId` plus expanded affix fields for
  physical defense, magic defense, HP, and critical damage.
- `ItemFactory` uses the rarity-specific base item catalog to generate stable
  `DisplayName` and `ItemId` values from deterministic `FRandomStream` input.
- Equipment sets now cover Warrior, Guardian, Arcane, Assassin, Hunter, Holy,
  and Berserker with mirrored 2-piece and 4-piece bonuses in client and server.
- The affix pool covers CritRate, AtkSpeed, MagicAtk, PhysDef, MagicDef, HP,
  and CritDmg while preserving the PR #40 rarity count rules.
- `InventoryComponent::ComputeEquipmentBonus`, `FItemPowerScore`, and server
  `equipment.ts` include the expanded affix fields.
- `UI.csv` includes localized base item, set, rarity, and affix text.

## Verification Points

- Common and None remain no-affix and no-set cases.
- Mythic rolls exactly three unique affixes from the expanded pool.
- Four-piece set bonuses include both the 2-piece and 4-piece tiers.
- Save round trips preserve `BaseItemId`, expanded set enum, and expanded
  affix fields.
- Client and server base item catalogs expose the same stat scale anchors.
- Generated item names localize through `UI.csv` in both `en` and `ko`.

## Verification Run

- `npm run build`
- `npm run lint`
- `npm test`
- `Build.bat IdleProjectEditor Win64 Development`
- UE Automation `Automation RunTests IdleProject`

## Codex TM Fix Follow-Up

- Server `rollBaseItem` now returns client-parity base item stat scales:
  `atkScale`, `defScale`, and `hpScale`.
- Client generated item names now use `UI.csv` localization keys for rarity
  plus base item names.
- Added regression coverage for base item scale parity and localized generated
  item names.
