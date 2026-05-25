# M2 Skill Tree V1 QA Scenarios

## Scope
- PR #15 Skill Tree V1: warrior active/passive/ultimate skills, cooldown HUD, ultimate gauge, server SkillDB mirror.
- Automation targets: UE Automation (`IdleProject.Combat.Skills`, `IdleProject.Combat.BattleAI.AoeTargetGathering`, `IdleProject.UI.HUD.SkillDisplayModel`) and Vitest (`src/core/data/skills.test.ts`).

## Scenario 1: Active Skill Cooldowns
- Given a warrior has the default 7 skills loaded.
- When `heavy_strike` is cast at 10.0s.
- Then it is not ready at 12.0s, reports 2.0s remaining, reports 0.5 cooldown ratio, and becomes ready at 14.0s.

## Scenario 2: AoE Target Collection
- Given one owner, two living enemies inside 400 units, one enemy outside 400 units, and one dead enemy inside 400 units.
- When BattleAI gathers AoE targets for `whirlwind`.
- Then both living in-range enemies are included, the far enemy is excluded, the dead enemy is excluded, and the owner is excluded.

## Scenario 3: Ultimate Gauge Boundaries
- Given the ultimate gauge starts at 0.
- When gauge is added above 100, consumed, and then decremented below 0.
- Then it clamps at 100, becomes ready, resets to 0 on consume, and never drops below 0.

## Scenario 4: Passive Stat Application
- Given base derived stats HP 1000, ATK 200, DEF 50.
- When warrior passives are applied.
- Then `weapon_mastery` raises ATK to 230, `toughness` raises HP to 1200, and DEF is unchanged.

## Scenario 5: HUD Cooldown And Gauge
- Given `heavy_strike` was cast 2 seconds ago and ultimate gauge is 100.
- When the HUD skill view model is built.
- Then four active slots are shown, the first slot displays `강타`, cooldown ratio is 0.5, remaining time is 2.0s, and the ultimate view model is ready with 1.0 gauge ratio.

## Scenario 6: Server SkillDB Mirror
- Given the server exposes a read-only warrior skill definition seed.
- When Vitest loads `warriorSkillDefinitions`.
- Then all 7 warrior skills have stable `skillId`, `classId`, `type`, `effectType`, cooldown, coefficient, buff, and gauge fields for cross-reference.

## Regression Levels
- Smoke: run HUD model, cooldown, and SkillDB mirror tests.
- Functional: run all `IdleProject.Combat.Skills`, `IdleProject.Combat.BattleAI.AoeTargetGathering`, and server Vitest.
- Edge: include gauge clamp/reset, 0 cooldown passive/ultimate rows, dead target exclusion, owner exclusion, far target exclusion, and 2D distance checks.

## Reproduction Evidence
- UE command: `UnrealEditor-Cmd.exe client/IdleProject.uproject -unattended -nop4 -nosplash -NullRHI -ExecCmds="Automation RunTests IdleProject;Quit"`
- Server command: `npm test -- --run src/core/data/skills.test.ts`
