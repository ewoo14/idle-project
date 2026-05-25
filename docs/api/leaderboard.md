# Leaderboard API

Base path: `/v1/leaderboard`

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
