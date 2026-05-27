# M9 Achievements Collection QA 시나리오

## 범위

- 9개 업적 카테고리와 22개 정의의 트랙 breadth 및 클라/서버 parity.
- `UAchievementService` 누적/최대 지표 기록, 티어 달성, 포인트 산정.
- `FAchievementFormula` 와 서버 `achievement.ts` 의 임계값/배수 공식.
- `RefreshDerivedStats` 합성 배수
  `Transcend * TowerMilestone * Achievement`.
- 업적 상태의 `UIdleSaveGame` 저장, #54 `clientSave` 클라우드 동기화.
- HUD 업적 패널, 카테고리 요약, 달성 토스트, ko/en 로컬라이즈.
- v1/v2 세이브 호환과 0 포인트 회귀 안전.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 5 |
| 엣지 | 4, 6 |
| 회귀 | 2, 4, 5, 6 |

## 시나리오 1: 업적 카탈로그가 9개 카테고리와 22개 정의를 제공

Given 업적 정의가 클라이언트 `FAchievementFormula` 와 서버
`achievement.ts` 에 등록되어 있다.

And 카테고리는 Combat, Progression, Gear, Economy, Skill, Pet, Quest,
Collection, Misc 를 포함한다.

When 업적 카탈로그를 초기화한다.

Then 정의 수는 22개 이상이고 카테고리 수는 9개다.

And 각 정의는 ID, 카테고리, 지표, 지표 모드, base threshold, growth,
points per tier, 표시명을 가진다.

And 누적 지표와 최고값 지표가 모두 존재한다.

Automation: `IdleProject.GameCore.Achievement.FormulaCatalog`,
`server/src/core/formulas/achievement.test.ts`

## 시나리오 2: 업적 티어 달성으로 CP가 단조 증가

Given 새 게임 인스턴스의 업적 포인트가 0 이고 업적 배수는 `x1.00` 이다.

And 현재 전투력은 #49 `ComputeCombatPower` 로 계산된다.

When 몬스터 처치, 장비 강화, 퀘스트 완료, 레벨 도달, 환생, 초월,
탑 최고층, 펫 먹이기, 오프라인 보상 수령 이벤트가 업적 지표를 기록한다.

Then 달성 임계값을 넘은 트랙의 티어와 총 포인트가 증가한다.

And `GetAchievementStatMultiplier()` 는 포인트 증가를 반영한다.

And `RefreshDerivedStats` 이후 HP, 물공, 마공, 물방, 마방이 증가하여
CP가 이전보다 낮아지지 않는다.

And `AtkSpeed`, `CritRate` 같은 rate 계열 스탯은 업적 배수로 변하지
않는다.

Automation: `IdleProject.GameCore.Achievement.GameInstanceHooks`,
`IdleProject.Character.Stats.AchievementMultiplier`

## 시나리오 3: 합성 배수 순서가 Transcend, Tower, Achievement 를 보존

Given 캐릭터가 초월 카운트, 탑 최고층, 업적 포인트를 모두 가진다.

And 업적 0 포인트의 배수는 `x1.00` 이다.

When `AIdleCharacter::RefreshDerivedStats` 가 파생 스탯을 재계산한다.

Then 최종 합성 배수는 `Transcend * TowerMilestone * Achievement` 이다.

And HP, 물공, 마공, 물방, 마방은 세 배수를 모두 곱한 값과 일치한다.

And 업적 포인트가 0 인 세이브는 기존 초월/탑 합성 결과와 동일하다.

Automation: `IdleProject.Character.Stats.TowerMilestoneMultiplier`,
`IdleProject.Character.Stats.AchievementMultiplier`

## 시나리오 4: 소프트캡 배수 공식이 클라/서버와 밸런스 리포트에서 일치

Given 총 업적 포인트가 0, 3, 100, 125, 250, 500 인 샘플을 준비한다.

When 클라이언트 `FAchievementFormula::GetStatMultiplier` 와 서버
`getAchievementStatMultiplier` 를 실행한다.

Then 0 포인트는 `x1.00`, 3 포인트는 `x1.03`, 100 포인트는 `x2.00` 이다.

And 125 포인트는 소프트캡 적용 후 약 `x2.1967347` 이다.

And 250 포인트는 기존 선형 `x3.50` 이 아니라 약 `x2.475` 이다.

And 500 포인트는 유효 상한에 가까운 약 `x2.50` 이다.

And 음수 포인트 또는 손상된 포인트 값은 `x1.00` 으로 새니타이즈된다.

Automation: `IdleProject.GameCore.Achievement.FormulaCatalog`,
`server/src/core/formulas/achievement.test.ts`,
`server/tests/balance-sim.test.ts`

## 시나리오 5: 업적 상태가 저장과 클라우드 복원으로 라운드트립

Given 플레이어가 여러 업적 지표를 누적하고 일부 티어를 달성했다.

And `UIdleSaveGame` 은 `AchievementMetrics` 와 `Achievements` 필드를 가진다.

When `CaptureToSave` 가 업적 상태를 저장하고 #54 `clientSave` 업로드가
해당 payload 를 전송한다.

Then 누적 지표, 최고값 지표, 달성 티어, 총 포인트가 payload 에 포함된다.

And 다른 세션에서 `ApplyFromSave` 또는 클라우드 다운로드를 수행하면
동일한 업적 배수와 포인트가 복원된다.

And 복원 직후 `RefreshDerivedStats` 는 저장 전과 같은 CP를 재계산한다.

Automation: `IdleProject.GameCore.Achievement.ServiceProgress`,
`IdleProject.GameCore.Achievement.GameInstanceHooks`,
`IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip`

## 시나리오 6: v1/v2 세이브와 빈 업적 상태가 회귀 없이 동작

Given 기존 v1/v2 세이브에는 업적 필드가 없다.

And 서버 또는 로컬 payload 의 업적 배열이 비어 있거나 일부 항목이
손상되어 있다.

When `ApplyFromSave` 와 `UAchievementService::RestoreState` 가 payload 를
읽는다.

Then 누락된 업적 상태는 0 지표, 0 티어, 0 포인트로 초기화된다.

And 업적 배수는 `x1.00` 이며 기존 저장/로드, 인벤토리, 스킬, 퀘스트,
시즌 복원에 영향을 주지 않는다.

And 다음 저장은 정상화된 업적 상태를 포함한다.

Automation: `IdleProject.GameCore.SaveSystem.ApplyCaptureRoundTrip`,
`IdleProject.GameCore.Achievement.ServiceProgress`

## 시나리오 7: HUD 업적 패널과 달성 토스트가 진행 정보를 표시

Given HUD 가 `UAchievementService` 의 업적 상태와
`OnAchievementUnlocked` 델리게이트를 구독한다.

When 업적 패널 뷰모델을 빌드한다.

Then 패널은 총 포인트, 스탯 보너스, 카테고리별 현재 티어, 현재 진행,
다음 임계값을 표시한다.

And 카테고리 행은 최소 8개 이상이며 9개 카테고리를 빠짐없이 요약한다.

And 업적 달성 시 ko/en 로컬라이즈된 달성 토스트가 표시된다.

And 패널은 오른쪽 HUD 스택에서 기존 전투/저장 상태 UI와 겹치지 않는다.

Automation: `IdleProject.UI.HUD.AchievementPanelViewModel`,
`IdleProject.Localization.CsvIntegrity`, Playwright/PIE visual smoke.

## 수동 재현 노트

- 카탈로그 확인: PIE 시작 후 업적 패널을 열어 9개 카테고리 행과 총
  포인트/스탯 보너스 표시를 캡처한다.
- 진행 확인: 몬스터 처치 10회, 장비 강화 5회, 레벨 10 도달, 퀘스트
  완료 등 낮은 임계값을 순서대로 달성하고 토스트와 CP 증가를 캡처한다.
- 저장 확인: 업적 달성 후 autosave 또는 종료 저장을 수행하고 재시작 후
  포인트, 티어, CP가 동일하게 복원되는지 확인한다.
- 클라우드 확인: backend 실행 상태에서 업적 세이브를 업로드하고 새 로컬
  세이브 환경에서 다운로드하여 업적 상태가 복원되는지 확인한다.
- 회귀 확인: 업적 필드가 없는 v1/v2 세이브를 로드하고 배수가 `x1.00`
  인 상태로 기존 저장/로드 기능이 동작하는지 확인한다.

## 기대 증거

- UE Automation stdout 에 `IdleProject.GameCore.Achievement.*`,
  `IdleProject.Character.Stats.AchievementMultiplier`,
  `IdleProject.UI.HUD.AchievementPanelViewModel` 이 `Result={Success}` 로
  표시된다.
- 서버 vitest stdout 에 `server/src/core/formulas/achievement.test.ts` 가
  22개 정의, 9개 카테고리, 임계값, 소프트캡 배수 parity 를 통과했다고
  표시된다.
- `server/tests/balance-sim.test.ts` 와 `npm run balance:sim` 출력은
  250 포인트 `x2.475`, 500 포인트 약 `x2.50` 배수 압력을 포함한다.
- 수동 PIE 검증은 업적 패널, 달성 토스트, 저장 전후 CP 비교,
  클라우드 payload 의 업적 필드 캡처를 남긴다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
Push-Location server
npm run build
npm test -- src/core/formulas/achievement.test.ts
npm test -- tests/balance-sim.test.ts
npm run balance:sim
Pop-Location

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.GameCore.Achievement; Quit' `
  -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Character.Stats.AchievementMultiplier; Automation RunTests IdleProject.UI.HUD.AchievementPanelViewModel; Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
