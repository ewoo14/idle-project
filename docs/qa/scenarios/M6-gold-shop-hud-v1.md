# M6 Gold Shop HUD V1 QA Scenario

## Scope

- C++ Canvas HUD gold shop panel rendering.
- Gear roll cost display from
  `FShopFormula::GetGearRollCost(GlobalStageIndex)`.
- `ShopGearRoll` HitBox registration only when current gold is enough.
- `TryBuyGearRoll` click flow, gold deduction, guaranteed item result, and
  recent result feedback.
- Regression coverage for existing inventory, enhancement, and localization flows.

## Scenario 1: Gear Roll Ready State

Given the player has an inventory component and current gold greater than the
current stage gear roll cost.

When the HUD renders the gold shop panel.

Then the panel displays the gear roll cost and current gold.

And the roll button is enabled and registers the `ShopGearRoll` HitBox.

Automation: `IdleProject.UI.HUD.ShopPanelViewModel`

## Scenario 2: Insufficient Gold State

Given the player has an inventory component and current gold lower than the
current stage gear roll cost.

When the HUD renders the gold shop panel.

Then the panel displays an insufficient-gold state.

And the roll button is disabled and does not register the `ShopGearRoll` HitBox.

Automation: `IdleProject.UI.HUD.ShopPanelViewModel`,
`IdleProject.GameCore.IdleGameInstance.GearRollPurchase`

## Scenario 3: Successful Gear Roll Click

Given the player has enough gold for one gear roll.

When the player clicks the `ShopGearRoll` HitBox.

Then the HUD calls `TryBuyGearRoll(PlayerInventory)`.

And gold is deducted exactly once by the roll cost.

And one guaranteed non-None equipment item is added to inventory.

Automation: `IdleProject.GameCore.IdleGameInstance.GearRollPurchase`

## Scenario 4: Recent Result Feedback

Given a gear roll purchase succeeds.

When `OnShopPurchase` broadcasts the result.

Then the HUD stores the result and displays the rarity, item name, and acquired
feedback.

And the feedback color uses `RarityToColor(Result.Rarity)`.

Automation: `IdleProject.UI.HUD.ShopPanelViewModel`

## Scenario 5: Regression Guard

Given the gold shop HUD is present with enhancement and stat panels.

When the player uses enhancement, stat allocation, quest, pet, season, and
rebirth controls.

Then existing HitBox prefixes remain unchanged:
`EnhanceSlot_`, `StatAlloc_`, quest, pet, season, rebirth, and offline reward
actions.

And CSV key parity remains valid for ko/en UI localization.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`,
`IdleProject.UI.HUD.StatAllocationPanelViewModel`,
`IdleProject.Localization.CsvIntegrity`

## Verification

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex -NoHotReloadFromIDE

$Editor = 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

& $Editor `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.ShopPanelViewModel; Quit' `
  -TestExit='Automation Test Queue Empty'

& $Editor `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```
