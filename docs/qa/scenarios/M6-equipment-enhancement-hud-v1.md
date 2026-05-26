# M6 장비 강화 HUD V1 QA 시나리오

## 범위

- C++ Canvas HUD 강화 패널 표시
- 장착 슬롯별 강화 레벨, 비용, 성공률, 상태 문구
- 골드 부족, 최대 강화, 빈 슬롯 비활성 처리
- 강화 클릭 후 성공/실패 피드백
- 희귀도별 강화 비용 표시와 실제 차감 비용 정합
- 기존 인벤토리, 스탯, 퀘스트 진행 회귀

## 시나리오 1: 강화 가능 슬롯 표시

Given 플레이어가 무기 슬롯에 +2 Common 장비를 장착하고 1,000 골드를 보유한다.

When HUD가 강화 패널을 렌더링한다.

Then 무기 행은 `+2 / +5`, 비용 `900`, 성공률 `70%`를 표시한다.

And 강화 버튼은 활성 상태로 표시된다.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 1-1: 희귀도별 강화 비용 표시

Given 플레이어가 무기 슬롯에 +0 Common 장비를 장착하고 장갑 슬롯에 +0 Legendary 장비를 장착한다.

When HUD가 강화 패널을 렌더링한다.

Then 무기 행은 Common 배수 1을 적용한 비용 `100`을 표시한다.

And 장갑 행은 Legendary 배수 16을 적용한 비용 `1600`을 표시한다.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 2: 골드 부족 비활성

Given 플레이어가 무기 슬롯에 +2 장비를 장착하고 100 골드를 보유한다.

When HUD가 강화 패널을 렌더링한다.

Then 무기 행은 골드 부족 상태를 표시한다.

And 강화 버튼은 클릭 대상에 등록되지 않는다.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 2-1: Legendary 골드 게이트

Given 플레이어가 무기 슬롯에 +0 Legendary 장비를 장착하고 Common +0 비용인 100 골드만 보유한다.

When 플레이어가 강화 버튼을 클릭한다.

Then `TryEnhanceEquipped(Weapon)`은 시도하지 않은 결과를 반환한다.

And 골드는 차감되지 않고 장비 강화 레벨도 유지된다.

Automation: `IdleProject.GameCore.IdleGameInstance.EnhanceAttempt`

## 시나리오 3: 최대 강화 비활성

Given 플레이어가 투구 슬롯에 +5 장비를 장착한다.

When HUD가 강화 패널을 렌더링한다.

Then 투구 행은 최대 강화 상태를 표시한다.

And 강화 비용과 성공률 대신 비활성 값을 표시한다.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 4: 빈 슬롯 비활성

Given 플레이어가 상의 슬롯에 장비를 장착하지 않았다.

When HUD가 강화 패널을 렌더링한다.

Then 상의 행은 빈 슬롯 상태를 표시한다.

And 강화 버튼은 비활성 상태로 표시된다.

Automation: `IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 5: 강화 결과 피드백

Given 플레이어가 강화 가능한 장비와 충분한 골드를 보유한다.

When 플레이어가 강화 버튼을 클릭한다.

Then HUD는 `TryEnhanceEquipped(Slot)` 결과를 받아 성공 또는 실패 피드백을 표시한다.

And 성공 시 신규 강화 레벨을 피드백에 포함한다.

And 실패 시 소모 골드를 피드백에 포함한다.

Automation: `IdleProject.GameCore.IdleGameInstance.EnhanceAttempt`,
`IdleProject.UI.HUD.EnhancePanelViewModel`

## 시나리오 6: Common 비용 회귀

Given 플레이어가 무기 슬롯에 +0 Common 장비를 장착하고 100 골드를 보유한다.

When 플레이어가 강화 버튼을 클릭한다.

Then `TryEnhanceEquipped(Weapon)`은 기존 단일 인자 비용과 같은 100 골드로 시도한다.

And 골드는 한 번만 차감되며 성공 여부와 무관하게 추가 비용은 발생하지 않는다.

Automation: `IdleProject.GameCore.IdleGameInstance.EnhanceAttempt`

## 회귀 확인

```powershell
Build.bat IdleProjectEditor Win64 Development `
  -Project=client/IdleProject.uproject `
  -WaitMutex

UnrealEditor-Cmd.exe client/IdleProject.uproject `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds="Automation RunTests IdleProject; Quit" `
  -TestExit="Automation Test Queue Empty"
```

- `markdownlint-cli2 "**/*.md" "#node_modules" "#client" "#server/node_modules"`
