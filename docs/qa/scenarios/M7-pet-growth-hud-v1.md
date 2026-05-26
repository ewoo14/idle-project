# M7 Pet Growth HUD V1 QA Scenario

## Scope

- C++ Canvas HUD pet panel growth display.
- Per-pet level, next feed cost, level-scaled bonus, and feed button state.
- `PetFeed_` HitBox click flow through `UIdleGameInstance::TryFeedPet`.
- `OnPetFed` feedback text for success and blocked feed attempts.
- Regression coverage for existing pet equip, season, localization, and gold
  spending guards.

## Scenario 1: Feed-Ready Pet Row

Given the player owns the default `dog` and `bird` pets.

And the equipped dog is level 1.

And current gold is at least `FPetLevelFormula::GetFeedCost(1)`.

When the HUD renders the pet panel.

Then the dog row displays `Lv 1/10`.

And the row displays the next feed cost.

And the level-scaled gold bonus is shown as `22%`.

And the row registers a `PetFeed_dog` HitBox.

Automation: `IdleProject.UI.HUD.PetSeasonViewModels`

## Scenario 2: Insufficient Gold State

Given a pet is below `MaxPetLevel`.

And current gold is lower than the pet's next feed cost.

When the HUD builds the pet row ViewModel.

Then the feed action is disabled.

And the status label reports that more gold is required.

And no `PetFeed_` HitBox is registered for that row.

Automation: `IdleProject.UI.HUD.PetSeasonViewModels`,
`IdleProject.GameCore.IdleGameInstance.PetFeed`

## Scenario 3: Max-Level State

Given a pet is already at `FPetLevelFormula::MaxPetLevel`.

When the HUD builds the pet row ViewModel.

Then the feed action is disabled.

And the feed cost label is replaced by a max-state label.

And the status label reports max level.

Automation: `IdleProject.GameCore.PetService.Growth`,
`IdleProject.UI.HUD.PetSeasonViewModels`

## Scenario 4: Successful Feed Click

Given the player has enough gold for the equipped pet's next feed cost.

When the player clicks the `PetFeed_` HitBox for that pet.

Then the HUD calls `TryFeedPet(PetId)`.

And gold is deducted exactly once.

And the pet level increases by one.

And `OnPetFed` feedback displays `Lv N!` with the spent gold amount.

Automation: `IdleProject.GameCore.IdleGameInstance.PetFeed`

## Scenario 5: Regression Guard

Given pet growth HUD is present with the previous pet equip and season panels.

When the player equips a pet, feeds a pet, claims season rewards, and switches
language between ko/en.

Then existing `PetEquip_` behavior remains unchanged.

And `PetFeed_` does not collide with quest, season, enhancement, stat, shop, or
rebirth HitBox prefixes.

And ko/en UI CSV key parity remains valid.

Automation: `IdleProject.GameCore.PetService.DefinitionParity`,
`IdleProject.GameCore.PetService.EquipBonus`,
`IdleProject.Localization.CsvIntegrity`

## Manual Evidence

- Screenshot: 1080p pet panel with level, feed cost, enabled feed button, and
  scaled bonus visible.
- Screenshot: 1440p or 4K pet panel with no row text overlap.
- Screenshot or log: insufficient-gold row disabled.
- Screenshot or log: max-level row disabled.
- Log/stdout: `TryFeedPet` spends gold once and broadcasts `OnPetFed`.

## Verification

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

$Editor = 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

& $Editor `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.PetSeasonViewModels; Quit' `
  -TestExit='Automation Test Queue Empty'

& $Editor `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```
