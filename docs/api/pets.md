# Pets API

Base path: `/v1/pets`

All endpoints require `Authorization: Bearer <accessToken>`.

## Rules

- Pet definitions live in `server/src/core/data/pets.ts`.
- V1 grants both pets by default: `dog` gives `gold +20%`, `cat` gives `drop +15%`.
- Only one pet can be equipped at a time.

## GET `/`

Returns owned pets, equipped state, and the active bonus.

```bash
curl "http://localhost:3000/v1/pets" \
  -H "authorization: Bearer $ACCESS"
```

## POST `/:id/equip`

Equips one owned pet.

```bash
curl -X POST "http://localhost:3000/v1/pets/dog/equip" \
  -H "authorization: Bearer $ACCESS"
```

Errors:

| Code | Status | Meaning |
| --- | ---: | --- |
| `PET_NOT_FOUND` | 404 | Pet definition does not exist. |
| `PET_NOT_OWNED` | 400 | Pet is not owned by the user. |

Rate limit: list uses read policy, equip uses mutate policy.
