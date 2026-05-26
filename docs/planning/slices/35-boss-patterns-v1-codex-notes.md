# PR #35 Boss Patterns V1 - Codex Character Notes

## Scope

- Added client `FBossPhaseFormula` in `client/Source/IdleProject/CombatSystem/`.
- Added boss-only attack phase scaling and special attack broadcast in `UBattleAIComponent`.
- Kept normal monster attack damage and boss base `Atk` / `AtkSpeed` values unchanged.

## Automation Coverage

- `IdleProject.Combat.BossPhase.Formula`
- `IdleProject.Combat.BattleAI.BossAttackPhaseScaling`
- `IdleProject.Combat.BattleAI.NormalAttackUnchangedByBossPhase`

## Verification

- Red check: `Build.bat IdleProjectEditor Win64 Development` failed on missing `CombatSystem/BossPhaseFormula.h`.
- Build: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- Automation:
  `UnrealEditor-Cmd.exe ... Automation RunTests IdleProject` exited 0.
- Automation log summary: 87 success, 0 fail.
