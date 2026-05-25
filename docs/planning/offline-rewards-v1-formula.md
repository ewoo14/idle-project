# Offline Rewards V1 Formula

Server source of truth: `server/src/core/formulas/offline.ts`.
Client mirror: `client/Source/IdleProject/GameCore/OfflineRewardFormula.h/.cpp`.

## Constants

| Name | Value | Note |
| --- | ---: | --- |
| `OFFLINE_CAP_SECONDS` | `43200` | 12 hour cap |
| `OFFLINE_EFFICIENCY` | `0.75` | Active baseline efficiency |
| `OFFLINE_TIME_BONUS_PER_HOUR` | `0.005` | Linear capped-session bonus |
| `OFFLINE_REBIRTH_BONUS` | `0.05` | Per rebirth count |

## Rates

```text
baseGoldPerSec(level) = level * 4
baseExpPerSec(level) = level * 4
timeBonusMultiplier = 1 + cappedHours * 0.005 + rebirthCount * 0.05
gold = round(baseGoldPerSec(level) * cappedSeconds * 0.75 * timeBonusMultiplier)
exp = round(baseExpPerSec(level) * cappedSeconds * 0.75 * timeBonusMultiplier)
```

The level 1 active baseline mirrors M1's approximate 14,400 gold/EXP per hour before offline efficiency and time/rebirth bonuses.

## Verification

- Server: `npm test -- offline.test.ts`
- Client: `IdleProject.GameCore.OfflineRewardFormula.ComputeOfflineRewards`
- Claim flow: `IdleProject.GameCore.IdleGameInstance.ClaimOfflineRewards`
