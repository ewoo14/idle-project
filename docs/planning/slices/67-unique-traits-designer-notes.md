# PR #67 Designer Notes - Unique Trait HUD

## Scope

- Extend the existing equipment HUD instead of adding a new panel.
- Show `UniqueTrait1` and `UniqueTrait2` as trait name plus value.
- Keep ko/en UI localization keys paired and covered by `CsvIntegrity`.

## HUD Layout

- Weapon rows append trait summaries below affix summaries.
- Armor and accessory rows append compact `{Item}: {Trait} +{Value}` lines below
  the armor summary.
- Unique gear shows one trait; Transcendent gear shows two traits in slot order.
- Trait text reuses the current equipment rarity color and existing HUD text
  hierarchy, with no new hard-coded colors.
- The existing Canvas-height scale clamp remains 1.0-2.0 for 1080p, 1440p, and
  4K overlap safety.

## Localization

- English labels use `All-Stat Surge`, `Crit Damage Surge`, `Crit Rate Surge`,
  `Life Surge`, `Swift Surge`, `Physical Mastery`, `Magic Mastery`, and
  `Guard Mastery`.
- Korean labels use `전능`, `치명 피해`, `치명 확률`, `생명력`, `신속`,
  `물리 숙련`, `마법 숙련`, and `수호 숙련`.
- Percent traits use `UNIQUE_TRAIT_PERCENT_FORMAT`; flat attack-speed traits use
  `UNIQUE_TRAIT_FLAT_FORMAT`.

## Assets

No binary UI assets were added. Existing `.gitattributes` LFS patterns continue
to cover future exported `*.uasset` and `*.png` UI assets.
