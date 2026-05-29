# Save API

Base path: `/v1/save`

모든 엔드포인트는 `Authorization: Bearer <accessToken>`이 필요하다.

## GET `/`

현재 최신 세이브를 조회한다.

```bash
curl "http://localhost:3000/v1/save?characterId=$CHARACTER_ID" -H "authorization: Bearer $ACCESS"
```

## PUT `/`

서버가 `level`, `rebirthCount`, `maxEquipmentGrade`와 클라우드 확장 필드의 경계를 검증한 뒤 저장한다.
PR #54부터 현재 클라 성장 상태를 반영해 `level`은 1~1000, `maxEquipmentGrade`는
0~6(Mythic)을 허용한다. `totalExp`는 `cumulativeExp(level)`의 1% 또는 1 중 큰 값만큼
하한 오차를 허용하며, 환생/초월 누적 이력 때문에 상한은 두지 않는다.

```json
{
  "characterId": "uuid",
  "version": 1,
  "payload": {
    "level": 10,
    "rebirthCount": 0,
    "maxEquipmentGrade": 7,
    "totalExp": 12345,
    "gold": 1000,
    "lastSeenUnixSec": 1760000000,
    "transcendCount": 1,
    "towerHighestFloor": 25,
    "skillPoints": 12,
    "weeklyBossDamage": 4500,
    "weeklyBossWeekId": "2026-W22",
    "weeklyBossChallengesUsed": 3,
    "weeklyBossClaimedMilestones": 2
  }
}
```

필수 필드는 `level`, `rebirthCount`, `maxEquipmentGrade`이다. payload는 향후 클라 필드를
보존하기 위해 `additionalProperties: true`이며, 서버가 아는 확장 필드
`transcendCount`, `towerHighestFloor`, `skillPoints`는 정수 0 이상이어야 한다.

서버 캐릭터보다 낮은 `level` 또는 `rebirthCount`, 서버 보유량보다 낮은 `gold`, 범위를 벗어난
payload는 `400 SAVE_VALIDATION_FAILED`.

## GET `/history`

최근 세이브 히스토리를 최신순으로 조회한다. `limit` 기본값은 10, 최대 50이다.

```bash
curl "http://localhost:3000/v1/save/history?characterId=$CHARACTER_ID&limit=10" -H "authorization: Bearer $ACCESS"
```

Rate limit: 저장 30/min/User, 조회 120/min/User.
## Client save v4 note

`payload.clientSave.SaveVersion` 4 adds `RuneCodex`, a 54-cell rune collection grid
stored as `FRuneCodexEntry[]` (`RuneType`, `Rarity`, `bUnlocked`). Servers keep this
inside `clientSave` without interpreting it; clients loading saves with version `<4`
must treat the codex as an empty grid.

## Client save v6 note

`payload.clientSave.SaveVersion` 6 adds `RuneSet` to each `FRuneSaveEntry`.
Valid values are `0=None`, `1=Offense`, `2=Bastion`, `3=Vitality`, and
`4=Fortune`. Servers keep the field inside `clientSave` without interpreting it;
clients loading saves with version `<6` must treat missing rune-set data as
`None`, which contributes no set bonus.

## Client save v15 note

`payload.clientSave.SaveVersion` 15 adds weekly boss state:
`WeeklyBossWeekId`, `WeeklyBossDamage`, `WeeklyBossChallengesUsed`, and
`WeeklyBossClaimedMilestones`. Servers also accept the summary fields
`weeklyBossWeekId`, `weeklyBossDamage`, `weeklyBossChallengesUsed`, and
`weeklyBossClaimedMilestones` at the top level of `payload` for cloud progress
validation. `weeklyBossChallengesUsed` is capped at 7, and `weeklyBossWeekId`
must be a 1-32 character string. Clients loading saves with version `<15` must
treat weekly boss state as the current UTC week with zero damage, zero
challenges used, and zero claimed milestones.
