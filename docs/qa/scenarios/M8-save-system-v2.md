# M8 Save System V2 QA 시나리오

## 범위

- `UIdleSaveGame` 버전 2 페이로드(Inventory/Skill/Quest/Season).
- `UIdleGameInstance::CaptureToSave` / `ApplyFromSave` V2 라운드트립.
- 장비·스킬 랭크·퀘스트 상태·시즌 보상의 재시작 영속.
- V1 세이브 호환(신규 V2 서브시스템은 기본값).
- 손상 V2 페이로드 새니타이즈 및 graceful no-op.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 4 |
| 엣지 | 5, 6 |
| 회귀 | 1, 3, 5, 6 |

## 시나리오 1: 전체 V2 재시작 시 장비·진행 복원

Given 플레이어가 희귀도·강화 단계·affix 스탯·세트 소속을 가진
장비 2~3개를 보유한다.

And 그중 최소 한 개가 유효한 장비 슬롯에 장착되어 있다.

And 플레이어가 기본값이 아닌 스킬 랭크, 미사용 스킬 포인트,
진행 중 퀘스트, 수령한 퀘스트 보상, 시즌 토큰, 수령한 시즌
티어를 가진다.

When `SaveProgress()`가 V2 `IdleSave` 슬롯에 저장하고 게임을
재시작한다.

Then `LoadProgress()`가 `ItemId`·슬롯·희귀도·세트·강화·표시명·affix
값을 손실 없이 인벤토리 행을 복원한다.

And 장착 슬롯 인덱스가 복원된 동일 아이템 행을 가리킨다.

And 스킬 랭크·스킬 포인트·퀘스트 진행·퀘스트 수령 플래그·시즌
토큰·수령 티어가 저장 상태와 일치한다.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
`IdleProject.GameCore.SaveSystem.InventoryStateRoundTrip`,
`IdleProject.GameCore.SaveSystem.SkillRankStateRoundTrip`,
`IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip`

## 시나리오 2: 스킬 저장 시 랭크·미사용 포인트 복원

Given 플레이어가 0이 아닌 랭크를 가진 여러 알려진 스킬 ID를
보유한다.

And 플레이어가 0이 아닌 `SkillPoints` 값을 가진다.

When `CaptureToSave()`가 스킬 상태를 저장하고 `ApplyFromSave()`가
새 게임 인스턴스에 복원한다.

Then 각 알려진 스킬 랭크가 저장된 랭크로 복원된다.

And 가용 스킬 포인트가 정확히 한 번 복원된다.

And 쿨다운·게이지·런타임 전용 버프 상태는 이 V2 페이로드에
요구되지 않는다(휘발성).

Automation: `IdleProject.GameCore.SaveSystem.SkillRankStateRoundTrip`,
`IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`

## 시나리오 3: 퀘스트 저장 시 진행·수령 상태·일일 리셋 보존

Given 일일/챕터/반복 퀘스트가 섞인 진행도 값을 가진다.

And 일부 퀘스트는 완료, 일부는 수령, 일부는 진행 중이다.

And 세이브가 현재 퀘스트 일일 리셋 날짜를 포함한다.

When 퀘스트 상태가 capture·저장·복원된다.

Then 알려진 퀘스트 ID가 진행도·완료·수령 상태·퀘스트 타입·일일
리셋 날짜를 보존한다.

And 일일 리셋 경계 이후 로드 시 일일 퀘스트 리셋 로직이 비일일
퀘스트 진행을 손상시키지 않고 재적용된다.

Automation: `IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip`,
`IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`

## 시나리오 4: 시즌 저장 시 토큰·수령 티어 복원

Given 현재 시즌 ID가 활성 상태다.

And 플레이어가 시즌 토큰을 획득하고 여러 보상 티어를 수령했다.

When 시즌 상태가 capture·복원된다.

Then `SeasonId`·`SeasonTokens`·`SeasonClaimedTiers`가 저장 상태와
일치한다.

And 재시작 후 이전에 수령한 티어를 다시 수령할 수 없다.

Automation: `IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip`,
`IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`

## 시나리오 5: V1 세이브 마이그레이션 시 신규 V2 시스템 기본값 유지

Given 레거시 V1 세이브가 골드·레벨·스테이지·탑·펫 등 코어 진행을
포함한다.

And V1 페이로드에 Inventory/Skill/Quest/Season V2 필드가 없다.

When V1 세이브를 V2 세이브 시스템이 로드한다.

Then V1 코어 진행이 정상 복원된다.

And 인벤토리는 빈 상태로 시작하고, 스킬 랭크/포인트는 0 또는
기본값, 퀘스트는 기본값에서 초기화, 시즌 토큰/수령 티어는 0
또는 빈 기본값으로 시작한다.

And 다음 정상 저장은 `SaveVersion` 2 로 기록한다.

Automation: `IdleProject.GameCore.SaveSystem.SaveGameDefaults`,
`IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
`IdleProject.GameCore.SaveSystem.InvalidLoadIsNoOp`

## 시나리오 6: 손상 V2 페이로드 새니타이즈

Given V2 세이브 페이로드가 잘못된 장비 인덱스, 잘못된 장비 슬롯
매핑, 미지의 아이템 ID, 미지의 스킬 ID, 음수 스킬 포인트, 미지의
퀘스트 ID, 목표 초과 퀘스트 진행, 불일치 시즌 ID, 미지의 수령
티어를 포함한다.

When `ApplyFromSave()`가 손상 페이로드를 복원한다.

Then 무효 인벤토리 행과 장비 매핑이 크래시 없이 무시되거나
클램프된다.

And 미지의 스킬 ID는 무시되고 유효 랭크는 허용 한계로
클램프된다.

And 퀘스트 진행은 `0..TargetCount`로 클램프되고, 미지의 퀘스트는
무시되며, 다른 시즌 ID의 시즌 페이로드는 토큰/티어를 복원하지
않는다.

Automation: `IdleProject.GameCore.SaveSystem.MalformedV2PayloadIsSanitized`,
`IdleProject.GameCore.SaveSystem.RestoreSanitizesServiceState`,
`IdleProject.GameCore.SaveSystem.InvalidLoadIsNoOp`

## 수동 재현 노트

- 정상 흐름: PIE 시작 → 아이템 2~3개 획득/주입 → 한 개 장착 →
  스킬 최소 1개 랭크업 → 퀘스트 진행·수령 → 시즌 티어 수령 →
  저장 → PIE 종료 → 재시작 → 복원 값 비교.
- 엣지 흐름: 백엔드 정지 상태로 로드 → Inventory/Skill/Quest/Season
  복원 경로가 로컬이므로 로컬 V2 저장/로드가 정상 사용 가능함을
  확인.
- 회귀 흐름: V1 세이브 픽스처 또는 강제 `SaveVersion == 1` 페이로드
  로드 → V1 코어 진행 유지 + V2 시스템 기본값 시작 확인.

## 기대 증거

- Automation stdout 에 `IdleProject.GameCore.SaveSystem` 의 모든 나열
  테스트가 `Result={Success}` 보고.
- 수동 PIE 검증 시 인벤토리·장비·스킬 랭크/포인트·퀘스트·시즌
  패널의 전/후 스크린샷 캡처.
- 손상 페이로드 검증 시 크래시 없음 + 새니타이즈된 복원 상태를
  보이는 Automation 로그 캡처.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.GameCore.SaveSystem; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
