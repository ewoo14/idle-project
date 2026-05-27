# Quest API

Base path: `/v1/quests`

All endpoints require `Authorization: Bearer <accessToken>`.

## Rules

- Quest definitions live in `server/src/core/data/quests.ts`.
- Main quests unlock when their `prerequisiteQuestId` has been claimed.
- Daily quests reset lazily on first quest read or write after the UTC date changes.
- Weekly quests reset lazily on first quest read or write after the UTC ISO week changes.
- Rewards are server-authoritative and update `characters.gold` and `characters.total_exp`.
- Quest types are `main`, `daily`, and `weekly`.
- Objective ids are `kill_monster`, `clear_map`, `claim_offline`, `enhance`,
  `defeat_boss`, `rebirth`, `transcend`, `climb_tower`, `reach_level`,
  `spend_gold`, `roll_gear_shop`, and `feed_pet`.

## GET `/`

Returns active quests for a character. Locked main quests are omitted. Daily
and weekly quests are always active in their current reset window.

```bash
curl "http://localhost:3000/v1/quests?characterId=$CHARACTER_ID" \
  -H "authorization: Bearer $ACCESS"
```

## POST `/:id/progress`

Adds server-accepted progress to an active quest.

```bash
curl -X POST "http://localhost:3000/v1/quests/main_ch1_001/progress" \
  -H "authorization: Bearer $ACCESS" \
  -H "content-type: application/json" \
  -d '{"characterId":"uuid","amount":1}'
```

## POST `/:id/claim`

Claims a completed quest reward once.

```bash
curl -X POST "http://localhost:3000/v1/quests/main_ch1_001/claim" \
  -H "authorization: Bearer $ACCESS" \
  -H "content-type: application/json" \
  -d '{"characterId":"uuid"}'
```

Response includes the claimed quest state, updated character totals, and newly unlocked main quest IDs.

Quest objects include `dailyResetDate` for daily quests and `weeklyResetId`
for weekly quests. Non-matching quest types return `null` for those reset
fields.

Errors:

| Code | Status | Meaning |
| --- | ---: | --- |
| `QUEST_CHARACTER_NOT_FOUND` | 404 | Character does not belong to the authenticated user or does not exist. |
| `QUEST_NOT_FOUND` | 404 | Quest definition does not exist. |
| `QUEST_LOCKED` | 400 | Main quest prerequisite has not been claimed. |
| `QUEST_NOT_COMPLETED` | 400 | Quest reward cannot be claimed before completion. |
| `QUEST_ALREADY_CLAIMED` | 400 | Quest reward has already been claimed. |
| `QUEST_PROGRESS_AMOUNT_INVALID` | 400 | Progress amount is not a positive integer. |

Rate limit: list uses read policy, progress and claim use mutate policy.
