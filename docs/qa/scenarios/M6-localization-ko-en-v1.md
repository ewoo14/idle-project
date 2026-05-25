# M6 Localization KO/EN V1 QA

## Scope

Localization V1 covers the client-side CSV string tables and HUD lookup
model for Korean and English.

## Automated Coverage

- CSV key parity: `IdleProject.Localization.CsvIntegrity`
  verifies `UI`, `Story`, `StoryText`, `Skill`, and `Quest` have matching
  `ko/en` key sets with no duplicate or empty translations.
- Culture switch lookup: `IdleProject.Localization.LookupAndCultureSwitch`
  verifies `HUD_GOLD_FORMAT` returns Korean text in `ko`, English text in
  `en`, and unsupported languages normalize to `ko`.
- HUD model regression: `IdleProject.UI.HUD.*` verifies existing offline
  reward, quest, rebirth, class, pet/season, and skill display view models
  keep passing after table lookup conversion.

## Manual Smoke

1. Start PIE in Korean and confirm HP/EXP/Gold/Level, quest, rebirth,
   class, pet, and season labels use Korean copy.
2. Call `UIdleGameInstance::SetLanguage("en")`, restart or rebuild the
   relevant HUD models, and confirm the same labels use English copy.
3. Confirm `Language` persists in `GGameUserSettingsIni` and defaults back
   to `ko` for unsupported values.

## Scenario Details

### Scenario 1: Korean HUD and story terminology

Given the game starts with the default `ko` language,
When the HUD, quest log, class panel, skill HUD, and Chapter 1 story
lookups are rendered,
Then user-facing UI labels use Korean copy, the Chapter 1 boss is named
`안개 군주`, and story text uses `균열` and `달그림자계` according to the
story bible.

### Scenario 2: English culture switch

Given `UIdleGameInstance::SetLanguage("en")` has been called,
When the HUD models and localization lookups are rebuilt,
Then UI, skill, quest, `Story`, and `StoryText` keys resolve to English
copy, `Lumina` remains the region name, and the Chapter 1 boss is rendered
as `Mist Lord`.

### Scenario 3: Unsupported language fallback

Given a user or settings file requests an unsupported language such as `ja`,
When `NormalizeLanguage()` and localized lookups run,
Then the language normalizes to `ko` and missing English-only assumptions
do not appear in the HUD or story tables.

### Scenario 4: Story table split

Given both `Story.csv` and `StoryText.csv` exist for `ko` and `en`,
When CSV integrity validation scans the story tables,
Then `StoryText.csv` contains dialogue/cutscene keys, `Story.csv`
contains chapter/map/boss/rebirth metadata keys, and no key appears in
both tables for the same language.

## Verification Evidence

- Build:
  `Build.bat IdleProjectEditor Win64 Development` with the client
  project path -> `Result: Succeeded`.
- Automation:
  `UnrealEditor-Cmd.exe` with `Automation RunTests IdleProject` and
  `-NullRHI` -> 47 `Success`, `TEST COMPLETE. EXIT CODE: 0`.
