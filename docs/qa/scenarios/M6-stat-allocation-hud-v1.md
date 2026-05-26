# M6 스탯 분배 HUD V1 QA 시나리오

## 범위

- C++ Canvas HUD 스탯 분배 패널 표시
- STR/DEX/INT/WIS/CON/LUK 6개 1차 능력치의 기본값, 분배값, 합산값 표시
- 잔여 스탯 포인트 표시와 `StatAlloc_` + 버튼 활성/비활성 처리
- `StatReset` 초기화 액션과 분배 포인트 반환
- 스탯 포인트 변경 후 캐릭터 파생 능력치 갱신 및 HUD 재렌더링

## 시나리오 1: 잔여 포인트가 있을 때 + 버튼 활성화

Given 플레이어가 레벨업으로 잔여 스탯 포인트 4점을 보유한다.

When HUD가 스탯 분배 패널을 렌더링한다.

Then 패널은 STR/DEX/INT/WIS/CON/LUK 6개 행을 표시한다.

And 각 행의 + 버튼은 활성 상태로 표시된다.

Automation: `IdleProject.UI.HUD.StatAllocationPanelViewModel`

## 시나리오 2: 능력치 분배 후 합산값 표시

Given 플레이어가 INT에 2점, LUK에 3점을 분배했다.

When HUD가 스탯 분배 패널을 렌더링한다.

Then INT 행은 기본 INT에 +2가 반영된 합산값을 표시한다.

And LUK 행은 기본 LUK에 +3이 반영된 합산값을 표시한다.

Automation: `IdleProject.UI.HUD.StatAllocationPanelViewModel`

## 시나리오 3: 잔여 포인트 0점일 때 + 버튼 비활성화

Given 플레이어의 잔여 스탯 포인트가 0점이다.

When HUD가 스탯 분배 패널을 렌더링한다.

Then 모든 + 버튼은 비활성 상태로 표시된다.

And + 버튼 HitBox는 등록되지 않아 `AllocateStatPoint`가 호출되지 않는다.

Automation: `IdleProject.UI.HUD.StatAllocationPanelViewModel`

## 시나리오 4: 초기화 버튼

Given 플레이어가 하나 이상의 스탯 포인트를 분배했다.

When 플레이어가 초기화 버튼을 클릭한다.

Then HUD는 `ResetStatPoints()`를 호출한다.

And 분배된 포인트는 잔여 포인트로 반환되고 6개 분배값은 0으로 돌아간다.

Automation: `IdleProject.GameCore.IdleGameInstance.StatPointAllocation`,
`IdleProject.UI.HUD.StatAllocationPanelViewModel`

## 시나리오 5: 스탯 변경 후 파생 능력치 갱신

Given 플레이어가 STR에 1점을 분배했다.

When `OnStatPointsChanged`가 브로드캐스트된다.

Then 캐릭터는 `RefreshDerivedStats()`를 통해 파생 능력치를 다시 계산한다.

And HUD는 다음 DrawHUD에서 변경된 합산값을 표시한다.

Automation: `IdleProject.Character.StatPointFormula.AllocatedPrimaryStatsAffectDerivedStats`

## 회귀 확인

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

$Tests = 'IdleProject.UI.HUD.StatAllocationPanelViewModel'
UnrealEditor-Cmd.exe 'C:\game\idle game\repo\client\IdleProject.uproject' `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds="Automation RunTests $Tests; Quit" `
  -TestExit="Automation Test Queue Empty"
```
