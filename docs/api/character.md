# Character API

Base path: `/v1/characters`

모든 엔드포인트는 `Authorization: Bearer <accessToken>`이 필요하다.

## POST `/`

PR #2 범위에서는 `classId=1` 전사만 생성한다.

```json
{ "classId": 1 }
```

응답:

```json
{ "ok": true, "data": { "id": "uuid", "classId": 1, "level": 1, "rebirthCount": 0, "stats": { "str": 12, "dex": 6, "int": 3, "luk": 4, "hp": 120, "mp": 30 } } }
```

```bash
curl -X POST http://localhost:3000/v1/characters -H "authorization: Bearer $ACCESS" -H "content-type: application/json" -d '{"classId":1}'
```

## GET `/:id`

사용자 소유 캐릭터만 조회한다.

```bash
curl http://localhost:3000/v1/characters/$CHARACTER_ID -H "authorization: Bearer $ACCESS"
```

없는 캐릭터 또는 타 사용자 캐릭터는 `404 NOT_FOUND`.
