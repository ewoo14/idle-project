# M9 Quests Expansion QA 시나리오

## 범위

- Main, Daily, Weekly 퀘스트 타입과 ch1/ch2 메인 라인 확장.
- 24개 안팎의 클라이언트/서버 퀘스트 정의 parity.
- `KillMonster`, `ClearMap`, `DefeatBoss`, `Enhance`, `Rebirth`,
  `Transcend`, `ClimbTower`, `ReachLevel`, `SpendGold`,
  `RollGearShop`, `FeedPet`, `ClaimOffline` 목표 진행 훅.
- 일일 UTC 날짜 리셋과 주간 ISO week 리셋.
- 보상 수령 1회성, 골드/EXP 지급, 선행 퀘스트 잠금/해제.
- 저장 #53, 클라우드 #54 를 통한 퀘스트 상태 영속.
- Quest ko/en 로컬라이즈와 HUD Main/Daily/Weekly 섹션 표시.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 4, 5 |
| 엣지 | 6, 7, 8 |
| 회귀 | 4, 5, 8, 9 |

## 시나리오 1: 퀘스트 정의가 Main, Daily, Weekly 로 확장된다

Given 클라이언트 `BuildDefaultDefinitions` 와 서버
`questDefinitions` 가 퀘스트 정의를 가진다.

And 정의에는 `main`, `daily`, `weekly` 타입이 모두 포함된다.

When 퀘스트 카탈로그를 초기화한다.

Then 총 정의 수는 24개 안팎이며 Main/Daily/Weekly 가 모두 존재한다.

And 각 정의는 `questId`, `type`, `objective`, `targetCount`, `reward`,
`prerequisiteQuestId`, `chapterMapId` parity 항목을 가진다.

And 클라이언트와 서버의 각 parity 항목 값이 동일하다.

Automation: `IdleProject.GameCore.QuestService.DefinitionParity`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 2: ch1 에서 ch2 까지 메인 선행 체인이 유지된다

Given 새 캐릭터는 첫 번째 ch1 메인 퀘스트만 해금된 상태다.

And ch2 첫 퀘스트는 마지막 ch1 보스 퀘스트를 선행 조건으로 가진다.

When ch1 메인 퀘스트를 순서대로 진행하고 보상을 수령한다.

Then 다음 메인 퀘스트만 순차적으로 해금된다.

And ch2 퀘스트는 ch1 마지막 퀘스트 수령 전까지 잠겨 있다.

And ch2 퀘스트의 `chapterMapId` 는 `2-1` 부터 `2-5` 까지 보존된다.

Automation: `IdleProject.GameCore.QuestService.ChapterTwoPrerequisiteChain`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 3: 신규 목표 훅이 실제 게임 경로에서 진행된다

Given 활성 Main/Daily/Weekly 퀘스트가 신규 목표를 기다린다.

When 플레이어가 처치, 맵 클리어, 보스 처치, 강화, 환생, 초월, 탑 등반,
레벨업, 골드 소비, 상점 뽑기, 펫 먹이기, 오프라인 보상 수령을 수행한다.

Then 각 경로는 대응하는 `RecordQuestProgress(objective, amount)` 를
호출한다.

And 목표가 같은 활성 퀘스트만 증가하고 다른 목표는 증가하지 않는다.

And 목표 진행은 업적 #55 `RecordMetric` 과 공존해도 중복 보상을 만들지
않는다.

Automation: `IdleProject.GameCore.IdleGameInstance.ExpandedQuestHooks`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 4: 일일 UTC 리셋과 주간 ISO 리셋이 독립적으로 동작한다

Given 일일 퀘스트는 `dailyResetDate`, 주간 퀘스트는 `weeklyResetId` 를
저장한다.

And 현재 서버 시간이 `2026-05-26T00:00:00.000Z` 이다.

When UTC 날짜가 바뀌고 ISO week 는 그대로인 상태로 목록을 조회한다.

Then 일일 퀘스트 진행, 완료, 수령 상태만 초기화된다.

And 주간 퀘스트 진행, 완료, 수령 상태는 유지된다.

When ISO week 가 `2026-W22` 에서 `2026-W23` 으로 바뀐다.

Then 주간 퀘스트 진행, 완료, 수령 상태가 초기화된다.

Automation: `IdleProject.GameCore.QuestService.DailyReset`,
`IdleProject.GameCore.QuestService.ExpandedUnlockWeeklyReset`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 5: 보상 수령은 1회만 가능하고 다음 메인 퀘스트를 해금한다

Given 메인 퀘스트가 목표 수량까지 완료되어 있고 아직 수령되지 않았다.

When 플레이어가 보상을 수령한다.

Then 골드와 EXP 보상이 정확히 한 번 지급된다.

And 퀘스트 상태는 `claimed=true` 로 저장된다.

And 다음 메인 퀘스트가 선행 조건을 만족해 활성 목록에 포함된다.

When 같은 퀘스트 보상을 다시 수령하려고 한다.

Then 서버와 클라이언트는 이미 수령된 보상을 거부하고 재지급하지 않는다.

Automation: `IdleProject.GameCore.IdleGameInstance.QuestRewardAndHooks`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 6: 잠긴 메인 퀘스트와 잘못된 진행 요청은 거부된다

Given `main_ch1_002` 의 선행 퀘스트가 아직 수령되지 않았다.

When `main_ch1_002` 에 직접 진행량을 추가한다.

Then 진행 요청은 `QUEST_LOCKED` 로 실패한다.

And 저장된 진행량과 보상 상태는 변하지 않는다.

When 존재하지 않는 `questId` 또는 미지원 목표 타입으로 진행을 요청한다.

Then 요청은 검증 오류 또는 `QUEST_NOT_FOUND` 로 실패한다.

Automation: `server/src/modules/quest/quest.service.test.ts`,
`IdleProject.GameCore.QuestService.ProgressClaimUnlock`

## 시나리오 7: 0/음수/초과 진행량이 안전하게 처리된다

Given 활성 퀘스트의 `targetCount` 가 10 이고 현재 진행량은 9 다.

When 진행량 0 또는 음수를 기록한다.

Then 진행량은 감소하거나 완료 처리되지 않는다.

When 진행량 999 를 기록한다.

Then 저장 진행량은 목표치를 초과해도 보상 수령은 1회만 가능하다.

And UI 진행률은 완료 상태를 안정적으로 표시한다.

Automation: `IdleProject.GameCore.QuestService.ProgressClaimUnlock`,
`server/src/modules/quest/quest.service.test.ts`

## 시나리오 8: 저장과 클라우드 복원이 퀘스트 상태를 보존한다

Given 플레이어가 Main, Daily, Weekly 퀘스트를 각각 진행했다.

And 주간 퀘스트는 `weeklyResetId=2026-W22` 를 가진다.

When #53 로컬 저장이 quest 상태를 캡처하고 #54 클라우드 업로드가
동일 payload 를 전송한다.

Then `questId`, `progress`, `completed`, `claimed`, `dailyResetDate`,
`weeklyResetId` 가 payload 에 포함된다.

And 다른 세션에서 복원하면 진행량, 수령 상태, 리셋 앵커가 동일하다.

And 다음 ISO week 에 진입하면 복원된 주간 퀘스트도 정상 리셋된다.

Automation: `IdleProject.GameCore.QuestService.WeeklySaveRoundTrip`,
`IdleProject.GameCore.SaveSystem.QuestSeasonRoundTrip`,
`server/src/modules/save/save.test.ts`

## 시나리오 9: 로컬라이즈와 HUD 섹션이 Main/Daily/Weekly 를 표시한다

Given Quest ko/en CSV 에 모든 퀘스트 제목과 설명 키가 존재한다.

And HUD 퀘스트 로그는 `FIdleHUDQuestLogViewModel` 을 사용한다.

When 퀘스트 로그를 연다.

Then Main, Daily, Weekly 섹션이 모두 표시된다.

And 각 행은 로컬라이즈된 제목, 설명, 진행량, 보상, 수령 상태를 표시한다.

And 다수 항목이 있어도 스크롤/섹션 배치가 기존 HUD 영역과 겹치지 않는다.

Automation: `IdleProject.UI.HUD.QuestLogViewModel`,
`IdleProject.Localization.CsvIntegrity`, Playwright/PIE visual smoke.

## 수동 재현 노트

- 정의 확인: PIE 시작 후 퀘스트 로그를 열고 Main/Daily/Weekly 섹션,
  ch1/ch2 메인 라인, 보상/목표 표시를 캡처한다.
- 진행 확인: 몬스터 처치, 보스 처치, 맵 클리어, 강화, 레벨업, 골드 소비,
  상점 뽑기, 펫 먹이기, 오프라인 보상 수령을 순서대로 수행한다.
- 리셋 확인: 테스트 시간 또는 저장 payload 를 조정해 UTC 날짜와 ISO week
  경계를 각각 넘긴 뒤 일일/주간 상태만 초기화되는지 캡처한다.
- 저장 확인: Main/Daily/Weekly 진행 후 로컬 저장과 클라우드 업로드를
  수행하고 새 세션에서 동일 상태로 복원되는지 확인한다.
- 회귀 확인: 잠긴 메인 퀘스트 직접 진행, 중복 수령, 0/음수 진행량,
  초과 진행량을 시도하고 오류 또는 안전한 상태를 기록한다.

## 기대 증거

- UE Automation stdout 에 `IdleProject.GameCore.QuestService.*`,
  `IdleProject.GameCore.IdleGameInstance.ExpandedQuestHooks`,
  `IdleProject.UI.HUD.QuestLogViewModel` 이 `Result={Success}` 로 표시된다.
- 서버 vitest stdout 에 `server/src/modules/quest/quest.service.test.ts` 가
  정의 parity, enum, 리셋, 진행, 수령, 잠금 검증을 통과했다고 표시된다.
- CsvIntegrity stdout 은 Quest ko/en 키 수와 필수 열 정합이 통과했음을
  표시한다.
- 수동 PIE 증거는 Main/Daily/Weekly HUD 캡처, 리셋 전후 payload,
  중복 수령 거부 로그, 클라우드 복원 후 진행량 비교를 포함한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
Push-Location server
npm run build
npm test -- src/modules/quest/quest.service.test.ts
npm test -- src/modules/save/save.test.ts
Pop-Location

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.GameCore.QuestService; Automation RunTests IdleProject.GameCore.IdleGameInstance.ExpandedQuestHooks; Quit' `
  -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.QuestLogViewModel; Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
