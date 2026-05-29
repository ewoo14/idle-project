# Leaderboard API

Base path: `/v1/leaderboard`

Client contract: `score` is serialized as a JSON string to preserve bigint
precision. UE parses it into `int64` in `ULeaderboardService`; non-ok or
offline responses fall back to an empty top-N list and rank `0` for `/me`.

리더보드는 Redis ZSET을 우선 조회하고, 캐시가 비어 있으면 PostgreSQL에서 조회한다. 시즌은 양의 정수 `season`으로 지정하며, 시즌 생성/마감 운영 정책은 1.0 운영자 기능에서 확장한다.

## GET `/power`

```bash
curl "http://localhost:3000/v1/leaderboard/power?season=1&limit=100"
```

응답:

```json
{ "ok": true, "data": [{ "characterId": "uuid", "score": "1000", "rank": 1 }] }
```

## GET `/rebirth`

```bash
curl "http://localhost:3000/v1/leaderboard/rebirth?season=1&limit=100"
```

`limit` 기본값은 100, 최대 100이다. 조회 한도는 120/min/User 또는 비인증 IP 기준이다.

## GET `/power/me`

```bash
curl "http://localhost:3000/v1/leaderboard/power/me?season=1&characterId=00000000-0000-4000-8000-000000000001"
```

`characterId`는 UUID여야 한다. 요청한 시즌 안에서 해당 캐릭터의 순위를
반환한다. 미등록 캐릭터는 rank `0`, score `"0"`을 반환한다.

```json
{ "ok": true, "data": { "rank": 2, "score": "900" } }
```

## GET `/rebirth/me`

```bash
curl "http://localhost:3000/v1/leaderboard/rebirth/me?season=1&characterId=00000000-0000-4000-8000-000000000001"
```

동일 환생 수는 PostgreSQL `rank()` 기준으로 같은 순위를 공유한다. 미등록
캐릭터는 rank `0`, score `"0"`을 반환한다.

```json
{ "ok": true, "data": { "rank": 1, "score": "3" } }
```
