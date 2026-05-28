# PR #67 Codex Character Notes

## Scope

- Added `EUniqueTrait` and `FItemInstance.UniqueTrait1/2` with `None` defaults.
- Added `FUniqueTraitFormula` for value parity, rarity gating, deterministic rolls,
  flat utility accumulation, and core-stat percent multiplier accumulation.
- Wired generated drops so Unique gear rolls one trait, Transcendent gear rolls two
  distinct traits, and Mythic gear rolls no unique traits.
- Bumped local save writes/defaults to `SaveVersion = 9`.
- Added server mirror `server/src/core/formulas/uniqueTrait.ts` and formula export.

## Rules Preserved

- Trait eligibility is exactly `Unique = 4` and `Transcendent = 6`.
- `Mythic = 7` stays excluded from unique traits.
- `None` traits contribute zero, including legacy/v8 saves that lack the fields.
- Transcendent trait value is `Unique * 1.5`.
- Transcendent roll guarantees `UniqueTrait1 != UniqueTrait2`.
- Core-stat traits are not added to flat `EquipmentBonus`; they are accumulated
  as percent multipliers and applied once in `RefreshDerivedStats` after
  `DeriveStats` has composed base plus flat stats.
- `CritDamageSurge`, `CritRateSurge`, and `SwiftSurge` remain flat utility
  additions through the equipment bonus path.

## Verification

- PR #67 Codex fix:
  - RED: `npm test -- uniqueTrait.test.ts` failed on missing
    `accumulateTraitEffects`, proving server formulas lacked split
    flat/percent semantics.
  - GREEN: `npm test -- uniqueTrait.test.ts equipment.test.ts balance-sim.test.ts`
    passed 54 tests.
  - `npm run balance:sim` regenerated
    `tools/balance-sim/reports/balance-sim-report.md`; Transcendent
    AllStatSurge+PhysMastery pressure is CP x1.175 / DPS x1.362.
  - `npm run build`, `npm run lint`, and `npm test` passed; Vitest reported
    41 files passed, 1 skipped; 528 tests passed, 1 skipped.
  - UE `Build.bat IdleProjectEditor Win64 Development` passed.
  - UE `Automation RunTests IdleProject` passed with exit code 0; unique trait
    automation paths `AccumulateBonus`, `ComputeEquipmentBonus`, `RollRules`,
    `Values`, and `ItemFactoryRolls` completed successfully.

- RED: `npm test -- src/core/formulas/uniqueTrait.test.ts` failed because
  `./uniqueTrait.js` did not exist.
- GREEN: `npm test -- src/core/formulas/uniqueTrait.test.ts` passed 4 tests.
- `npm run build` passed.
- `npm run lint` passed.
- `npm test` passed: 41 files passed, 1 skipped; 525 tests passed, 1 skipped.
- UE Build passed:
  `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject`.
- UE Automation passed:
  `Automation RunTests IdleProject; Quit`.
