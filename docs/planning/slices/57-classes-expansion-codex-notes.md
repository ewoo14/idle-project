# PR #57 Classes Expansion - Codex Character Notes

## Scope

- Expanded client `EClassId` from 5 to 8 playable classes:
  `Paladin = 6`, `Berserker = 7`, and `Summoner = 8`.
- Added deterministic `FClassGrowth` profiles for the three classes.
  Existing Warrior, Mage, Archer, Thief, and Cleric growth values were kept
  unchanged.
- Added seven default skills per new class in `USkillComponent` and wired
  `LoadSkillsForClass` for all eight classes.
- Reused existing effect, status, and element enums. No new backend API,
  secrets, or environment files were changed.
- Updated the class selection view model and localization keys so the selector
  exposes eight class options.

## Balance Anchors

- Paladin: high CON with solid STR and WIS support for a holy tank profile.
- Berserker: highest STR growth, lower CON/WIS, physical burst profile.
- Summoner: high INT/WIS, lower HP/CON, sustained magic profile.
- Summoner is routed through magic attack/magic defense in combat formulas and
  skill damage resolution. Paladin and Berserker stay physical.

## Automation

- RED: `Build.bat IdleProjectEditor Win64 Development` failed after adding
  tests because `EClassId::Paladin/Berserker/Summoner` and
  `LoadDefault*Skills` did not exist.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- GREEN: affected Automation groups exited 0:
  `IdleProject.Character.StatFormulas`, `IdleProject.Combat.Skills`,
  `IdleProject.Combat.Formulas.ClassDamage`,
  `IdleProject.UI.HUD.ClassSelectionDisplayModel`,
  `IdleProject.Character.ClassSelection.AppliesSkills`, and
  `IdleProject.Localization.CsvIntegrity`.
- GREEN: full `Automation RunTests IdleProject` exited 0; log reported 158
  tests found and `TEST COMPLETE. EXIT CODE: 0`.

## Review Notes

- The Korean localization CSV files already contain mojibake in existing rows.
  New Korean-side class/skill keys were added with ASCII source strings to keep
  ko/en key parity without further encoding churn.
- Backend parity remains for the backend owner because this Codex pass was
  limited to client C++ and client CSV/localization wiring.
