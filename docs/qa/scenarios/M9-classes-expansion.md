# M9 Classes Expansion QA 시나리오

## 범위

- 기존 5직업에 Paladin, Berserker, Summoner 를 더한 8직업 선택.
- `EClassId` 숫자 고정: Warrior~Cleric 유지, Paladin=6,
  Berserker=7, Summoner=8.
- 신규 3직업의 `FClassGrowth` 차별화와 서버 stats mirror parity.
- 신규 3직업 x 7스킬 로딩, `LoadSkillsForClass` switch, SkillDB parity.
- Summoner 마법 피해 라우팅, Paladin/Berserker 물리 피해 라우팅.
- Class selection HUD 8옵션, ko/en 직업명/역할, Skill ko/en 키.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 4, 5 |
| 엣지 | 6, 7 |
| 회귀 | 1, 2, 3, 4, 5, 6, 7 |

## 시나리오 1: 8직업 enum 과 선택 순서가 고정된다

Given `EClassId` 는 기존 None, Warrior, Mage, Archer, Thief, Cleric 값을
보존한다.

And 신규 직업은 Paladin=6, Berserker=7, Summoner=8 로 추가된다.

When class selection HUD 옵션을 생성한다.

Then 총 8개 옵션이 Warrior, Mage, Archer, Thief, Cleric, Paladin,
Berserker, Summoner 순서로 표시된다.

And 선택된 직업은 `CurrentClassId` 와 일치하는 옵션에만 selected 상태를
부여한다.

Automation: `IdleProject.UI.HUD.ClassSelectionDisplayModel`,
`IdleProject.Character.ClassSelection.AppliesSkills`

## 시나리오 2: 신규 3직업 스킬은 각각 7개 구성으로 로딩된다

Given `USkillComponent` 가 비어 있거나 다른 직업 스킬을 가진다.

When `LoadDefaultPaladinSkills()`, `LoadDefaultBerserkerSkills()`,
`LoadDefaultSummonerSkills()` 를 각각 호출한다.

Then 각 직업은 active 4개, passive 2개, ultimate 1개를 포함해 정확히
7개 스킬을 가진다.

And Paladin 은 `holy_verdict`, `guardian_aegis`, `lay_on_hands`,
`divine_bastion` 을 포함한다.

And Berserker 는 `rage_cleave`, `blood_surge`, `savage_leap`,
`berserk_apex` 를 포함한다.

And Summoner 는 `spirit_bolt`, `familiar_swarm`, `void_call`,
`grand_familiar` 를 포함한다.

Automation: `IdleProject.Combat.Skills.ClassDefaults`

## 시나리오 3: LoadSkillsForClass 가 8직업 전체를 dispatch 한다

Given 플레이어가 이전 직업 스킬 세트를 이미 로딩했다.

When `LoadSkillsForClass(EClassId)` 를 classId 1부터 8까지 순서대로
호출한다.

Then 각 호출은 해당 직업의 기본 스킬 세트로 교체된다.

And 이전 직업의 active/passive/ultimate 스킬은 중복되거나 남지 않는다.

When `EClassId::None` 또는 지원하지 않는 classId 를 전달한다.

Then 스킬 목록은 안전하게 비워지거나 기본 Warrior 로 암묵 매핑되지
않는다.

Automation: `IdleProject.Combat.Skills.ClassDefaults`,
`IdleProject.Character.ClassSelection.AppliesSkills`

## 시나리오 4: 신규 FClassGrowth 가 직업별 스탯 역할을 만든다

Given Paladin, Berserker, Summoner 의 `FClassGrowth` 프로필이 존재한다.

When 레벨 1과 레벨 50 기본 스탯을 계산한다.

Then Paladin 은 STR/CON 중심이며 HP/물리 방어 성장 기대치를 충족한다.

And Berserker 는 STR 중심 물리 공격 성장과 낮은 WIS anchor 를 가진다.

And Summoner 는 INT/WIS 중심이며 CON 이 탱커 직업보다 낮다.

And 기존 Warrior, Mage, Archer, Thief, Cleric 레벨 anchor 는 변경되지
않는다.

Automation: `IdleProject.Character.StatFormulas.DefaultPrimaryStats`,
`IdleProject.Character.StatFormulas.DeriveStats`,
`server/src/core/formulas/stats.test.ts`

## 시나리오 5: 전투 라우팅이 신규 직업 역할과 일치한다

Given Paladin 과 Berserker 는 물리 직업으로 정의된다.

When `FCombatFormulas::ComputeDamage` 와 서버 `computeClassDamage` 를
호출한다.

Then Paladin 과 Berserker 피해는 PhysAtk 와 PhysDef 로 계산된다.

Given Summoner 는 마법 직업으로 정의된다.

When Summoner 의 active 또는 ultimate 피해 스킬이 resolve 된다.

Then Summoner 피해는 MagicAtk 와 MagicDef 로 계산된다.

And Summoner 스킬의 poison/freeze/lightning element/status metadata 는
스킬 정의 parity 에 포함된다.

Automation: `IdleProject.Combat.Formulas.ClassDamage`,
`IdleProject.Combat.Skills.MagicDamage`,
`IdleProject.Combat.Skills.ElementStatusApplication`,
`server/src/core/formulas/combat.test.ts`

## 시나리오 6: 클라이언트와 서버 SkillDB parity 가 56개 스킬을 검증한다

Given 클라이언트 `USkillComponent` 와 서버
`server/src/core/data/skills.ts` 가 같은 8직업 정의를 가진다.

When parity 테스트가 classId 1부터 8까지 로딩한다.

Then 총 56개 스킬이 검증된다.

And 각 스킬은 `skillId`, `classId`, `type`, `effectType`, `cooldown`,
`damageCoeff`, `buffMagnitude`, `buffDuration`, `gaugeGainOnHit`,
`gaugeGainOnTakeDamage`, `statusEffect`, `statusChance`,
`statusDuration`, `element` 값이 일치한다.

And Paladin 의 heal/self-buff, Berserker 의 dash/burn/ultimate,
Summoner 의 poison/freeze/lightning 스킬이 모두 포함된다.

Automation: `IdleProject.Combat.Skills.DefinitionParity`,
`server/src/core/data/skills.test.ts`

## 시나리오 7: 로컬라이즈와 HUD 표시가 8직업을 모두 커버한다

Given UI ko/en CSV 에 신규 직업명과 역할 키가 있다.

And Skill ko/en CSV 에 신규 21개 skillId 키가 있다.

When class selection HUD 와 skill HUD 를 연다.

Then Paladin, Berserker, Summoner 옵션은 로컬라이즈된 이름, 역할,
주요 스탯 요약을 표시한다.

And class selection HUD 는 8개 옵션이 겹치거나 잘리지 않는다.

And skill HUD 는 선택 직업의 7개 스킬명과 cooldown/ultimate 상태를
표시한다.

When 로케일을 ko 와 en 으로 전환한다.

Then 모든 신규 직업/스킬 문자열은 fallback key 대신 번역 문자열로
표시된다.

Automation: `IdleProject.UI.HUD.ClassSelectionDisplayModel`,
`IdleProject.UI.HUD.SkillDisplayModel`,
`IdleProject.Localization.CsvIntegrity`, Playwright/PIE visual smoke.

## 수동 재현 노트

- 직업 선택: PIE 시작 후 class selection HUD 를 열고 8개 옵션 순서,
  selected 상태, Paladin/Berserker/Summoner 설명 표시를 캡처한다.
- 스킬 로딩: 신규 3직업을 각각 선택하고 HUD 에 active 4개, passive 2개,
  ultimate 1개가 표시되는지 확인한다.
- 전투 라우팅: Summoner 와 Paladin/Berserker 를 같은 공격/방어 anchor 로
  비교해 마법/물리 방어 적용 차이를 로그로 남긴다.
- 로컬라이즈: ko/en 로케일에서 직업명, 역할, 신규 21개 스킬명이 fallback
  key 없이 표시되는지 캡처한다.
- 회귀 확인: 기존 Warrior, Mage, Archer, Thief, Cleric 선택과 스킬 수,
  기존 classId 저장값, 기존 스탯 anchor 가 변하지 않았음을 기록한다.

## 기대 증거

- UE Automation stdout 에 `IdleProject.Combat.Skills.ClassDefaults`,
  `IdleProject.Combat.Skills.DefinitionParity`,
  `IdleProject.Combat.Formulas.ClassDamage`,
  `IdleProject.Character.StatFormulas.DefaultPrimaryStats`,
  `IdleProject.UI.HUD.ClassSelectionDisplayModel` 이 `Result={Success}` 로
  표시된다.
- 서버 vitest stdout 에 `server/src/core/data/skills.test.ts`,
  `server/src/core/formulas/stats.test.ts`,
  `server/src/core/formulas/combat.test.ts` 가 통과했다고 표시된다.
- CsvIntegrity stdout 은 UI/Skill ko/en 신규 키와 필수 열 정합이
  통과했음을 표시한다.
- 수동 PIE 증거는 8직업 선택 HUD 캡처, 신규 직업별 7스킬 HUD 캡처,
  Summoner 마법 라우팅 로그, Paladin/Berserker 물리 라우팅 로그를
  포함한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
Push-Location server
npm run build
npm test -- `
  src/core/data/skills.test.ts `
  src/core/formulas/stats.test.ts `
  src/core/formulas/combat.test.ts
Pop-Location

$UE = 'C:\Program Files\Epic Games\UE_5.7\Engine'
$Project = 'C:\game\idle game\repo\client\IdleProject.uproject'

& "$UE\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project=$Project `
  -WaitMutex

$CombatTests = 'Automation RunTests IdleProject.Combat.Skills; ' +
  'Automation RunTests IdleProject.Combat.Formulas.ClassDamage; Quit'

& "$UE\Binaries\Win64\UnrealEditor-Cmd.exe" `
  $Project `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds=$CombatTests `
  -TestExit='Automation Test Queue Empty'

$HudTests = 'Automation RunTests IdleProject.Character.StatFormulas; ' +
  'Automation RunTests IdleProject.UI.HUD.ClassSelectionDisplayModel; ' +
  'Automation RunTests IdleProject.Localization.CsvIntegrity; Quit'

& "$UE\Binaries\Win64\UnrealEditor-Cmd.exe" `
  $Project `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds=$HudTests `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
