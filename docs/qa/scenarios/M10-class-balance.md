# M10 Class Balance QA 시나리오

## 범위

- 8직업 동일 투자 조건에서 유효 전투 출력과 CP 비교.
- DPS 6직업(Warrior, Mage, Archer, Thief, Berserker, Summoner)의
  중앙값 대비 +/-15% 밴드 검증.
- Paladin 은 탱커 예외, Cleric 은 회복/유틸 예외로 DPS 밴드 밖 허용.
- Berserker, Summoner 의 PR #60 튜닝 anchor 와 클라/서버 parity.
- `tools/balance-sim` 클래스 스냅샷과 1000-run 첫 환생 진행 페이싱.
- `StatFormulas`, `skills.ts`, `SkillComponent`,
  `ClassDamage` Automation 회귀.
- `docs/planning/05-balance-philosophy.md` 가 테스트 기준을 설명.

## 시나리오 분류

| 분류 | 시나리오 |
| --- | --- |
| 정상 | 1, 2, 3, 4 |
| 엣지 | 2, 5, 6 |
| 회귀 | 1, 3, 4, 5, 6 |

## 시나리오 1: DPS 6직업이 동일 조건에서 +/-15% 밴드에 들어온다

Given `tools/balance-sim` 은 8직업 class snapshot 을 생성한다.

And 비교 조건은 동일 레벨, 동일 장비, 동일 스탯 분배, 동일 적 방어다.

When Lv50 과 Lv100 에서 Warrior, Mage, Archer, Thief, Berserker,
Summoner 의 effective DPS 를 계산한다.

Then 6직업 effective DPS 는 각 레벨 중앙값 대비 +/-15% 안에 있다.

And `server/tests/class-balance.test.ts` 는 밴드 이탈 시 실패한다.

And Warrior vs Berserker, Mage vs Summoner 는 한쪽이 DPS, CP, 생존 지표를
동시에 strict-better 하지 않는다.

Automation: `server/tests/class-balance.test.ts`, `npm run balance:sim`

## 시나리오 2: 탱커와 힐러는 역할 예외로 DPS 밴드 밖을 허용한다

Given Paladin 은 CON/방어 중심 탱커 역할이다.

And Cleric 은 회복과 유틸리티 중심 지원 역할이다.

When Lv50 과 Lv100 class snapshot 을 생성한다.

Then Paladin 은 DPS 밴드보다 낮을 수 있지만 HP, 방어, CP 보상으로
역할 정체성을 가진다.

And Cleric 은 DPS 밴드보다 낮을 수 있지만 self-heal/support budget 이
문서와 테스트 기대치에 반영된다.

And Paladin 과 Cleric 은 DPS 밴드 실패로 판정하지 않는다.

Automation: `server/tests/class-balance.test.ts`,
`IdleProject.Combat.Formulas.ClassDamage`

## 시나리오 3: Berserker 와 Summoner 튜닝 anchor 가 고정된다

Given PR #60 은 Berserker 와 Summoner 의 성장/스킬 계수를 조정했다.

When 클라이언트 `FClassGrowth` 와 서버 `stats.ts` 를 비교한다.

Then Berserker STR level bonus 는 `1.5` 이고 이전 `1.8` 로 돌아가지 않는다.

And Berserker STR 은 Warrior 의 `1.4` 보다 약간 높지만 strict-better
프로필이 되지 않도록 방어/역할 trade-off 를 유지한다.

And Summoner INT level bonus 는 `1.45`, CON level bonus 는 `0.52` 이다.

And Summoner 는 Mage 보다 높은 raw INT 성장으로 상위호환이 되지 않는다.

When 클라이언트 `SkillComponent` 와 서버 `skills.ts` 를 비교한다.

Then Berserker active damage coefficients 는 `2.35`, `1.65`, `2.05` 이다.

And Summoner active damage coefficients 는 `1.9`, `1.45`, `2.0` 이다.

Automation: `IdleProject.Character.StatFormulas.DefaultPrimaryStats`,
`IdleProject.Character.StatFormulas.DeriveStats`,
`IdleProject.Combat.Skills.DefinitionParity`,
`server/src/core/formulas/stats.test.ts`,
`server/src/core/data/skills.test.ts`

## 시나리오 4: 클라이언트와 서버 공식 parity 가 8직업 전체를 검증한다

Given 클라이언트 `StatFormulas.cpp`, `SkillComponent.cpp` 와 서버
`stats.ts`, `skills.ts` 는 같은 classId 와 계수를 가진다.

When parity 테스트가 classId 1부터 8까지 stats 와 skills 를 읽는다.

Then 8직업 성장 anchor 는 `Math.fround` 허용 범위 안에서 일치한다.

And 8직업 x 7스킬 정의의 cooldown, damageCoeff, buff, gauge, status,
element 값이 일치한다.

And Summoner 는 MagicAtk/MagicDef 라우팅을, Berserker 는
PhysAtk/PhysDef 라우팅을 유지한다.

Automation: `IdleProject.Character.StatFormulas.DeriveStats`,
`IdleProject.Combat.Formulas.ClassDamage`,
`IdleProject.Combat.Skills.DefinitionParity`,
`server/src/core/formulas/combat.test.ts`,
`server/src/core/formulas/stats.test.ts`,
`server/src/core/data/skills.test.ts`

## 시나리오 5: 상대 밸런스 튜닝이 진행 페이싱을 깨지 않는다

Given PR #60 튜닝은 절대 파워 급변이 아니라 상대 파워 보정이다.

When `npm run balance:sim` 으로 1000-run 첫 환생 리포트를 생성한다.

Then median first-rebirth time 은 5~10h 목표 안에 있다.

And p10, p90, min, max 는 기존 검토 밴드에서 급격히 벗어나지 않는다.

And class snapshot 변경은 무한 성장 곡선, 보상 곡선, 강화 비용 곡선을
수정하지 않는다.

Automation: `server/tests/balance-sim.test.ts`,
`server/tests/class-balance.test.ts`, `npm run balance:sim`

## 시나리오 6: 문서와 수동 증거가 QA 재현성을 보장한다

Given `docs/planning/05-balance-philosophy.md` 에 PR #60 class balance
guardrail 이 있다.

When QA 가 PR 본문 또는 TM 리뷰에 증거를 첨부한다.

Then `server/tests/class-balance.test.ts` 통과 로그와 class snapshot 표를
포함한다.

And `npm run balance:sim` 의 median 5~10h 유지 결과를 포함한다.

And UE Automation stdout 의 `StatFormulas`, `ClassDamage`,
`DefinitionParity` 성공 로그를 포함한다.

And 수동 PIE 검증이 필요하면 같은 장비/레벨 anchor 에서 Warrior vs
Berserker, Mage vs Summoner 비교 캡처 또는 로그를 남긴다.

Automation: markdown review, PR evidence review, Playwright/PIE visual smoke.

## 수동 재현 노트

- 밸런스 표 확인: `npm run balance:sim` 실행 후 Lv50/Lv100 class
  snapshot 에 8직업이 모두 표시되는지 확인한다.
- DPS 밴드 확인: Warrior, Mage, Archer, Thief, Berserker, Summoner 의
  effective DPS delta 가 +/-15% 안인지 기록한다.
- 역할 예외 확인: Paladin 은 탱커 보상, Cleric 은 heal/support 보상으로
  DPS 밴드 밖 허용 사유가 리포트와 문서에 남는지 확인한다.
- 상위호환 확인: Warrior/Berserker, Mage/Summoner 를 같은 조건으로
  비교해 한쪽이 DPS, CP, 생존 지표를 모두 이기지 않는지 기록한다.
- parity 확인: 서버 vitest 와 UE Automation 결과에서 stats/skills,
  `DeriveStats`, `ClassDamage` 성공 로그를 캡처한다.
- 회귀 확인: `docs/planning/05-balance-philosophy.md` 의 PR #60 anchor 와
  실제 테스트 expectation 이 같은 값을 가리키는지 확인한다.

## 기대 증거

- 서버 vitest stdout 에 `server/tests/class-balance.test.ts` 가 DPS 밴드,
  Paladin/Cleric 역할 예외, Berserker/Summoner anchor 를 통과했다고
  표시된다.
- `npm run balance:sim` 출력과
  `tools/balance-sim/reports/balance-sim-report.md` 는 8직업 class
  snapshot 과 첫 환생 median 5~10h 결과를 포함한다.
- 서버 vitest stdout 에 `server/src/core/formulas/stats.test.ts`,
  `server/src/core/data/skills.test.ts`,
  `server/src/core/formulas/combat.test.ts` 가 통과했다고 표시된다.
- UE Automation stdout 에 `IdleProject.Character.StatFormulas`,
  `IdleProject.Combat.Formulas.ClassDamage`,
  `IdleProject.Combat.Skills.DefinitionParity` 가 `Result={Success}` 로
  표시된다.
- 수동 증거는 class snapshot 표, Warrior/Berserker 비교,
  Mage/Summoner 비교, Paladin/Cleric 역할 예외 메모를 포함한다.

## 검증 명령

<!-- markdownlint-disable MD013 -->

```powershell
Push-Location server
npm run build
npm test -- tests/class-balance.test.ts
npm test -- tests/balance-sim.test.ts
npm test -- `
  src/core/formulas/stats.test.ts `
  src/core/data/skills.test.ts `
  src/core/formulas/combat.test.ts
npm run balance:sim
Pop-Location

$UE = 'C:\Program Files\Epic Games\UE_5.7\Engine'
$Project = 'C:\game\idle game\repo\client\IdleProject.uproject'

& "$UE\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project=$Project `
  -WaitMutex

$StatsTests = 'Automation RunTests IdleProject.Character.StatFormulas; '
$ClassBalanceTests = $StatsTests +
  'Automation RunTests IdleProject.Combat.Formulas.ClassDamage; ' +
  'Automation RunTests IdleProject.Combat.Skills.DefinitionParity; Quit'

& "$UE\Binaries\Win64\UnrealEditor-Cmd.exe" `
  $Project `
  -unattended -nop4 -nosplash -NullRHI `
  -ExecCmds=$ClassBalanceTests `
  -TestExit='Automation Test Queue Empty'
```

<!-- markdownlint-enable MD013 -->
