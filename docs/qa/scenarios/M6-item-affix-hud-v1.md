# M6 Item Affix HUD V1 QA 시나리오

## 범위

- 장비 요약 HUD의 affix 표시
- Common/0 affix 장비의 기존 표시 회귀 방지
- 장비 교체 및 강화 후 HUD 갱신
- ko/en UI CSV 키 무결성

## 시나리오 1: affix가 있는 무기 요약 표시

Given 플레이어가 Rare 무기를 장착했고 해당 무기에 `Crit +3%`, `MATK +12`
affix가 있다.

When HUD가 장비 요약을 갱신한다.

Then 무기 줄은 이름, 희귀도, `ATK+N`을 표시한다.

And 다음 줄에 `Crit +3% / MATK +12` affix 요약을 표시한다.

Automation: `IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 2: 세 affix 표시 순서

Given 플레이어가 Legendary 무기를 장착했고 치명, 공속, 마공 affix가 모두 있다.

When HUD가 affix 요약을 만든다.

Then affix는 `Crit +3% / ASPD +0.10 / MATK +12` 순서로 표시된다.

And 공속은 소수점 둘째 자리까지 표시된다.

Automation: `IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 3: affix가 없는 Common 장비 회귀

Given 플레이어가 Common 무기를 장착했고 모든 affix 값이 0이다.

When HUD가 장비 요약을 갱신한다.

Then affix 줄은 표시하지 않는다.

And 기존 무기/방어구 요약 줄은 그대로 유지된다.

Automation: `IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 4: 장비 교체 후 affix 갱신

Given 플레이어가 affix 없는 Common 무기를 장착 중이다.

When 자동 장착 또는 상점 결과로 affix가 있는 Rare 무기로 교체된다.

Then `OnEquippedChanged` 이후 장비 요약 HUD에 새 무기의 affix 줄이 표시된다.

And 이전 무기의 affix 없음 상태가 남지 않는다.

Automation: `IdleProject.Inventory.AutoEquip.BetterWeapon`,
`IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 5: 강화 후 표시 안정성

Given 플레이어가 affix가 있는 무기를 장착하고 강화에 성공한다.

When HUD가 강화 결과를 받은 뒤 장비 요약을 갱신한다.

Then 무기 기본 공격력 표시와 affix 요약이 함께 유지된다.

And affix 값은 0보다 큰 항목만 표시된다.

Automation: `IdleProject.GameCore.IdleGameInstance.EnhanceAttempt`,
`IdleProject.UI.HUD.EquipmentAffixSummary`

## 시나리오 6: ko/en CSV 키 무결성

Given UI CSV에 다음 키가 있다.

- `AFFIX_CRIT_RATE_FORMAT`
- `AFFIX_ATK_SPEED_FORMAT`
- `AFFIX_MAGIC_ATK_FORMAT`

When CsvIntegrity Automation을 실행한다.

Then ko/en UI CSV의 키 개수와 키 목록이 일치한다.

And 빈 키 또는 중복 키가 없다.

Automation: `IdleProject.Localization.CsvIntegrity`

## 수동 확인

- 1080p, 1440p, 4K에서 좌측 하단 장비 요약이 HP/EXP/Gold, 스킬 HUD와
  겹치지 않는지 확인한다.
- affix 줄이 1줄로 스캔 가능하고, 장비 이름과 방어구 요약을 밀어내도
  하단 여백 안에 남는지 확인한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -ExecCmds='Automation RunTests IdleProject; Quit' `
  -unattended -nop4 -nosplash -NullRHI `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
