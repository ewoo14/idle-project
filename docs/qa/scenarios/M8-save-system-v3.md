# M8 Save System V3 QA 시나리오

## 범위

- 로컬 `USaveGame` 과 서버 `GET /v1/save/`, `PUT /v1/save/` 동기화.
- `EnsureCharacter`, `DownloadSave`, `UploadSave` 인증/캐릭터 연결 흐름.
- `FCloudSaveMergePolicy` 충돌 해소와 로컬 세이브 회귀 방지.
- 서버 save schema 검증 현행화(Mythic grade6, 확장 필드, 퇴행 거부).
- HUD cloud sync 상태(`Syncing`, `Synced`, `Offline`, `Conflict`) 표시.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 7 |
| 엣지 | 4, 5, 6 |
| 회귀 | 1, 3, 5, 6, 7 |

## 시나리오 1: 클라우드 업로드가 로컬 autosave 진행을 보존

Given 플레이어가 로그인했고 `EnsureCharacter` 가 캐릭터 ID를 확보했다.

And 로컬 세이브가 골드, 레벨, 환생 수, 초월 수, 탑 최고층, 스킬
포인트, Mythic 장비 grade6 진행을 포함한다.

When autosave 또는 종료 저장이 로컬 `IdleSave` 슬롯을 갱신한다.

And `UploadSave` 가 `PUT /v1/save/` 로 version 이 증가한 payload 를
전송한다.

Then 서버 요청은 `characterId`, `version`, `payload.level`,
`payload.rebirthCount`, `payload.maxEquipmentGrade` 를 포함한다.

And `gold`, `totalExp`, `lastSeenUnixSec`, `transcendCount`,
`towerHighestFloor`, `skillPoints` 가 손실 없이 직렬화된다.

And 업로드 성공 후 로컬 version 이 다음 업로드 기준으로 보존된다.

Automation: `IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip`,
`server/src/modules/save/save.test.ts`

## 시나리오 2: 다른 기기 다운로드 후 로컬 복원

Given 기기 A가 서버에 level/rebirth/gold 와 확장 필드를 포함한
클라우드 세이브를 업로드했다.

And 기기 B는 같은 계정으로 로그인했지만 로컬 `IdleSave` 가 없거나
더 낮은 진행도를 가진다.

When 기기 B의 시작 흐름이 `DownloadSave` 로 `GET /v1/save/` 를
호출한다.

Then 수신 payload 가 로컬 `UIdleSaveGame` 으로 매핑된다.

And `LoadProgress()` 이후 level, rebirth, gold, totalExp, transcend,
tower, skillPoints 가 서버 상태와 일치한다.

And 다음 autosave 는 다운로드된 version 보다 큰 version 으로
업로드를 시도한다.

Automation: `IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip`,
manual PIE smoke with backend running.

## 시나리오 3: 서버 진행도가 높으면 서버 세이브 채택

Given 로컬 세이브가 level 120, rebirth 1, gold 10,000 을 가진다.

And 서버 세이브가 rebirth, level, gold 우선순위 중 하나에서 로컬보다
높은 진행도를 가진다.

When 시작 시 `DownloadSave` 가 완료되고 `FCloudSaveMergePolicy` 가
로컬 snapshot 과 서버 snapshot 을 비교한다.

Then 결정은 `AdoptServer` 이다.

And 로컬 진행은 서버 payload 값으로 갱신된다.

And 서버 채택 직후 불필요한 낮은 로컬 payload 업로드가 발생하지
않는다.

Automation: `IdleProject.GameCore.SaveSystem.CloudMergePolicy`,
`IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip`

## 시나리오 4: 로컬 진행도가 높으면 로컬 세이브 유지

Given 로컬 세이브가 서버보다 rebirth 또는 level 이 높다.

And 서버 세이브는 오래된 기기에서 업로드된 낮은 진행도다.

When 시작 시 `FCloudSaveMergePolicy` 가 두 snapshot 을 비교한다.

Then 결정은 `KeepLocal` 이다.

And 로컬 진행은 서버 payload 로 덮어써지지 않는다.

And 다음 upload 는 서버 퇴행 거부 정책을 통과하는 로컬 payload 를
전송한다.

Automation: `IdleProject.GameCore.SaveSystem.CloudMergePolicy`,
`server/src/modules/save/save.test.ts`

## 시나리오 5: 서버 미응답 시 로컬 전용 폴백

Given 백엔드 또는 Docker Compose 의 server/profile 이 실행 중이
아니다.

And 플레이어가 기존 로컬 세이브를 보유한다.

When `EnsureCharacter`, `DownloadSave`, 또는 `UploadSave` 요청이
타임아웃되거나 실패한다.

Then 게임 진행, autosave, load, shutdown save 는 로컬 `USaveGame`
경로로 계속 동작한다.

And #52/#53 로컬 저장 및 V2 Inventory/Skill/Quest/Season 복원에
회귀가 없다.

And HUD 는 `Offline` 상태를 표시하고 정상 플레이를 막지 않는다.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
`IdleProject.UI.HUD.CloudSyncViewModel`, manual PIE smoke with backend stopped.

## 시나리오 6: 서버 schema 가 현재 클라이언트 진행을 거부하지 않음

Given payload 가 `maxEquipmentGrade: 6` 인 Mythic 장비 진행을 포함한다.

And payload 가 높은 level, rebirth, gold, totalExp, transcendCount,
towerHighestFloor, skillPoints 를 포함한다.

When 서버 `putSaveSchema` 와 `validateSavePayload` 가 payload 를
검증한다.

Then Mythic grade6 은 허용된다.

And 확장 필드는 0 이상의 정수로 허용된다.

And level/rebirth/gold 퇴행 payload 는 기존처럼 거부된다.

Automation: `server/src/modules/save/save.test.ts`

## 시나리오 7: 동기화 상태 HUD 피드백

Given HUD 가 `UIdleGameInstance::OnCloudSyncStateChanged` 를 구독한다.

When cloud sync 상태가 `Syncing`, `Synced`, `Offline`, `Conflict` 로
전환된다.

Then 각 상태는 ko/en 로컬라이즈된 라벨과 의도한 강조색을 가진다.

And `Syncing` 과 `Synced` 는 transient fade 규칙을 따른다.

And `Offline` 과 `Conflict` 는 사용자가 진행 가능 여부를 오해하지
않도록 오류 계열 피드백으로 표시된다.

Automation: `IdleProject.UI.HUD.CloudSyncViewModel`,
`IdleProject.Localization.CsvIntegrity`

## 수동 재현 노트

- 정상 흐름: Docker Compose 로 postgres/redis/server 를 실행 →
  PIE 로그인 → 캐릭터 ID 확보 → 진행도 변경 → autosave/종료 →
  서버 `GET /v1/save/` 응답의 version/payload 확인.
- 교차 기기 흐름: 동일 계정으로 새 로컬 세이브 환경에서 시작 →
  `DownloadSave` 로 서버 진행 복원 → 한 번 더 진행 후 upload version
  증가 확인.
- 충돌 흐름: 서버 payload 를 로컬보다 높은/낮은 rebirth, level, gold
  조합으로 준비 → `AdoptServer` 와 `KeepLocal` 양쪽 결과 확인.
- 오프라인 흐름: server 중지 또는 네트워크 실패 주입 →
  저장/로드/플레이 지속, HUD `Offline` 표시, V2 상태 복원 확인.

## 기대 증거

- UE Automation stdout 에 `CloudMergePolicy`, `CloudPayloadRoundTrip`,
  `CloudSyncViewModel` 테스트가 `Result={Success}` 로 보고된다.
- 서버 vitest stdout 에 `server/src/modules/save/save.test.ts` 가
  Mythic grade6, 확장 필드, 퇴행 거부 회귀를 통과했다고 표시된다.
- 수동 PIE 검증 시 upload/download 요청 로그, 로컬/서버 payload 비교,
  HUD 상태별 스크린샷을 캡처한다.
- 오프라인 검증 시 백엔드 실패 로그와 로컬 저장 성공 로그를 함께
  캡처한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
docker compose -f infra/docker-compose.yml up -d postgres redis

Push-Location server
npm run build
npm test -- save.test.ts
Pop-Location

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.GameCore.SaveSystem.CloudMergePolicy; Automation RunTests IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip; Automation RunTests IdleProject.UI.HUD.CloudSyncViewModel; Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
