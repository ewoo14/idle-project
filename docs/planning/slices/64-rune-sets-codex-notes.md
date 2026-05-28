# PR #64 Codex Notes - Rune Sets

## Scope

- Added `ERuneSet` and `FRuneInstance` / `FRuneSaveEntry` persistence.
- Added `FRuneSetFormula` and server mirror `runeSet.ts`.
- Integrated set counting into `URuneService` for regular slots 0-5 only.
- Bumped client save payload version to 6.

## TDD Evidence

- RED: `npm run test -- runeSet.test.ts` failed because `./runeSet.js` did not exist.
- RED: `Build.bat IdleProjectEditor Win64 Development` failed because `RuneSystem/RuneSetFormula.h` did not exist.
- GREEN: `npm run test -- runeSet.test.ts` passed after adding `runeSet.ts`.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` succeeded after adding the C++ implementation.

## Review Notes

- `FRuneSetFormula::ComputeSetBonus` resets outputs to pure zero-base additive bonuses.
- `GetEquippedUtilValues` caps regular utility rune values before applying set bonuses.
- ClassMastery slot index 6 is excluded from rune-set counting.
- Legacy v5-style entries without `RuneSet` remain `None` and contribute no set bonus.
