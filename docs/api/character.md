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
{
  "ok": true,
  "data": {
    "id": "uuid",
    "classId": 1,
    "level": 1,
    "rebirthCount": 0,
    "stats": {
      "primary": {
        "str": 12,
        "dex": 6,
        "int": 3,
        "wis": 3,
        "con": 10,
        "luk": 4
      },
      "derived": {
        "hp": 120,
        "mp": 30,
        "physAtk": 24,
        "magicAtk": 8,
        "physDef": 17,
        "magicDef": 5,
        "atkSpeed": 1,
        "moveSpeed": 1,
        "critRate": 0.008,
        "critDmg": 1.504,
        "dodge": 0.013,
        "accuracy": 0.762
      }
    }
  }
}
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

PR #4~#10 단계: 클라이언트 UI 는 ClassDB 의 Mvp=1 (전사) 만 노출. 다직업 활성화는 PR #11.
