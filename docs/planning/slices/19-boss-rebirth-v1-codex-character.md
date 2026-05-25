# PR 19 Codex Character Implementation Note

Date: 2026-05-26

## Scope

- Client-side Chapter 1 boss V1.
- Client-side rebirth V1 in `UIdleGameInstance`.
- C++ and TypeScript stat formula parity for rebirth bonus points.
- Automation and Vitest coverage for the new behavior.

## Implemented Rules

- Chapter 1 boss is the last monster in the initial `AIdleProjectGameModeBase` spawn group.
- Boss V1 stats are `HP 500 / ATK 24 / DEF 12 / ATK speed 0.8`.
- Normal monster stats remain `HP 50 / ATK 8 / DEF 5 / ATK speed 1.0`.
- Boss death marks `bChapter1BossDefeated` through `UIdleGameInstance::MarkChapter1BossDefeated()`.
- `CanRebirth()` is true only when Chapter 1 boss is defeated and character level is at least 100.
- `Rebirth()` increments `RebirthCount`, adds `5` `RebirthBonusPoints`, resets level to `1`, resets EXP to `0`, keeps `floor(Gold * 0.1)`, and clears the Chapter 1 boss flag.
- Rebirth bonus point V1 grants `HP +10` and `PhysAtk +2` per point in both UE C++ and server TypeScript formula mirrors.
- Offline reward preview/claim uses stored `RebirthCount` by default while keeping explicit test overrides.

## Verification

- `npm test -- --run src/core/formulas`
- `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex`
- `UnrealEditor-Cmd.exe client/IdleProject.uproject -ExecCmds="Automation RunTests IdleProject; Quit" -TestExit="Automation Test Queue Empty"`
