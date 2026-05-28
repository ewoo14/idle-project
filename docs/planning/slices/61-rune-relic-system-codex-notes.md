# PR #61 Codex Character Notes

## Scope

- Added the client RuneSystem foundation: rune types, formulas, owned/equipped rune service, v3 save payload, and game-instance economy APIs.
- Added core-stat rune multipliers into `AIdleCharacter::RefreshDerivedStats` after transcend, tower, and achievement multipliers.
- Added rune utility hooks for kill gold, kill EXP, and offline reward multipliers.
- Added the server formula mirror in `server/src/core/formulas/rune.ts` and exported it from the formula index.

## TDD Evidence

- RED: `npm test -- rune.test.ts` failed before implementation because `./rune.js` was missing.
- RED: `Build.bat IdleProjectEditor Win64 Development` failed before implementation because `RuneSystem/RuneFormula.h` was missing.
- GREEN: `npm run build` exited 0.
- GREEN: `npm run lint` exited 0.
- GREEN: `npm test` exited 0 with 37 passed files, 444 passed tests, and 1 skipped integration test.
- GREEN: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex` exited 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds='Automation RunTests IdleProject.GameCore.SaveSystem; Quit'` exited 0 with 14 SaveSystem tests found and all successful.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds='Automation RunTests IdleProject; Quit'` exited 0 with 169 tests found and all successful.

## Review Notes

- `SaveVersion` is now 3. v2 saves deliberately restore empty rune state and zero rune essence.
- Equipped rune slots are persisted as six owned-rune indexes. Restore sanitizes invalid rune entries and remaps equipped indexes after filtering.
- Rune enhancement spends both rune essence and gold exactly once. Insufficient resources return `false` without mutation.
- Utility rune caps are enforced in both client and server mirrors: CritDamage 1.0, GoldFind 2.0, ExpBoost 2.0, OfflineEff 0.5.
