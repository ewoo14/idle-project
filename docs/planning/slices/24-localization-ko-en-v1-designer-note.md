# PR #24 Designer Implementation Note

Date: 2026-05-26

## Scope

- Added UTF-8 CSV string tables under
  `client/Content/Localization/Game/{ko,en}/` for `UI`, `Story`,
  `StoryText`, `Skill`, and `Quest`.
- Added `IdleProject::Localization` C++ lookup helper with `ko`/`en`
  culture normalization, CSV loading, Korean fallback, and
  Automation-only key integrity helpers.
- Replaced HUD/Slate user-facing labels in `IdleHUD` and `IdleHUDWidget`
  with localized UI table keys, including offline reward, quest log,
  rebirth, class selection, skill HUD, pet, season pass, and equipment
  summary labels.
- Wired `UIdleGameInstance::SetLanguage()` and `GetLanguage()` to Unreal
  culture switching and persisted `Language` in `GGameUserSettingsIni`
  next to `LastSeenUnixSec`.
- Skill display names and quest titles now resolve through `Skill.csv`
  and `Quest.csv`.
- `StoryText.csv` carries only dialogue/cutscene text while `Story.csv`
  carries chapter, map, boss, rebirth, class-branch, and rating-guide
  metadata. The two story tables keep separate keys with ko/en parity.

## Verification

- Build:
  `Build.bat IdleProjectEditor Win64 Development` with the client
  project path -> `Result: Succeeded`.
- Automation:
  `UnrealEditor-Cmd.exe` with `Automation RunTests IdleProject` and
  `-NullRHI` -> 47 `Success`, `TEST COMPLETE. EXIT CODE: 0`.
- Story copy pass: ko/en CSV scan confirms no duplicate keys between
  `Story.csv` and `StoryText.csv`, no empty translations, no stale
  boss/rift/realm terminology, and no raw English selection labels in
  Korean UI.
