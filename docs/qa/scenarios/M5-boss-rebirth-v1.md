# M5 Boss And Rebirth V1 QA Scenarios

Scope: PR #19 boss defeat gate, rebirth reset, permanent stat bonus, and
offline reward carryover.

## Scenario 1: Boss Gate Controls Rebirth Availability

Given a player has not defeated the Chapter 1 boss  
When the player reaches level 100  
Then `CanRebirth` is false.

Given a player defeated the Chapter 1 boss but is below level 100  
When the rebirth panel is evaluated  
Then `CanRebirth` is false.

Given a player defeated the Chapter 1 boss and reached level 100  
When the rebirth panel is evaluated  
Then `CanRebirth` is true and the rebirth action is enabled.

Automation: `server/src/modules/character/character.test.ts`,
`client/Source/IdleProject/Tests/RebirthTests.cpp`,
`client/Source/IdleProject/Tests/OfflineRewardHudTests.cpp`

## Scenario 2: Rebirth Reset Is Deterministic

Given a level 100 character has defeated the Chapter 1 boss and has 1,234
gold  
When the player performs rebirth  
Then `rebirthCount` increases by `1`  
And `rebirthBonusPoints` increases by `5`  
And level resets to `1`  
And EXP resets to `0`  
And gold keeps `10%` rounded down  
And the Chapter 1 boss gate resets to false.

Automation: `server/src/modules/character/character.test.ts`,
`client/Source/IdleProject/Tests/RebirthTests.cpp`

## Scenario 3: Rebirth Stat Formula Mirrors Server And Client

Given the warrior level 1 base derived stats are `HP = 120` and
`PhysAtk = 24`  
When `deriveStats` is computed with rebirth bonus points `0`, `5`, and `10`  
Then each point adds `+10 HP` and `+2 PhysAtk`  
And non-target derived stats such as attack speed remain unchanged.

Automation: `server/src/core/formulas/stats.test.ts`,
`client/Source/IdleProject/Tests/StatFormulasTests.cpp`,
`client/Source/IdleProject/Tests/RebirthTests.cpp`

## Scenario 4: Equipment Enhancement And Rebirth Bonuses Do Not Cross-Wire

Given equipped item bonuses use the enhancement multiplier
`1 + enhanceLevel * 0.1`  
When equipment bonuses are combined with rebirth-derived combat stats  
Then equipment enhancement still affects only equipment bonus totals  
And rebirth bonus points still affect only HP and physical attack in `deriveStats`.

Automation: `server/src/core/formulas/equipment.test.ts`,
`server/src/core/formulas/stats.test.ts`,
`client/Source/IdleProject/Tests/InventoryTests.cpp`,
`client/Source/IdleProject/Tests/StatFormulasTests.cpp`

## Scenario 5: Offline Rewards Include Rebirth Count

Given an offline reward preview or claim includes a non-zero `rebirthCount`  
When rewards are computed for the same capped duration and level  
Then the rebirthed character receives the documented `0.05` multiplier per
rebirth count  
And zero elapsed time still pays zero reward.

Automation: `server/src/core/formulas/offline.test.ts`,
`server/src/modules/offline/offline.service.test.ts`,
`client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Scenario 6: Rebirth HUD Shows The Preview Reward

Given a player has defeated the Chapter 1 boss and reaches level 100 with zero
rebirths  
When the rebirth panel is evaluated  
Then the current reward preview reads `이번 환생 보상 +5 포인트`.

Given the player has multiple prior rebirths and reaches level 150  
When `PreviewRebirthReward()` returns the scaled reward for the next
rebirth  
Then the HUD uses that exact preview value instead of the legacy fixed `+5`
copy.

Automation: `client/Source/IdleProject/Tests/RebirthTests.cpp`,
`client/Source/IdleProject/Tests/OfflineRewardHudTests.cpp`

## Manual Checks

- Capture the rebirth panel before boss clear, after boss clear below level
  100, and after boss clear at level 100.
- Capture the post-rebirth state showing level 1, reset EXP, retained 10%
  gold, increased rebirth count, and increased bonus points.
- Capture offline preview output before and after rebirth count changes to
  confirm the reward multiplier changes only through `rebirthCount`.
- Capture a scaled-reward rebirth panel at 1080p, 1440p, and 4K.
- Confirm the preview reward line stays inside the right-side panel and does
  not overlap the action button.
