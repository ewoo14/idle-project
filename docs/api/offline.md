# Offline Rewards API

Base path: `/v1/offline`

All endpoints require `Authorization: Bearer <accessToken>`.

## Formula

Server source of truth: `server/src/core/formulas/offline.ts`.

- Cap: `OFFLINE_CAP_SECONDS = 43200` (12 hours)
- Efficiency: `OFFLINE_EFFICIENCY = 0.75`
- Base rates: `baseGoldPerSec(level) = level * 4`, `baseExpPerSec(level) = level * 4`
- Time bonus: `1 + cappedHours * 0.005 + rebirthCount * 0.05`
- Rewards are rounded with `Math.round`.

V1 keeps gold and EXP base rates identical. The server is the source of truth; the C++ mirror uses `int64` arithmetic for base rates to match server `number` results at high levels.

## GET `/preview`

Returns the currently claimable offline rewards without mutating character state.

```bash
curl "http://localhost:3000/v1/offline/preview?characterId=$CHARACTER_ID" \
  -H "authorization: Bearer $ACCESS"
```

Response:

```json
{
  "ok": true,
  "data": {
    "characterId": "uuid",
    "lastSeenUnixSec": 1779753600,
    "nowUnixSec": 1779757200,
    "rewards": {
      "cappedSeconds": 3600,
      "gold": 10854,
      "exp": 10854,
      "timeBonusMultiplier": 1.005
    }
  }
}
```

## POST `/claim`

Claims offline rewards, increments character `gold` and `total_exp`, and advances `last_seen_at`.

```bash
curl -X POST "http://localhost:3000/v1/offline/claim" \
  -H "authorization: Bearer $ACCESS" \
  -H "content-type: application/json" \
  -d '{"characterId":"uuid"}'
```

Response:

```json
{
  "ok": true,
  "data": {
    "characterId": "uuid",
    "lastSeenUnixSec": 1779753600,
    "nowUnixSec": 1779757200,
    "rewards": {
      "cappedSeconds": 3600,
      "gold": 10854,
      "exp": 10854,
      "timeBonusMultiplier": 1.005
    },
    "totals": {
      "gold": 11954,
      "totalExp": 14054
    }
  }
}
```

Errors:

| Code | Status | Meaning |
| --- | ---: | --- |
| `OFFLINE_CHARACTER_NOT_FOUND` | 404 | Character does not belong to the authenticated user or does not exist. |
| `OFFLINE_REWARD_EMPTY` | 400 | No elapsed offline time is available to claim. |

Rate limit: preview uses read policy, claim uses mutate policy.
