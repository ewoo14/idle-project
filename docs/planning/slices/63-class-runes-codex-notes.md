# PR #63 Class Runes Codex Notes

## Summary
- Added `ERuneType::ClassMastery` and `FRuneInstance` / `FRuneSaveEntry` `ClassRestriction`.
- Expanded rune slots from 6 to 7. Slot index 6 is reserved for class mastery runes.
- Added `FClassRuneFormula` and server `classRune.ts` parity formula for 8 class mastery mappings:
  - Warrior: PhysAtk + PhysDef
  - Mage: MagicAtk
  - Archer: PhysAtk
  - Thief: PhysAtk
  - Cleric: MagicAtk + Hp
  - Paladin: PhysDef + Hp
  - Berserker: PhysAtk
  - Summoner: MagicAtk
- `URuneService` now rejects class mastery runes in regular slots and rejects regular or mismatched-class runes in the class slot.
- Class mastery bonuses are additive into the existing core multiplier path.
- Save version is now 5; legacy slot arrays are expanded by `RestoreState`.
- Added `TryCraftClassRune` and `TryRollClassRuneDrop` entry points on `UIdleGameInstance`.
- HUD/localization now labels class mastery runes.
- Balance simulator keeps regular core/util rune pressure at 6 slots while the runtime slot count is 7.

## Tests
- RED: `npm run test -- classRune.test.ts` failed before implementation because `classRune.js` did not exist.
- GREEN: `npm run build` passed.
- GREEN: `npm run lint` passed.
- GREEN: `npm run test` passed: 39 files, 466 tests, 1 skipped.
- GREEN: UE `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex` passed.
- GREEN: UE Automation affected groups passed: `IdleProject.Rune`, `IdleProject.GameCore.SaveSystem`, `IdleProject.UI.HUD`.
- GREEN: UE full Automation `Automation RunTests IdleProject` passed with exit code 0.
