# PR #73 Consumables QA Notes

## Scope

QA coverage for `docs/planning/slices/73-consumables.md` section 2.7 / DoD 6 and `docs/superpowers/plans/2026-05-29-consumables.md` Task 8.

## Given / When / Then

### Consumable E2E

Given a Warrior in a transient game world with `UIdleGameInstance`, `UBuffService`, and a possessed `AIdleCharacter`.

When `TryUseConsumable` activates AttackTonic, GuardTonic, AllStatElixir, FortuneScroll, GoldFeast, and WisdomBooster through production GameInstance APIs.

Then stock is consumed, `GetCurrentDerivedStats()` and `GetCombatPower()` increase only through the expected stat paths, Gold/EXP/Drop bonuses apply once in their economy paths, and an expired saved buff restores the baseline stats and CP.

Automation: `IdleProject.Consumable.GameInstanceHooks`.

### Buff Lifecycle And Edges

Given an initialized `UBuffService`.

When zero, negative, invalid, same-type reuse, different-type simultaneous buffs, import clamps, export, and import are exercised with explicit `NowUnixSec` values.

Then invalid inputs have no side effects, same-type reuse refreshes to `Now + 1800`, different buff types coexist, expired buffs return multiplier `1.0`, economy getters return zero after expiry, and save round-trip preserves count and active end timestamps.

Automation: `IdleProject.Consumable.BuffServiceLifecycle`.

### Formula And Parity

Given the six V1 consumable type ids from AttackTonic through WisdomBooster.

When C++ `FConsumableFormula` and server `consumable.ts` compute buff percent and duration.

Then all six anchors match the agreed `Math.fround` server values and invalid types return `0`.

Automation: `IdleProject.Consumable.FormulaAnchors`.

Vitest: `server/src/core/formulas/consumable.parity.test.ts`.

### Save Migration And Reset Persistence

Given v14 saves with consumable counts plus active buff end timestamps, and a legacy v13 save with the same payload shape.

When `ApplyFromSave`, `CaptureToSave`, `Rebirth()`, and `Transcend()` execute.

Then v14 preserves count and end timestamps, v13 migrates to empty consumable state, and Rebirth/Transcend do not reset consumable inventory or active buff timestamps.

Automation: `IdleProject.Consumable.ResetPersistence`.

## Regression Surface

- Existing baseline stats and CP are checked after an expired AttackTonic is loaded.
- Gold, EXP, and Drop paths are checked through production `AddGold`, `AddExp`, and `ApplyEquippedPetDropBonusChance`.
- Edge cases cover invalid type ids, zero/negative adds, negative imported payload fields, same-type refresh, and simultaneous different-type buffs.

## Verification

- Server lint: `cd server; npm run lint`
- Server targeted tests: `cd server; npm run test -- consumable`
- UE build: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex`
- UE Automation: `Automation RunTests IdleProject.Consumable; Quit`

## Notes

- `UIdleGameInstance::GetCurrentUnixSeconds()` is static, so exact-time expiry is covered through `UBuffService` APIs that accept `NowUnixSec` and through expired save import in the GameInstance E2E test.
- No Playwright coverage is required for this slice because the consumable UI contract is covered by `IdleProject.UI.HUD.ConsumablePanelViewModel` and localization integrity tests.
