# Season API

Base path: `/v1/season`

All endpoints require `Authorization: Bearer <accessToken>`.

## Rules

- Season tier definitions live in `server/src/core/data/season.ts`.
- V1 has one free track with 10 tiers.
- Quest completion can report season tokens through `POST /tokens`.
- Tier rewards are claimed once and update `characters.gold` or `characters.total_exp`.

## GET `/`

Returns current season tokens and tier claim state.

```bash
curl "http://localhost:3000/v1/season" \
  -H "authorization: Bearer $ACCESS"
```

## POST `/tokens`

Adds server-accepted season tokens after verifying character ownership.

```bash
curl -X POST "http://localhost:3000/v1/season/tokens" \
  -H "authorization: Bearer $ACCESS" \
  -H "content-type: application/json" \
  -d '{"characterId":"uuid","amount":5}'
```

## POST `/tiers/:tier/claim`

Claims one unlocked season tier reward.

```bash
curl -X POST "http://localhost:3000/v1/season/tiers/1/claim" \
  -H "authorization: Bearer $ACCESS" \
  -H "content-type: application/json" \
  -d '{"characterId":"uuid"}'
```

Errors:

| Code | Status | Meaning |
| --- | ---: | --- |
| `SEASON_CHARACTER_NOT_FOUND` | 404 | Character does not belong to the authenticated user or does not exist. |
| `SEASON_TOKEN_AMOUNT_INVALID` | 400 | Token amount is not a positive integer. |
| `SEASON_TIER_NOT_FOUND` | 404 | Season tier definition does not exist. |
| `SEASON_TIER_LOCKED` | 400 | Not enough tokens for this tier. |
| `SEASON_TIER_ALREADY_CLAIMED` | 400 | Tier reward has already been claimed. |

Rate limit: progress uses read policy, token report and claim use mutate policy.
