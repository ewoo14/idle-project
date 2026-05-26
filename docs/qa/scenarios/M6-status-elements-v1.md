# M6 Status Elements V1 QA Scenarios

## Scope

PR #30 adds status effects and element matchups to combat. QA covers Poison and
Burn damage over time, Freeze slow behavior, element multiplier boundaries, and
the HUD status indicators above active combat actors.

## Scenario 1: Poison And Burn Damage Over Time

Given a combat actor has active Poison or Burn with positive duration and
magnitude
When `UCombatComponent::TickStatuses` reaches each one-second tick before
expiry
Then the target takes magic damage for each due tick, the status remains active
until `EndTime`, and no additional damage is applied after the status expires.
If the 5Hz battle update reaches the component after more than one second has
elapsed, all due one-second ticks before expiry are applied in the same status
tick so delayed frames do not skip DoT damage.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Status.DamageOverTime`,
`IdleProject.Combat.Status.DamageOverTimeCatchesUp`)

## Scenario 2: Freeze Attack Speed Slow

Given a combat actor receives Freeze with positive duration and magnitude
When statuses are ticked before expiry
Then attack speed is reduced by the Freeze magnitude, `HasActiveStatus(Freeze)`
returns true, and the original attack speed is restored after expiry.
When Freeze is reapplied before expiry, the previous slow is removed before the
new slow is applied, so the final expiry restores the original attack speed.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Status.FreezeSlow`,
`IdleProject.Combat.Status.FreezeReapplyRestoresAttackSpeed`)

## Scenario 3: Element Multiplier Boundaries

Given a skill element is compared against a monster weak element
When the elements match, oppose each other, or include `None`
Then matching weakness applies `1.5`, opposed Fire/Ice applies `0.5`, unrelated
elements apply `1.0`, and `None` preserves existing neutral damage behavior.

Automation: `client/Source/IdleProject/Tests/CombatTests.cpp`
(`IdleProject.Combat.Formulas.ComputeDamage`)

## Scenario 4: Status HUD Indicators

Given a visible combat actor has active Poison, Burn, or Freeze
When `AIdleHUD::DrawHUD` projects the actor head location to screen
Then the HUD shows compact status indicators in stable order `P`, `B`, `F`,
uses design token colors, lays multiple statuses horizontally, and skips drawing
when projection fails or the status has expired.

Automation: `client/Source/IdleProject/Tests/DamageFloatingTextHudTests.cpp`
(`IdleProject.UI.HUD.StatusIndicatorViewModel`)

## Edge Cases

- `None`, non-positive duration, non-positive magnitude, and dead actors do not
  create active statuses.
- Reapplying the same status replaces the previous entry instead of duplicating
  icons.
- Expired status entries are hidden by the HUD view model even before the next
  combat status tick removes them.
- Multi-status targets keep a stable horizontal order so the display does not
  shuffle between frames.
- World-to-screen projection failure skips the indicator rather than drawing at
  a stale screen coordinate.

## Regression Checks

- Existing damage floating text still uses the global damage event path and is
  not coupled to status indicator rendering.
- Basic physical attacks remain neutral when skill element is `None`.
- Mage Burn and Thief Poison status applications continue to use the same
  `ApplyStatus` path.
- Freeze restores attack speed after expiry and does not permanently stack slow
  penalties.
- Respawned monsters are spawned through `SpawnMonsterAt`, finish `BeginPlay`,
  and restart `BattleAI`, so status ticks and HUD indicators use the same path
  as initially spawned monsters.

## Evidence Required

- UE editor target `Build.bat` stdout.
- UE Automation stdout for `Automation RunTests IdleProject` or the specific
  combat and HUD tests listed above.
- Markdown lint stdout for this file and `docs/qa/scenarios/README.md`.
