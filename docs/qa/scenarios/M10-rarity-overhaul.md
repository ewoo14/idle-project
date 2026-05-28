# M10 Rarity Overhaul QA 시나리오

## 범위

- 장비/룬 rarity 7단계: Common, Rare, Epic, Unique, Legendary, Transcendent, Mythic.
- v6 이하 저장 데이터의 레거시 정수 등급을 v7 rarity enum으로 1회 변환.
- 룬 도감 54셀 저장을 63셀 그리드로 복원하면서 레거시 2/3등급 unlock을 Rare 행으로 병합.
- 서버 저장 payload `maxEquipmentGrade` 0~7 수용 및 8 이상 거부.
- 클라이언트/서버 formula parity와 활성 코드의 제거된 등급명 회귀 방지.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3 |
| 경계 | 1, 2, 4 |
| 회귀 | 2, 3, 4, 5 |

## 시나리오 1: 레거시 등급 저장이 v7 rarity로 1회 마이그레이션된다

Given `SaveVersion == 6` 저장에 장비와 룬의 레거시 rarity 값 1, 2, 3, 4, 5, 6, 0, 8이 포함된다.

When `UIdleGameInstance::ApplyFromSave()`가 저장을 적용하고 `CaptureToSave()`가 다시 저장한다.

Then 1은 Common, 2와 3은 Rare, 4는 Epic, 5는 Legendary, 6은 Mythic으로 변환된다.

And 0과 8 이상은 None 또는 invalid로 취급되어 활성 룬 저장에서 제외된다.

And 저장 결과는 `SaveVersion == 7`이다.

Automation: `IdleProject.Item.RarityMigration.LegacyValues`, `IdleProject.Item.RarityMigration.ApplyFromSaveV6ToV7`

## 시나리오 2: v7 저장은 두 번 변환되지 않는다

Given `SaveVersion == 7` 저장에 Transcendent 장비가 포함된다.

When `ApplyFromSave()`와 `CaptureToSave()`를 연속 실행한다.

Then Transcendent는 Mythic 또는 다른 행으로 재해석되지 않고 그대로 유지된다.

And v7 저장의 rarity 정수 0~7은 서버 item contract와 cloud save bounds에서 모두 유효하다.

Automation: `IdleProject.Item.RarityMigration.ApplyFromSaveV6ToV7`, `server/src/core/formulas/equipment.test.ts`, `server/src/modules/save/save.test.ts`

## 시나리오 3: 7단계 formula parity가 클라이언트와 서버에서 유지된다

Given rarity 행은 Common, Rare, Epic, Unique, Legendary, Transcendent, Mythic 순서로 정의된다.

When drop, enhance, rune, rune codex, rune set, class rune, set bonus formula 테스트를 실행한다.

Then 서버 vitest와 UE Automation은 7개 활성 rarity 행의 stat multiplier, enhancement cost multiplier, row completion bonus, set eligibility를 같은 값으로 검증한다.

And Unique는 Epic과 Legendary 사이, Transcendent는 Legendary와 Mythic 사이의 보간 행으로 유지된다.

Automation: `server/src/core/formulas/drop.test.ts`, `server/src/core/formulas/enhance.test.ts`, `server/src/core/formulas/rune.test.ts`, `server/src/core/formulas/runeCodex.test.ts`, `IdleProject.Item.*`, `IdleProject.Rune.*`

## 시나리오 4: 룬 도감 54셀 저장이 63셀로 복원되고 신규 행은 잠겨 있다

Given v6 저장의 룬 도감에 9개 룬 타입마다 레거시 2등급과 3등급 unlock이 있다.

When `ApplyFromSave()`가 룬 도감 저장을 복원한다.

Then 복원된 도감은 63셀, core 35셀, util 28셀을 가진다.

And 레거시 2등급과 3등급 unlock은 9개의 Rare 행 unlock으로 병합된다.

And Unique와 Transcendent 행은 새 저장에서 획득하기 전까지 locked 상태다.

Automation: `IdleProject.Item.RarityMigration.RuneCodexLegacy54To63`, `IdleProject.Rune.Service.Codex*`, `server/src/core/formulas/runeCodex.test.ts`

## 시나리오 5: 제거된 rarity 명칭이 활성 코드에 재도입되지 않는다

Given PR #65 이후 활성 클라이언트/서버 코드는 7단계 rarity 명칭만 사용한다.

When QA가 `client/Source/IdleProject`와 `server/src`를 대상으로 제거된 영문 등급명을 grep한다.

Then 결과는 0건이어야 한다.

And 과거 QA 문서나 계획 문서에 남은 역사적 표현은 활성 코드 회귀로 판정하지 않는다.

Automation: `rg -n "Uncommon" client/Source/IdleProject server/src`

## 수동 재현 힌트

- v6 세이브 fixture를 만들 때 장비/룬 rarity 값 1~6, 0, 8을 모두 넣고 적용 후 저장을 다시 캡처한다.
- 룬 도감은 9개 타입 x 레거시 2/3등급 unlock을 입력해 Rare row 9칸만 켜지는지 확인한다.
- 서버 저장 업로드는 `maxEquipmentGrade` 0~7을 각각 통과시키고 -1/8은 ValidationError인지 확인한다.
- UI smoke에서는 rarity label이 7단계만 보이고 Unique/Transcendent 색상이 token과 일치하는지 확인한다.

## 기대 증거

- UE Automation stdout: `IdleProject.Item.RarityMigration.*`와 `IdleProject.Rune.*` 성공 로그.
- 서버 vitest stdout: `save.test.ts`, `equipment.test.ts`, `drop.test.ts`, `enhance.test.ts`, `rune.test.ts`, `runeCodex.test.ts` 성공 로그.
- grep stdout: 활성 코드 대상 `Uncommon` 검색 0건.
- 수동 캡처 또는 로그: v6 fixture 적용 후 `SaveVersion=7`, `RuneCodex.Num=63`, Rare row unlock 병합, Unique/Transcendent locked.

## 검증 명령

```powershell
Push-Location server
npm run build
npm test -- src/core/formulas/equipment.test.ts src/core/formulas/drop.test.ts src/core/formulas/enhance.test.ts src/core/formulas/rune.test.ts src/core/formulas/runeCodex.test.ts src/modules/save/save.test.ts
Pop-Location

rg -n "Uncommon" client/Source/IdleProject server/src

$UE = 'C:\Program Files\Epic Games\UE_5.7\Engine'
$Project = 'C:\game\idle game\repo\client\IdleProject.uproject'

& "$UE\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project=$Project `
  -WaitMutex

& "$UE\Binaries\Win64\UnrealEditor-Cmd.exe" `
  $Project `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds='Automation RunTests IdleProject.Item.RarityMigration; Quit' `
  -TestExit='Automation Test Queue Empty'
```
