# M7 Equipment Sets HUD V1 QA 시나리오

## 범위

- 장비 요약 HUD의 세트 진행도 표시
- 2세트/4세트 활성 단계와 다음 단계 안내
- 세트 보너스 요약의 ko/en 로컬라이즈 키 무결성
- 기존 장비/affix/방어구 요약 회귀 방지

## 시나리오 1: 3/4 세트 진행도와 2세트 활성 표시

Given 플레이어가 Warrior 세트 장비 3개와 세트가 없는 장비 1개를 장착했다.

When HUD가 장비 요약을 갱신한다.

Then 세트 줄에 `Warrior Set 3/4 (2-piece active)`가 표시된다.

And 다음 단계 안내는 `Next 4-piece: 1 more`로 표시된다.

And 활성 보너스는 `Bonus: PATK +20`으로 표시된다.

Automation: `IdleProject.UI.HUD.EquipmentSetSummary`

## 시나리오 2: 4세트 완성 표시와 누적 보너스

Given 플레이어가 Warrior 세트 장비 4개를 장착했다.

When HUD가 세트 요약 ViewModel을 만든다.

Then 현재 단계는 `4-piece active`로 표시된다.

And 다음 단계 안내는 `Set complete`로 표시된다.

And 보너스는 2세트와 4세트 누적값인 `Bonus: PATK +70 / Crit +5%`로 표시된다.

Automation: `IdleProject.UI.HUD.EquipmentSetSummary`

## 시나리오 3: 미충족 세트는 다음 2세트 안내만 표시

Given 플레이어가 Guardian 세트 장비 1개만 장착했다.

When HUD가 세트 요약 ViewModel을 만든다.

Then 현재 단계는 세트 보너스 없음으로 표시된다.

And 다음 단계 안내는 2세트까지 필요한 장비 수를 표시한다.

And 장비 자체의 공격/방어/체력 요약은 기존 값과 동일하게 유지된다.

Automation: `IdleProject.UI.HUD.EquipmentSetSummary`,
`IdleProject.Inventory.Bonus.SetBonus`

## 시나리오 4: 세트 없는 장비는 요약에 포함되지 않음

Given 플레이어가 Common 장비 또는 `ItemSet=None` 장비만 장착했다.

When HUD가 세트 요약 ViewModel을 만든다.

Then 세트 요약 행은 생성되지 않는다.

And 기존 무기/방어구/affix 요약만 표시된다.

Automation: `IdleProject.UI.HUD.EquipmentSetSummary`,
`IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 5: ko/en CSV 무결성

Given UI CSV에 세트 이름, 단계, 다음 단계, 보너스 포맷 키가 있다.

When CsvIntegrity Automation을 실행한다.

Then ko/en UI CSV의 키 목록이 일치한다.

And 비어 있거나 중복된 키가 없다.

Automation: `IdleProject.Localization.CsvIntegrity`

## 수동 확인

- 1080p, 1440p, 4K에서 좌측 하단 장비 요약이 HP/EXP/Gold, 스킬 HUD,
  스탯 정보 토글과 겹치지 않는지 확인한다.
- 세트 요약이 2줄 이상일 때 하단 여백 안에 들어오고, 무기 affix 줄과
  방어구 요약 줄을 밀어도 읽기 순서가 유지되는지 확인한다.
- ko/en 전환 후 세트 이름, 단계, 다음 단계, 보너스 문구가 모두 번역되는지
  확인한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -ExecCmds='Automation RunTests IdleProject.UI.HUD.EquipmentSetSummary; Automation RunTests IdleProject.Localization.CsvIntegrity; Quit' `
  -unattended -nop4 -nosplash -NullRHI `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
