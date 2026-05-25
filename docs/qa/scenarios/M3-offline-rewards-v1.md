# M3 Offline Rewards V1 QA Scenarios

Scope: PR #16 offline reward formula, preview, claim, and client mirror parity.

## Scenario 1: Preview Shows Claimable Offline Rewards

Given an authenticated player has a level 10 character with `lastSeenAt` one hour before now  
When the player calls `GET /v1/offline/preview`  
Then the response includes `cappedSeconds = 3600`, positive `gold`, positive `exp`, and a `timeBonusMultiplier` greater than `1`  
And the character totals and `lastSeenAt` are not mutated.

Automation: `server/src/modules/offline/offline.service.test.ts`

## Scenario 2: Claim Applies Rewards Once

Given an authenticated player has a character with elapsed offline time  
When the player calls `POST /v1/offline/claim`  
Then the server increments `gold` and `totalExp` by the computed rewards  
And `lastSeenAt` advances to the claim timestamp  
And a duplicate claim at the same timestamp returns no additional local client reward.

Automation: `server/src/modules/offline/offline.service.test.ts`, `client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Scenario 3: Cap Boundary Is Exact

Given elapsed offline time is near the 12 hour cap  
When rewards are computed at 43,199 seconds, 43,200 seconds, and 43,201 seconds  
Then 43,199 remains uncapped  
And 43,200 is accepted as the cap  
And 43,201 clamps to 43,200.

Automation: `server/src/core/formulas/offline.test.ts`, `client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Scenario 4: Zero And Invalid Time Do Not Pay

Given `nowUnixSec` is equal to or earlier than `lastSeenUnixSec`  
When rewards are previewed or claimed  
Then preview returns zero `cappedSeconds`, `gold`, and `exp`  
And claim is rejected with `OFFLINE_REWARD_EMPTY`.

Automation: `server/src/core/formulas/offline.test.ts`, `server/src/modules/offline/offline.service.test.ts`

## Scenario 5: Unix Epoch Last Seen Is Valid

Given a valid character has `lastSeenUnixSec = 0`  
When rewards are computed with `nowUnixSec > 0`  
Then the elapsed seconds are computed from Unix epoch zero  
And gold and EXP are positive for positive elapsed time.

Automation: `server/src/core/formulas/offline.test.ts`, `client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Scenario 6: High-Level Formula Parity

Given a high-level character uses `level = 1,000,000,000`  
When base rates are computed  
Then server `number` and client `int64` arithmetic both produce `4,000,000,000` for gold and EXP per second.

Automation: `server/src/core/formulas/offline.test.ts`, `client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Scenario 7: Rebirth And Time Bonuses Are Stable

Given the constants are `0.75` efficiency, `0.005` per capped hour, `0.05` per rebirth, and a 12 hour cap  
When rewards are computed for longer capped sessions or higher rebirth count  
Then rewards increase monotonically without changing the documented constants.

Automation: `server/src/core/formulas/offline.test.ts`, `client/Source/IdleProject/Tests/OfflineRewardFormulaTests.cpp`

## Manual Checks

- Capture preview and claim API responses in server logs or curl output for a real character.
- Capture the client offline reward panel before and after claim to confirm preview values and HUD totals update.
- Confirm repeated claim at the same timestamp does not increase local gold or EXP.
