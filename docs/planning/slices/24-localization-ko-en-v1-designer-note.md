# PR #24 Designer Implementation Note

Date: 2026-05-26

## Scope

- Added UTF-8 CSV string tables under `client/Content/Localization/Game/{ko,en}/` for `UI`, `Story`, `StoryText`, `Skill`, and `Quest`.
- Added `IdleProject::Localization` C++ lookup helper with `ko`/`en` culture normalization, CSV loading, Korean fallback, and Automation-only key integrity helpers.
- Replaced HUD/Slate user-facing labels in `IdleHUD` and `IdleHUDWidget` with localized UI table keys, including offline reward, quest log, rebirth, class selection, skill HUD, pet, season pass, and equipment summary labels.
- Wired `UIdleGameInstance::SetLanguage()` and `GetLanguage()` to Unreal culture switching and persisted `Language` in `GGameUserSettingsIni` next to `LastSeenUnixSec`.
- Skill display names and quest titles now resolve through `Skill.csv` and `Quest.csv`; story CSV parity is ready for story-side copy review.

## Verification

- Build: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex` -> `Result: Succeeded`.
- Automation: `UnrealEditor-Cmd.exe client/IdleProject.uproject -ExecCmds="Automation RunTests IdleProject; Quit" -unattended -nop4 -nosplash -NullRHI` -> 47 `Success`, `TEST COMPLETE. EXIT CODE: 0`.
