# M8 Tower Milestone Bonus V1 QA Scenarios

## Scope

- C++ Canvas HUD tower panel milestone bonus labels.
- Current permanent tower multiplier and next milestone floor.
- ko/en UI CSV integrity for milestone HUD copy.
- Manual 1080p, 1440p, and 4K panel overlap checks.

## Scenario 1: Tower Panel Shows Current Milestone Multiplier

Given the highest cleared tower floor is 0 through 9.
When `IdleProject::UI::BuildTowerViewModel()` builds the tower panel model.
Then the milestone label shows `Permanent Bonus x1.00`.
And the next milestone label shows `Next +2%: Floor 10`.
And the combat power label remains visible in the same panel.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`

## Scenario 2: Floor 10 Updates Permanent Bonus

Given the highest cleared tower floor is 10.
When the tower panel ViewModel is built.
Then `FTowerMilestoneFormula::GetTowerMilestoneMultiplier(10)` is represented
as `Permanent Bonus x1.02`.
And the next milestone label points to floor 20.
And the current CP, next CP, status, and climb CTA remain unchanged.

Automation: `IdleProject.UI.HUD.TowerPanelViewModel`,
`IdleProject.GameCore.Tower.MilestoneFormula`

## Scenario 3: Milestone Copy Is Localized

Given ko/en UI CSV files include the tower milestone keys.
When `IdleProject.Localization.CsvIntegrity` runs.
Then both languages contain `TOWER_MILESTONE_MULTIPLIER_FORMAT` and
`TOWER_NEXT_MILESTONE_FORMAT`.
And English copy renders the approved `Permanent Bonus x1.02` and
`Next +2%: Floor 20` labels.

Automation: `IdleProject.Localization.CsvIntegrity`,
`IdleProject.UI.HUD.TowerPanelViewModel`

## Scenario 4: Viewport Layout Remains Stable

Given the tower panel includes title, floor, next CP, current CP, milestone
multiplier, next milestone, status, CTA, and feedback.
When the panel is checked at 1080p, 1440p, and 4K.
Then all labels remain inside the panel border.
And the tower panel does not overlap the stage indicator, boss bar, left-side
systems panels, or bottom skill HUD.

Automation: manual PIE viewport smoke plus
`IdleProject.UI.HUD.TowerPanelViewModel`

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
  -ExecCmds='Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
