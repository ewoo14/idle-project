# M2 Skill Rank Points V1 QA Scenarios

## Scope

PR #27 extends Skill Tree V1 with level-up skill points, per-skill ranks,
effective damage coefficients, effective cooldowns, and HUD rank-up affordances.

## Scenario 1: Level-Up Grants A Skill Point

Given an idle character has an attached `USkillComponent`
When `AIdleCharacter::HandleLevelUp` receives a level-up event
Then the skill component gains exactly 1 skill point.

Automation: `IdleProject.Character.LevelUp.GrantsSkillPoint`

## Scenario 2: Rank-Up Spends Points And Caps At MaxRank

Given `heavy_strike` is loaded at rank 0 and the character has skill points
When the player ranks up `heavy_strike`
Then each rank spends 1 point and the rank cannot exceed `MaxRank=5`.

Automation: `IdleProject.Combat.Skills.RankPoints`

## Scenario 3: Rank Affects Damage And Cooldown

Given `heavy_strike` has `baseDamageCoeff=2.5` and `baseCooldown=4.0s`
When it reaches rank 5
Then the effective damage coefficient is 3.75 and the effective cooldown is
3.0s, matching +50% damage coefficient and -25% cooldown.

Automation: `IdleProject.Combat.Skills.RankEffectiveValues`

## Scenario 4: Ranked Damage Uses Effective Values

Given a ranked damage skill is ready and the target has combat stats
When `USkillComponent::TickSkills` executes the skill
Then damage application uses `GetEffectiveDamageCoeff` rather than the base
coefficient.

Automation: `IdleProject.Combat.Skills.RankDamageApplication`

## Scenario 5: HUD Exposes Rank State

Given at least one active skill is ranked and unspent skill points remain
When the skill HUD view model is built
Then each active slot exposes current rank, `MaxRank`, available points, and
rank-up availability.

Automation: `IdleProject.UI.HUD.SkillDisplayModel`

## Edge Cases

- Missing skill IDs cannot be ranked up.
- Rank-up with 0 points is rejected.
- Negative or zero point grants are ignored.
- Positive cooldown skills use the rank-reduced cooldown for remaining time and
  ratio calculations.
- Zero-cooldown passive and ultimate rows stay at 0 cooldown.
- Class switching keeps rank/point state in memory, but a rank only applies
  when the matching `SkillId` is loaded.
- Rebirth reset or preservation remains out of scope for V1.

## Evidence Required

- UE Automation stdout for `IdleProject.Combat.Skills.RankPoints`,
  `IdleProject.Combat.Skills.RankEffectiveValues`,
  `IdleProject.Combat.Skills.RankDamageApplication`,
  `IdleProject.Character.LevelUp.GrantsSkillPoint`, and
  `IdleProject.UI.HUD.SkillDisplayModel`.
- Build stdout for the UE editor target before PR handoff.
