# PR #72 Unified Mastery QA Notes

## Scope

QA coverage added for `docs/planning/slices/72-unified-mastery.md` DoD 7 and `docs/superpowers/plans/2026-05-29-unified-mastery.md` Task 11.

## Given / When / Then

### Progression E2E

Given a level 100 Warrior in a transient game world with Quest and Dungeon services initialized.

When `RecordMonsterKilled`, `RecordGearEnhanced`, `TryRunDungeon`, and `ClaimQuest` run through `UIdleGameInstance`.

Then Combat, Equipment, Abyss, and Explore mastery XP increase through their production hooks, Combat and Equipment reach level 1, World Power updates, `GetCurrentDerivedStats()` reflects the core mastery multiplier, and `GetCombatPower()` remains equal to `FCombatPowerFormula::ComputeCombatPower()`.

Automation: `IdleProject.Mastery.ProgressionE2E`.

### Reset Persistence

Given a v13 save with Combat, Equipment, and Explore mastery XP.

When `Rebirth()` and `Transcend()` execute.

Then mastery XP remains unchanged and the global mastery core bonus remains available after the reset.

Automation: `IdleProject.Mastery.ResetPersistence`.

### Zero Regression

Given a legacy v12 save with no mastery payload.

When the save is applied and a Warrior refreshes derived stats.

Then mastery starts at zero World Power, the core multiplier is `1.0`, HP/attack/crit remain at the existing non-mastery baseline, and CP is still computed from the baseline derived stats.

Automation: `IdleProject.Mastery.ZeroRegression`.

### Server Parity

Given mastery level anchors `0, 1, 5, 30, 100`.

When server `mastery.ts` computes `coreStatMultiplier`, `critRateAdd`, `dropRateAdd`, `goldFindPct`, and `expBoostPct`.

Then values match the C++ `FMasteryFormula` float anchors, including `Math.fround` boundaries.

Vitest: `server/src/core/formulas/mastery.parity.test.ts`.

## Verification

- `npm run test -- mastery`
- UE build: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex`
- UE Automation: `Automation RunTests IdleProject.Mastery; Quit`

## Notes

- First UE run failed because the transient Automation world did not attach a `FWorldContext` to `UIdleGameInstance`, so `FindPlayerCharacter()` could not exercise the production `TryRunDungeon` and stat-refresh paths. The test now attaches a real world context and possessed player character before running the E2E flow.
- CEF sandbox warnings appeared on UE command stdout, but the final Automation run completed with exit code 0.
