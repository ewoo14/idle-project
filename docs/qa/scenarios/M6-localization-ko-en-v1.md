# M6 Localization KO/EN V1 QA

## Scope

Localization V1 covers the client-side CSV string tables and HUD lookup model for Korean and English.

## Automated Coverage

| Scenario | Automation target | Expected |
| --- | --- | --- |
| CSV key parity | `IdleProject.Localization.CsvIntegrity` | `UI`, `Story`, `Skill`, and `Quest` have matching `ko/en` key sets with no duplicate or empty translations. |
| Culture switch lookup | `IdleProject.Localization.LookupAndCultureSwitch` | `HUD_GOLD_FORMAT` returns Korean text in `ko`, English text in `en`, and unsupported languages normalize to `ko`. |
| HUD model regression | `IdleProject.UI.HUD.*` | Existing offline reward, quest, rebirth, class, pet/season, and skill display view models keep passing after table lookup conversion. |

## Manual Smoke

1. Start PIE in Korean and confirm HP/EXP/Gold/Level, quest, rebirth, class, pet, and season labels use Korean copy.
2. Call `UIdleGameInstance::SetLanguage("en")`, restart or rebuild the relevant HUD models, and confirm the same labels use English copy.
3. Confirm `Language` persists in `GGameUserSettingsIni` and defaults back to `ko` for unsupported values.

## Verification Evidence

- Build: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex` -> `Result: Succeeded`.
- Automation: `UnrealEditor-Cmd.exe client/IdleProject.uproject -ExecCmds="Automation RunTests IdleProject; Quit" -unattended -nop4 -nosplash -NullRHI` -> 47 `Success`, `TEST COMPLETE. EXIT CODE: 0`.
