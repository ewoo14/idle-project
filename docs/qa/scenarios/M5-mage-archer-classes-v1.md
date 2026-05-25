# M5 Mage/Archer Classes V1 QA Scenarios

## Scope

PR #21 adds Mage(classId 2) and Archer(classId 3) beside the existing
Warrior(classId 1). QA focuses on class skill definition parity, class loading,
story branch localization, and balance guardrails.

## Scenario 1: Skill Definition Parity

Given the server SkillDB mirror defines Warrior, Mage, and Archer V1 skill
sets
When the parity tests load classId 1, 2, and 3 definitions
Then each class exposes exactly 7 skills with matching skillId, classId, type,
effectType, cooldown, damageCoeff, buffMagnitude, buffDuration,
gaugeGainOnHit, and gaugeGainOnTakeDamage.

Automation: `server/src/core/data/skills.test.ts`,
`client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Skills.DefinitionParity`)

## Scenario 2: Class Loading and Replacement

Given an idle character starts with a skill component
When the selected class changes from Warrior to Mage or Archer
Then the previous class skills are reset and only the selected class skill set
remains loaded.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Skills.ClassDefaults`,
`IdleProject.Character.ClassSelection.AppliesSkills`)

## Scenario 3: Combat Role Coverage

Given Mage and Archer use different V1 combat roles
When passive and ultimate skills are applied
Then Mage improves MagicAtk/MP and Archer improves CritRate/AtkSpeed without
changing unrelated stats.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Skills.PassiveStats`,
`IdleProject.Combat.Skills.UltimateBuff`)

## Scenario 4: Story Branch Localization

Given the player selects Warrior, Mage, or Archer
When chapter 1 intro story text requests the class branch key
Then the localization table contains a non-empty Korean line for the selected
class and does not require `{PlayerName}`.

Manual check: `client/Content/Localization/Game/ko/StoryText.csv`

## Edge Cases

- Invalid classId returns no server skill definitions and must not silently map
  to Mage or Archer.
- Repeated class switching must not duplicate active/passive/ultimate skills.
- Zero-cooldown passive and ultimate entries must not be shown as active HUD
  cooldown slots.
- AoE skills must keep lower coefficients than equivalent single-target skills
  for the same class.

## Evidence Required

- Vitest stdout for `src/core/data/skills.test.ts` or full `npm test`.
- UE Automation stdout for `IdleProject.Combat.Skills.DefinitionParity`.
- Build and lint stdout before PR handoff.
