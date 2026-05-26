# M6 Boss Patterns HUD V1 QA Scenarios

Scope: PR #35 boss phase HUD, boss HP bar, and special attack warning display.

## Scenario 1: Boss HP Bar Appears Only For Bosses

Given the world contains an alive `AIdleMonster` with `IsBoss() = true`  
When `AIdleHUD::DrawHUD` resolves visible combat actors  
Then the HUD shows a top-center boss HP bar with current and max HP.

Given the world contains only normal monsters  
When the HUD resolves visible combat actors  
Then the boss HP bar is hidden and the existing stage HUD remains unchanged.

Automation: `IdleProject.UI.HUD.BossViewModel`

## Scenario 2: Boss Phase Mirrors Combat Formula

Given a boss has `CurrentHp = 250` and `MaxHp = 500`  
When the boss HUD view model is built  
Then the HP ratio is `0.5`, the percent is `50`, and the phase label is
`Phase 2`.

Given the boss reaches zero HP during the final rendered frame  
When the boss HUD view model is built  
Then the HP ratio is `0.0` and the phase remains `3`.

Automation: `IdleProject.UI.HUD.BossViewModel`,
`IdleProject.Combat.BossPhase.Formula`

## Scenario 3: HP Changes Refresh The Boss Bar

Given a boss is alive and visible  
When combat damage changes `CurrentHp` from phase 1 to phase 2 or phase 3  
Then the next `DrawHUD` call reads `UCombatComponent::CurrentHp` and updates
the bar fill, HP label, phase label, and phase color.

Manual evidence: capture the boss HUD at 100 percent, about 50 percent, and
below 33 percent HP in PIE.

## Scenario 4: Special Attack Warning Is Transient

Given the boss `UBattleAIComponent` broadcasts `OnBossSpecialAttack`  
When the HUD receives the event  
Then a localized warning banner is shown near the boss bar for a short duration.

Given the boss despawns or the HUD ends play  
When the HUD unbinds from the boss battle AI  
Then later broadcasts do not call stale HUD delegates.

Automation: `IdleProject.Combat.BattleAI.BossAttackPhaseScaling`

## Regression Checks

- Existing skill, stage, rebirth, enhancement, stat, pet, season, floating
  damage, and status HUD panels still render from their current data paths.
- Boss HUD copy exists in both `client/Content/Localization/Game/ko/UI.csv`
  and `client/Content/Localization/Game/en/UI.csv`.
- CSV integrity remains green for duplicate keys, empty keys, and ko/en keyset
  parity.
- No `.env` or secret files are modified.

## Verification Commands

```powershell
$Editor = "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project="C:/game/idle game/repo/client/IdleProject.uproject" `
  -WaitMutex

& $Editor `
  "C:/game/idle game/repo/client/IdleProject.uproject" `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds="Automation RunTests IdleProject; Quit" `
  -TestExit="Automation Test Queue Empty"
```
