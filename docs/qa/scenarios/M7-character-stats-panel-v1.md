# M7 Character Stats Panel V1 QA Scenarios

## Scope

- C++ Canvas HUD character stat info toggle and panel.
- Final primary stats: STR/DEX/INT/WIS/CON/LUK from `GetCurrentPrimaryStats()`.
- Final derived stats: HP, MP, physical attack, magic attack, physical defense,
  magic defense, attack speed, crit rate, crit damage, dodge, and accuracy
  from `GetCurrentDerivedStats()`.
- Header context: class, current level, and rebirth count.
- ko/en UI CSV key parity for all new stat info labels.

## Scenario 1: Toggle opens the character stat info panel

Given the player character has spawned and HUD hit boxes are active.

When the player clicks the `StatInfoToggle` hit box.

Then the stat info panel opens without changing quest log, stat allocation,
shop, enhance, pet, or season panels.

And clicking `StatInfoToggle` again closes only the stat info panel.

Automation: `IdleProject.UI.HUD.StatInfoPanelViewModel`

## Scenario 2: Header shows class, level, and rebirth count

Given the player is a Mage at level 42 with 3 rebirths.

When the stat info ViewModel is built.

Then the header shows the localized class name, `Lv. 42`, and rebirth count
3.

And the values come from `GetClassId()`, `GetCurrentLevel()`, and `GetRebirthCount()`.

Automation: `IdleProject.UI.HUD.StatInfoPanelViewModel`

## Scenario 3: Primary stats show the final applied values

Given `GetCurrentPrimaryStats()` returns STR, DEX, INT, WIS, CON, and LUK
after class growth and stat allocation.

When the stat info panel renders.

Then the panel shows six primary rows in STR/DEX/INT/WIS/CON/LUK order.

And the INT row reads `FPrimaryStats::Int_`, not a separate stale value.

Automation: `IdleProject.UI.HUD.StatInfoPanelViewModel`

## Scenario 4: Derived stats use the final combat values and formats

Given equipment, affix, rebirth bonus points, and passive skills have already
been folded into `GetCurrentDerivedStats()`.

When the stat info ViewModel is built.

Then HP, MP, physical attack, magic attack, physical defense, and magic defense
are rounded as integers.

And attack speed is shown as a percentage.

And crit rate, dodge, and accuracy are shown as percentages.

And crit damage is shown as a multiplier.

Automation: `IdleProject.UI.HUD.StatInfoPanelViewModel`

## Scenario 5: Localization key parity

Given the ko/en UI CSV files include the stat info title, toggle, header, and
derived stat labels.

When CsvIntegrity Automation runs.

Then both languages contain the same key set.

And there are no empty or duplicate rows.

Automation: `IdleProject.Localization.CsvIntegrity`

## Manual Viewport Checks

- 1080p: open the panel and confirm it does not cover HP/EXP/gold, skill
  slots, or the quest log button area.
- 1440p: confirm the primary and derived columns remain aligned and readable.
- 4K: confirm the panel scales with the existing HUD scale and does not drift
  off the right edge.

## Verification Commands

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.StatInfoPanelViewModel; Quit' `
  -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
