# M8 Combat Power HUD V1 QA Scenarios

## Scope

- Combat Power value from `AIdleCharacter::GetCombatPower()`.
- C++ Canvas HUD stat info panel display.
- ko/en UI CSV localization for `HUD_COMBAT_POWER_FORMAT`.
- Growth source visibility across stat allocation, equipment, enhancement,
  set bonus, rebirth, transcendence, and skill rank/passive paths.

## Scenario 1: Stat info panel shows current combat power

Given the player character has spawned and `RefreshDerivedStats()` has built
the final derived stat cache.

When `AIdleHUD::DrawHUD()` resolves the player through
`ResolvePlayerCharacter()` and builds the stat info ViewModel.

Then the stat info panel shows a localized combat power label.

And the displayed number equals `ResolvePlayerCharacter()->GetCombatPower()`.

And the value uses thousands separators, for example
`Combat Power 1,234,567`.

Automation: `IdleProject.UI.HUD.StatInfoPanelViewModel`

## Scenario 2: Combat power copy is localized

Given ko/en UI CSV files include `HUD_COMBAT_POWER_FORMAT`.

When `IdleProject.Localization.CsvIntegrity` runs.

Then both languages contain the same combat power key.

And English copy renders `Combat Power {Amount}`.

And Korean copy renders `전투력 {Amount}`.

Automation: `IdleProject.Localization.CsvIntegrity`,
`IdleProject.Localization.LookupAndCultureSwitch`

## Scenario 3: Stat allocation changes are reflected on next draw

Given the player has unspent stat points and the stat info panel is open.

When the player allocates STR, DEX, INT, WIS, CON, or LUK through the stat
allocation HUD.

Then `AIdleCharacter::HandleStatPointsChanged()` refreshes derived stats.

And the next `DrawHUD()` call reads the updated combat power.

And combat power does not remain at the pre-allocation value.

Automation: `IdleProject.Character.CombatPower.GrowthSources`

## Scenario 4: Equipment, affix, set, and enhancement growth are reflected

Given the player equips items with attack, defense, HP, affix, and set bonus
contributions.

When equipment is equipped or enhanced.

Then `AIdleCharacter::HandleEquippedChanged()` refreshes derived stats.

And the combat power label increases through the same final derived stat path
used by the character stat panel.

Automation: `IdleProject.Character.CombatPower.GrowthSources`,
`IdleProject.Inventory.Bonus.Affixes`, `IdleProject.Inventory.Bonus.SetBonus`,
`IdleProject.Inventory.EnhanceEquippedItem.SuccessAndMaxGuard`

## Scenario 4.1: Growth-source parity never leaves final derived stats

Given CP has been sampled at base, stat allocation, equipment, enhancement,
rebirth, and transcendence milestones.

When each milestone calls `AIdleCharacter::GetCombatPower()`.

Then the value equals
`FCombatPowerFormula::ComputeCombatPower(GetCurrentDerivedStats())`.

And no milestone uses a stale pre-refresh stat cache or a direct CP increment.

Automation: `IdleProject.Character.CombatPower.GrowthSources`

## Scenario 5: Rebirth and transcendence growth are reflected

Given the player has rebirth bonus points or a transcend stat multiplier.

When rebirth or transcendence state changes and the player character refreshes
derived stats.

Then the combat power label reflects the refreshed `GetCurrentDerivedStats()`
output.

And the label remains a display-only value; it does not gate rebirth,
transcendence, stages, quests, shop, or skill rank actions in V1.

Automation: `IdleProject.Character.CombatPower.GrowthSources`,
`IdleProject.GameCore.Rebirth.ResetAndBonus`,
`IdleProject.GameCore.Transcend.ResetAndCount`

## Manual Viewport Checks

- 1080p: open stat info and confirm the combat power line is readable below
  the class/level/rebirth header.
- 1440p: confirm the combat power line does not collide with the first stat
  row or the stat info toggle.
- 4K: confirm the combat power line scales with the existing HUD scale and
  remains inside the right panel.

## Verification Commands

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex -NoHotReload

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.StatInfoPanelViewModel; Quit' `
  -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Localization; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
