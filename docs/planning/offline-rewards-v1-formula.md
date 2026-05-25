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

`baseGoldPerSec` and `baseExpPerSec` are the shared level-scaling base rates. They are intentionally identical in V1 so gold and EXP stay in parity before later content-specific reward tables. Server code uses JavaScript `number`; the C++ mirror casts level to `int64` before multiplication so high-level rates do not overflow an `int32` intermediate.

The level 1 active baseline mirrors M1's approximate 14,400 gold/EXP per hour before offline efficiency and time/rebirth bonuses.

## Verification

- Server formula: `npm test -- offline.test.ts`
- Server offline module: `npm test -- offline.service.test.ts`
- Client formula mirror: `IdleProject.GameCore.OfflineRewardFormula.ComputeOfflineRewards`
- Claim flow: `IdleProject.GameCore.IdleGameInstance.ClaimOfflineRewards`
