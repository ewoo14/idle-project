# PR #65 Designer Notes — Rarity UI

## Scope

Designer scope for PR #65 is limited to visible rarity language, color tokens,
and HUD usage guidance. Gameplay formulas, save migration, drop rates, and
server parity remain owned by character/backend/balance.

## Rarity Labels

| Enum | ko | en |
| --- | --- | --- |
| Common | 일반 | Common |
| Rare | 희귀 | Rare |
| Epic | 영웅 | Epic |
| Unique | 유니크 | Unique |
| Legendary | 전설 | Legendary |
| Transcendent | 초월 | Transcendent |
| Mythic | 신화 | Mythic |

`Uncommon` is not a display tier. Existing legacy Uncommon data migrates to
Rare, so no ko/en UI key should be added for it.

## Color Tokens

| Token | HEX / fallback | Usage |
| --- | --- | --- |
| `--rarity-common` | `#B0B5C0` | Common neutral text and low-emphasis gear |
| `--rarity-rare` | `#5B8BC0` | Rare blue |
| `--rarity-epic` | `#A05BC0` | Epic purple |
| `--rarity-unique` | `#5BC07A` | Unique green, reusing the former Uncommon visual lane |
| `--rarity-legendary` | `#F0A040` | Legendary orange |
| `--rarity-transcendent` | `#40D0E8` | Transcendent cyan between Legendary and Mythic |
| `--rarity-mythic` | `#FF7B00 -> #00B4FF` | Mythic gradient; UMG fallback uses start color plus blue edge accents |

The UE mirror is `client/Source/IdleProject/UI/UIThemeTokens.h`.
`Theme::RarityUnique`, `Theme::RarityTranscendent`, and
`Theme::RarityMythicStart/End` already match the token values above.

## HUD Guidance

Equipment, enhancement, shop result, rune inventory, rune codex, and class rune
HUD surfaces should render rarity through `RarityToLabel` and `RarityToColor`.
Do not hard-code display strings or colors in HUD drawing code.

For Mythic, flat text can use `RarityMythicStart`; bordered panels or codex
summary accents may add `RarityMythicEnd` as a secondary line. Unique remains a
single green token, and Transcendent remains a single cyan token to avoid
reading as another Mythic gradient.

The Korean item rarity label `초월` is valid only in item/rune context. Prestige
transcendence continues to use the existing `TRANSCEND_*` localization keys and
panel copy, so the two meanings stay separated by HUD location and namespace.

## Verification Notes

- ko/en `UI.csv` contains exactly the seven active `RARITY_*` keys plus
  `RARITY_NONE`.
- `docs/planning/ui-tokens.json` removes `uncommon` from active rarity tokens.
- `.gitattributes` already tracks UE and media assets with Git LFS; this PR does
  not add new binary art assets.
