# M11 Equipment Depth V1 QA 시나리오

## 범위

- 강화 안전 구간 +0~+9, 위험 구간 +10~+50의 실패 처리.
- 보호서 사용, 위험 구간 연속 실패 12회 천장, 강화 비용/성공률/보상 곡선 회귀.
- 잠재 레이어 게이팅, 잠재 줄 수와 등급 상한, 큐브 리롤.
- 잠재 % 옵션의 장비 보너스, PowerScore, 자동장착 잠금 반영.
- SaveVersion 11에서 12로의 저장 마이그레이션과 클라우드 라운드트립.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 2, 3, 4, 5 |
| 경계 | 1, 2, 3, 6 |
| 회귀 | 1, 4, 5, 6, 7 |

## 시나리오 1: 안전 구간 실패는 강화 단계를 유지한다

Given +0부터 +9 사이의 장착 장비가 있고 현재 강화 단계가 +5이며 `EnhanceFailStreak == 2`이다.

When 강화 시도에서 실패 roll이 발생한다.

Then 강화 단계는 +5로 유지된다.

And `EnhanceFailStreak`는 3으로 증가한다.

And 보호서는 소모되지 않고 하락 플래그도 발생하지 않는다.

Automation: `IdleProject.Inventory.EnhanceFormula.RiskOutcome`, `server/src/core/formulas/enhance.test.ts`

## 시나리오 2: 위험 구간 실패는 보호서가 없으면 -1단계 하락한다

Given +10부터 +50 사이의 장착 장비가 있고 현재 강화 단계가 +20이며 `EnhanceFailStreak == 4`이다.

When 보호서를 사용하지 않은 강화 시도에서 실패 roll이 발생한다.

Then 강화 단계는 +19가 된다.

And 실패 streak는 5로 증가한다.

And 결과는 위험 구간 하락으로 기록된다.

Automation: `IdleProject.Inventory.EnhanceFormula.RiskOutcome`, `IdleProject.Inventory.EnhanceFormula.Curve`, `server/src/core/formulas/enhance.test.ts`

## 시나리오 3: 보호서가 위험 구간 실패 하락을 막고 1개만 소모된다

Given +20 장비, 보호서 1개 이상, 보호서 사용 토글 ON 상태가 있다.

When 강화 시도에서 실패 roll이 발생한다.

Then 강화 단계는 +20으로 유지된다.

And 보호서가 정확히 1개 소모된다.

And `EnhanceFailStreak`는 누적되어 다음 천장 판정에 반영된다.

Automation: `IdleProject.Inventory.EnhanceFormula.RiskOutcome`, `IdleProject.UI.HUD.EnhancePanelViewModel`, `server/src/core/formulas/enhance.test.ts`

## 시나리오 4: 위험 구간 12회 연속 실패 후 다음 시도는 천장 성공한다

Given 위험 구간 장비가 같은 단계에서 `EnhanceFailStreak == 12`에 도달했다.

When 다음 강화 시도를 실행한다.

Then roll 값과 무관하게 강화가 성공한다.

And 강화 단계는 +1 증가하고 `EnhanceFailStreak`는 0으로 초기화된다.

And 보호서 사용 토글이 켜져 있어도 성공 결과에서는 보호서를 소모하지 않는다.

Automation: `IdleProject.Inventory.EnhanceFormula.RiskOutcome`, `server/src/core/formulas/enhance.test.ts`, `server/tests/balance-sim.test.ts`

## 시나리오 5: 잠재는 Rare+ 장비에만 붙고 큐브는 등급 규칙을 지킨다

Given Common, Rare, Epic, Unique, Legendary, Mythic 장비가 각각 있다.

When 잠재 부여와 재설정/등급 큐브 리롤을 실행한다.

Then Common은 잠재 상한이 None이고 잠재 줄을 갖지 않는다.

And Rare는 Epic, Epic은 Unique, Unique 이상은 Legendary 상한을 넘지 않는다.

And 잠재 줄 수는 Rare 1줄, Epic 2줄, Unique/Legendary 3줄이다.

And 등급 큐브는 상한 미만에서만 승급 가능하고 상한 도달 후에는 줄만 재롤한다.

Automation: `IdleProject.Inventory.PotentialFormula.Rules`, `IdleProject.UI.HUD.PotentialPanelViewModel`, `server/src/core/formulas/potential.test.ts`

## 시나리오 6: 잠재 % 옵션은 PowerScore에 반영되고 드롭 어픽스를 침범하지 않는다

Given Rare+ 장비가 기본 스탯, 드롭 어픽스, 잠재 % 옵션을 모두 갖고 있다.

When 장비 보너스와 PowerScore를 계산한다.

Then 잠재 % 옵션은 강화 보정 이후의 파생 스탯에 반영된다.

And PowerScore는 잠재 항을 포함해 클라이언트와 서버에서 같은 앵커 값을 낸다.

And 기존 드롭 어픽스 필드는 잠재 리롤로 변경되지 않는다.

Automation: `IdleProject.Inventory.Bonus.Potential`, `server/src/core/formulas/equipment.test.ts`, `server/src/core/formulas/drop.test.ts`

## 시나리오 7: 잠긴 장비는 자동장착 교체 대상에서 제외된다

Given 현재 장착 중인 장비가 `bLocked == true`이고 이후 더 높은 PowerScore의 같은 슬롯 장비가 드롭된다.

When 자동장착 평가가 실행된다.

Then 잠긴 장비가 계속 장착 상태로 남는다.

And 새 장비는 인벤토리에 보관되지만 기존 장착 슬롯을 교체하지 않는다.

And 잠금이 해제된 동일 조건에서는 PowerScore 비교에 따라 교체가 가능하다.

Automation: `IdleProject.Inventory.AutoEquip.LockedItem`

## 시나리오 8: v11 저장은 v12 장비 심화 필드를 기본값으로 마이그레이션한다

Given `SaveVersion == 11` 저장에 기존 장비와 장착 슬롯이 있으나 잠재, 실패 streak, 잠금, 보호서/큐브 자원 필드는 없다.

When 저장을 로드하고 다시 캡처한다.

Then 저장 버전은 12로 승격된다.

And 장비 신규 필드는 `PotentialGrade=None`, 잠재 줄 없음, `EnhanceFailStreak=0`, `bLocked=false` 기본값을 갖는다.

And 신규 자원 `ProtectionScrolls`, `ResetCubes`, `RankCubes`는 0으로 시작한다.

And v12 저장을 다시 로드해도 마이그레이션이 중복 적용되지 않는다.

Automation: `IdleProject.GameCore.SaveSystem.SaveGameDefaults`, `IdleProject.GameCore.SaveSystem.ExpandedItemFieldsRoundTrip`, `IdleProject.GameCore.SaveSystem.CloudPayloadRoundTrip`

## 수동 재현 힌트

- 강화 패널에서 +9와 +10 장비를 각각 준비해 실패 결과의 문구, 보호 토글 활성 여부, 천장 `N/12` 표시를 비교한다.
- 보호서 1개만 보유한 상태에서 위험 구간 실패를 강제하고 보호서가 0개로 감소하는지 확인한다.
- 잠재 패널에서 Common 장비는 리롤 버튼이 비활성이고 Rare+ 장비는 큐브 보유 수에 따라 Reset/Rank 버튼이 활성화되는지 확인한다.
- 잠긴 장비보다 높은 PowerScore 장비를 드롭시켜 장착 슬롯이 유지되는지 확인한다.
- v11 fixture를 적용한 뒤 저장 파일과 클라우드 payload에 v12 신규 필드가 기본값으로 포함되는지 확인한다.

## 기대 증거

- UE Automation stdout: `IdleProject.Inventory.EnhanceFormula.RiskOutcome`, `IdleProject.Inventory.PotentialFormula.Rules`, `IdleProject.Inventory.Bonus.Potential`, `IdleProject.Inventory.AutoEquip.LockedItem`, `IdleProject.GameCore.SaveSystem.ExpandedItemFieldsRoundTrip` 성공 로그.
- 서버 vitest stdout: `enhance.test.ts`, `potential.test.ts`, `equipment.test.ts`, `drop.test.ts`, `balance-sim.test.ts` 성공 로그.
- 수동 캡처 또는 로그: +9 실패 유지, +10 실패 하락, 보호 실패 유지 및 보호서 1개 소모, 12회 천장 성공, 잠금 자동장착 제외.
- 저장 증거: v11 fixture 적용 후 `SaveVersion=12`, 신규 자원 0, 잠재 기본값, `EnhanceFailStreak=0`, `bLocked=false`.

## 검증 명령

```powershell
Push-Location server
npm test -- src/core/formulas/enhance.test.ts src/core/formulas/potential.test.ts src/core/formulas/equipment.test.ts src/core/formulas/drop.test.ts tests/balance-sim.test.ts
Pop-Location

$UE = 'C:\Program Files\Epic Games\UE_5.7\Engine'
$Project = 'C:\game\idle game\repo\client\IdleProject.uproject'

& "$UE\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project=$Project `
  -WaitMutex

$Tests = @(
  'IdleProject.Inventory.EnhanceFormula',
  'IdleProject.Inventory.PotentialFormula',
  'IdleProject.Inventory.Bonus.Potential',
  'IdleProject.Inventory.AutoEquip.LockedItem',
  'IdleProject.GameCore.SaveSystem.ExpandedItemFieldsRoundTrip'
)

foreach ($Test in $Tests) {
  & "$UE\Binaries\Win64\UnrealEditor-Cmd.exe" `
    $Project `
    -unattended -nop4 -nosplash -NullRHI `
    -ExecCmds="Automation RunTests $Test; Quit" `
    -TestExit='Automation Test Queue Empty'
}
```
