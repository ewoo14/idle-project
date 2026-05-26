# M6 Damage Floating Text V1 QA Scenarios

## Scope

PR #28 adds combat damage feedback above damaged targets. QA focuses on the
damage event metadata path, HUD view-model timing and styling, respawned monster
coverage, and regressions against existing combat and skill behavior.

## Scenario 1: Normal Damage Text Lifecycle

Given a player attack applies physical damage to a monster
When `UCombatComponent::TakeDamageTyped` broadcasts the damage event
Then the HUD receives the damaged actor, creates one floating text entry above
the target head, shows the rounded amount, moves it upward, fades it during the
1.0s lifetime, and removes it after the lifetime expires.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Component.AnyDamageReceivedEvent`) and
`client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.DamageFloatingTextViewModel`)

## Scenario 2: Edge Damage Metadata And Styling

Given physical, magic, and critical damage events are produced with final damage
amounts
When the floating damage view model is built
Then physical damage uses primary text color, magic damage uses the blue token,
critical damage uses the gold token with larger scale and an emphasis mark, and
zero or expired view-model states do not leave visible text.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.DamageFloatingTextViewModel`)

## Scenario 3: Respawn Regression Coverage

Given an original monster dies and `AIdleProjectGameModeBase::ScheduleRespawn`
creates a replacement monster
When the respawned monster receives damage through its own new
`UCombatComponent`
Then the same global damage event path is emitted with the respawned damaged
actor, so HUD floating text does not depend on stale per-monster bindings.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Component.AnyDamageReceivedEvent`)

## Edge Cases

- Negative damage is clamped before events, so floating text never displays a
  negative number.
- Zero damage does not emit `OnAnyDamageReceived` and should not create a HUD
  entry.
- `Now < StartTime` and entries older than the lifetime are hidden.
- World-to-screen projection failure removes the HUD entry instead of drawing at
  a stale location.
- Player HP HUD updates remain bound only to the player combat component; the
  global damage event is for floating text placement only.

## Regression Checks

- Existing dynamic `OnDamageReceived(float, bool, EDamageKind)` listeners keep
  receiving final amount, crit flag, and damage kind.
- Basic attacks, Mage magic skills, Archer crit skills, Whirlwind AoE, and skill
  rank damage continue to use their existing damage formulas.
- Monster death and respawn still bind `OnDeath` for future respawns.
- HUD does not duplicate floating entries by binding both player-specific and
  global damage delegates.

## Evidence Required

- UE editor target `Build.bat` stdout.
- UE Automation stdout for `IdleProject.Combat.Component.AnyDamageReceivedEvent`
  and `IdleProject.UI.HUD.DamageFloatingTextViewModel`, or full
  `Automation RunTests IdleProject` stdout.
- Markdown lint stdout for `docs/qa/scenarios/M6-damage-floating-text-v1.md`
  and `docs/qa/scenarios/README.md`.
