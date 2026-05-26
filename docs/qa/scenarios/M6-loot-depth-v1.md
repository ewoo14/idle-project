# M6 Loot Depth V1 QA 시나리오

## 범위

- 장착 HUD와 강화 HUD의 희귀도 이름 및 색상 표시
- Common, Uncommon, Rare, Epic, Legendary 등급의 ko/en 로컬라이즈
- 1080p, 1440p, 4K 해상도에서 장비 요약과 강화 패널 겹침 확인
- 기존 인벤토리, 자동 장착, 강화, 로컬라이즈 CSV 무결성 회귀 확인

## 시나리오 1: 장착 무기 희귀도 라벨과 색상

Given 플레이어가 Rare 이상 무기를 장착했다.

When HUD가 장착 요약을 갱신한다.

Then 무기 줄은 아이템 이름, 희귀도 이름, 공격력 보너스를 함께 표시한다.

And 무기 줄은 해당 희귀도 토큰 색상으로 표시한다.

Automation: `IdleProject.UI.HUD.EnhancePanelRarityViewModel`

Manual: 1080p, 1440p, 4K에서 좌측 하단 장비 요약이 HP/EXP/Gold HUD와 겹치지 않는지 확인한다.

## 시나리오 2: 강화 패널 희귀도 표시

Given 플레이어가 Epic 무기와 Legendary 방어구를 장착했다.

When 강화 패널을 렌더링한다.

Then 각 장착 행은 슬롯, 아이템 이름, 강화 레벨, 비용, 성공률과 함께 희귀도 이름을 표시한다.

And Epic 행은 `RarityEpic`, Legendary 행은 `RarityLegendary` 색상을 사용한다.

Automation: `IdleProject.UI.HUD.EnhancePanelRarityViewModel`

## 시나리오 3: ko/en 희귀도 로컬라이즈

Given 현재 언어가 `en`이다.

When HUD가 Epic 또는 Legendary 장비 행을 빌드한다.

Then 희귀도 라벨은 `Epic`, `Legendary`로 표시한다.

Given 현재 언어가 `ko`이다.

When 동일한 장비 행을 빌드한다.

Then 희귀도 라벨은 `영웅`, `전설`로 표시한다.

Automation: `IdleProject.Localization.LookupAndCultureSwitch`, `IdleProject.Localization.CsvIntegrity`

## 시나리오 4: CSV 키 누락 방지

Given UI CSV에 다음 키가 있다.

- `RARITY_NONE`
- `RARITY_COMMON`
- `RARITY_UNCOMMON`
- `RARITY_RARE`
- `RARITY_EPIC`
- `RARITY_LEGENDARY`

When CsvIntegrity Automation을 실행한다.

Then ko/en UI CSV의 키 개수와 키 목록이 일치한다.

And 비어 있는 번역이나 중복 키가 없어야 한다.

Automation: `IdleProject.Localization.CsvIntegrity`

## 시나리오 5: 기존 기능 회귀

Given 기존 인벤토리, 자동 장착, 강화 테스트 데이터가 유지된다.

When `IdleProject.Inventory`와 `IdleProject.UI.HUD` Automation을 실행한다.

Then 희귀도 색상/라벨 추가가 장착 보너스, 강화 가능 상태, 비용,
성공률 계산을 변경하지 않는다.

Automation: `IdleProject.Inventory`, `IdleProject.UI.HUD`

## 검증 명령

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' `
  IdleProjectEditor Win64 Development `
  -Project='C:\game\idle game\repo\client\IdleProject.uproject' `
  -WaitMutex -NoHotReload

$EditorCmd = 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

& $EditorCmd `
  'C:\game\idle game\repo\client\IdleProject.uproject' `
  -Unattended -NullRHI -NoSound -NoSplash `
  -ExecCmds='Automation RunTests IdleProject; Quit' `
  -TestExit='Automation Test Queue Empty'
```
