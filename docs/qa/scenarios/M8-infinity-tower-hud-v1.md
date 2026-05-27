# M8 Infinity Tower HUD V1 QA Scenarios

## Scope

- C++ Canvas HUD tower panel.
- Highest floor, next-floor CP requirement, current combat power, climb CTA, and
  climb feedback.
- `TowerClimb` HitBox click path into `UIdleGameInstance::ClimbTower()`.
- ko/en UI CSV integrity for tower HUD copy.

## Scenario 1: Tower panel shows current tower state

Given the player character has spawned and `UTowerService` is initialized.

When `AIdleHUD::DrawHUD()` builds the tower panel.

Then the panel shows the localized title, highest cleared floor, next required
combat power, and current `AIdleCharacter::GetCombatPower()` value.

And the numeric labels use thousands separators.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`

## Scenario 2: Enough CP enables climb

Given the next floor requires 152 combat power.

And the player has 200 combat power.

When the tower panel ViewModel is built.

Then the status is `Ready to climb`.

And the CTA registers the stable `TowerClimb` HitBox.

And clicking `TowerClimb` calls `UIdleGameInstance::ClimbTower()`.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`,
`IdleProject.GameCore.IdleGameInstance.TowerHooks`

## Scenario 3: Insufficient CP blocks climb

Given the next floor requires 1,234,567 combat power.

And the player has 999 combat power.

When the tower panel ViewModel is built.

Then the status is `Need more CP`.

And no climb reward is granted.

And the blocked feedback uses the localized tower CP shortage copy.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`,
`IdleProject.GameCore.Tower.ServiceClimb`

## Scenario 4: Successful climb shows reward feedback

Given `UTowerService::TryClimbTower()` clears at least one new floor.

When `UTowerService::OnTowerClimbed` broadcasts the new highest floor and
total reward.

Then `AIdleHUD::HandleTowerClimbed()` shows localized feedback.

And the feedback includes the new highest floor and gold reward, for example
`Floor 7 cleared! Gold +12,500`.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`,
`IdleProject.GameCore.Tower.ServiceClimb`

## Scenario 5: Tower HUD copy is localized

Given ko/en UI CSV files include all `TOWER_*` keys.

When `IdleProject.Localization.CsvIntegrity` runs.

Then both languages contain the same tower HUD keys.

And English copy renders the approved `Infinity Tower`, `Climb`, and
`Need more CP` labels.

Automation: `IdleProject.Localization.CsvIntegrity`,
`IdleProject.UI.HUD.TowerPanelViewModel`

## Manual Viewport Checks

- 1080p: confirm the tower panel is centered below the stage and boss HUD and
  does not cover left/right progression panels or the bottom skill HUD.
- 1440p: confirm the title, CP labels, status, and CTA stay inside the panel.
- 4K: confirm the existing Canvas-height 1.0-2.0 scale keeps the panel readable
  and the feedback line remains within the panel border.

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
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.TowerPanelViewModel; Quit' `
  -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Localization; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
