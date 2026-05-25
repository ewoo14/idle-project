# Save API

Base path: `/v1/save`

모든 엔드포인트는 `Authorization: Bearer <accessToken>`이 필요하다.

## GET `/`

현재 최신 세이브를 조회한다.

```bash
curl "http://localhost:3000/v1/save?characterId=$CHARACTER_ID" -H "authorization: Bearer $ACCESS"
```

## PUT `/`

서버가 `level`, `rebirthCount`, `maxEquipmentGrade`의 경계를 검증한 뒤 저장한다. PR #2에서는 정밀 공식 검증 대신 boundary 검증을 수행한다.

```json
{
  "characterId": "uuid",
  "version": 1,
  "payload": {
    "level": 10,
    "rebirthCount": 0,
    "maxEquipmentGrade": 2
  }
}
```

조작된 payload는 `400 VALIDATION_ERROR`.

## GET `/history`

최근 세이브 히스토리를 최신순으로 조회한다. `limit` 기본값은 10, 최대 50이다.

```bash
curl "http://localhost:3000/v1/save/history?characterId=$CHARACTER_ID&limit=10" -H "authorization: Bearer $ACCESS"
```

Rate limit: 저장 30/min/User, 조회 120/min/User.
