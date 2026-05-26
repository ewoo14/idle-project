# M6 Thief / Cleric Classes V1 QA

PR #29 adds Thief and Cleric class skill sets and the new Heal effect. This
scenario records server/client definition parity, character routing, combat
behavior, and regression evidence.

## Skill Loading

Given a `USkillComponent`
When `LoadDefaultWarriorSkills()`, `LoadDefaultMageSkills()`,
`LoadDefaultArcherSkills()`, `LoadDefaultThiefSkills()`, and
`LoadDefaultClericSkills()` are called
Then each class exposes exactly 7 skills: 4 active, 2 passive, 1 ultimate, for
35 total V1 skill definitions.

Automation: `IdleProject.Combat.Skills.ClassDefaults`

## Definition Parity

Given server `server/src/core/data/skills.ts` and client `USkillComponent`
definitions
When the parity tests inspect Warrior, Mage, Archer, Thief, and Cleric
Then all 35 definitions match on `id`, `classId`, `type`, `effectType`,
cooldown, coefficient, buff, and ultimate gauge fields.

Given the Cleric definitions
When the parity tests inspect `heal` and `sanctuary`
Then both definitions use the `heal` effect type.

Automation: `server/src/core/data/skills.test.ts`,
`IdleProject.Combat.Skills.DefinitionParity`

## ClassId Routing

Given `LoadSkillsForClass(EClassId)`
When class ids 1 through 5 are loaded
Then Warrior, Mage, Archer, Thief, and Cleric each select their matching skill
set and replace the previous set.

Automation: `IdleProject.Combat.Skills.ClassDefaults`

## Heal Behavior

Given a Cleric owner at 50% HP
When the `heal` skill resolves
Then HP increases by `round(MaxHp * 0.20)` and does not exceed `MaxHp`.

Given a Cleric owner near full HP
When the `heal` skill resolves
Then the restored HP is clamped to `MaxHp`.

Given a selected target
When the Heal skill resolves
Then the target HP is unchanged because Heal does not enter the damage path.

Automation: `IdleProject.Combat.Skills.HealEffect`

## Damage Routing

Given Thief derived stats
When class damage is computed
Then physical attack is checked against physical defense.

Given Cleric derived stats
When class damage is computed
Then magic attack is checked against magic defense.

Automation: `IdleProject.Combat.Formulas.ClassDamage`

## Evidence Required Before Handoff

- `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex`
- `UnrealEditor-Cmd.exe client/IdleProject.uproject -unattended -nop4 -nosplash -NullRHI -ExecCmds="Automation RunTests IdleProject; Quit" -TestExit="Automation Test Queue Empty"`
