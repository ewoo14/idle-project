# 밸런스 철학 — idle-project

<!-- markdownlint-disable MD013 MD022 MD031 MD032 MD040 MD041 -->

> 본 문서는 수치 자체보다 **수치를 결정하는 원칙** 을 다룹니다. 구체 곡선/계수는 밸런스 슬라이스 PR (마일스톤별) 에서 데이터 테이블에 반영됩니다.

---

## 부록: M2 인벤토리 V1 수치

`client/Content/Data/ItemDB.csv`는 PR #9 범위의 8슬롯 × 3등급 기본 장비 24종을 정의한다. Weapon은 ATK 100%, Helmet/Top/Bottom/Shoes/Gloves/Cloak은 DEF 70% + HP 30%, Accessory는 ATK/DEF/HP 균형형으로 둔다.

| 등급 | ATK 평균 | DEF 평균 | HP 평균 | MaxEnhance |
| --- | ---: | ---: | ---: | ---: |
| Common | 1.0 | 2.5 | 11.25 | 5 |
| Rare | 1.7 | 4.25 | 19.13 | 50 |
| Epic | 2.3 | 5.75 | 25.88 | 50 |
| Unique | 2.75 | 6.88 | 30.94 | 50 |
| Legendary | 3.2 | 8.0 | 36.0 | 50 |
| Transcendent | 3.85 | 9.63 | 43.31 | 50 |
| Mythic | 4.5 | 11.25 | 50.63 | 50 |

PowerScore 공식은 UE5 `FItemPowerScore::Compute`와 서버 `computeItemPowerScore`가 공유한다.

```text
PowerScore = (ATK + DEF + HP / 10) × (1 + EnhanceLevel × 0.1)
```

자동 장착 점수 비교 흐름:

1. 드롭 장비가 `None` 슬롯/등급이면 인벤토리에 넣지 않는다.
2. 장비가 최대 100칸을 넘지 않으면 인벤토리에 추가한다.
3. 같은 슬롯의 현재 장착 장비와 신규 장비의 PowerScore를 비교한다.
4. 신규 PowerScore가 더 높을 때만 자동 장착하고 HUD 장비 요약을 갱신한다.
5. 같은 등급이어도 낮은 PowerScore면 기존 장비를 유지한다.

드롭 밀도 기준:

- 현재 슬라임 환경은 시간당 약 1200마리 처치 가정이다.
- 1마리 처치 시 장비 드롭 5%면 1시간 기대 드롭은 약 60개다.
- 이 수치는 M2 인벤토리 자동 흡수/자동 장착 체감 검증용이며, 실제 경제 확정값은 후속 밸런스 시뮬레이션에서 보정한다.

---

## 1. 큰 원칙

1. **항상 진행감(progress) 이 있어야 한다** — 1분이 지나도 무언가는 변한다 (골드, EXP, 작은 드롭).
2. **하지만 지수적 진행이 영원해선 안 된다** — 5~10시간마다 환생으로 인플레이션 리셋.
3. **선택의 무게** — 캐릭터 빌드, 장비 강화 우선순위, 환생 타이밍에 의미가 있어야 한다.
4. **불운(RNG) 의 보호** — 강화/잠재에 확률이 있되 천장(pity) 또는 결정적 대안이 존재.
5. **시간 = 자원** — 오프라인 보상이 액티브 플레이의 70~80% 효율 (액티브 인센티브 유지).
6. **서버 권위적 핵심 진행** — 클라이언트 임의 조작이 진행에 영향을 주지 못한다.

---

## 2. 성장 곡선

### 2.1 경험치 곡선 (레벨 1~200)

> **초안 — M2 시뮬레이터 (`tools/balance-sim/`) 도착 시 곡선/계수 모두 정밀화 예정.** 본 절의 모든 수치는 1차 가이드.

- **공식 (초안)**: `EXP_to_next(n) = round(50 × n^1.7 + 100 × n)`
- 공식 기반 근사값 (반올림 후):
  - 레벨 10: 약 **3,500 EXP**
  - 레벨 50: 약 **51,000 EXP**
  - 레벨 100: 약 **261,000 EXP**
  - 레벨 200: 약 **832,000 EXP**
- 후속 보정 후보: 곡선이 너무 완만하다면 지수를 1.85~2.0 으로 상향. 결정은 시뮬레이터 결과 + 환생 도달 5~10h 타깃과 함께.
- 사냥터별 시간당 EXP 는 권장 레벨 기준 1시간 ≒ 1레벨 (초반) → 5레벨/시간 (후반 동일 사냥터에서 환생 후 폭발 성장)

### 2.2 골드 / 드롭률
- 시간당 골드: 사냥터 권장 레벨 × 1,000 (기본) × 펫/장비 보너스
- 드롭률 (장비): shared `drop.ts` table keeps total probability at 1.0:
  Lv100 = None 2%, Common 56.8%, Rare 30%, Epic 6%, Unique 2.5%,
  Legendary 1.5%, Transcendent 0.7%, Mythic 0.5%.

### 2.3 강화 비용 / 성공률

성공률 + **자원 소비량** 동시 표기 (성공/실패 무관 시도당 소비).

| 강화 단계 | 성공률 | 시도당 골드 | 시도당 강화석 | 시도당 신비석 | 실패 결과 |
| --- | --- | --- | --- | --- | --- |
| +1 ~ +5 | 100% | 100 × 단계 | 0 | 0 | — |
| +6 ~ +10 | 90% | 500 × 단계 | 1 | 0 | 강화석 소모 (단계 유지) |
| +11 | 70% | 5,000 | 3 | 0 | 단계 -1 |
| +12 | 60% | 8,000 | 5 | 0 | 단계 -1 |
| +13 | 50% | 12,000 | 8 | 1 | 단계 -2 + 신비석 -1 |
| +14 | 40% | 20,000 | 12 | 2 | 단계 -2 + 신비석 -2 |
| +15 | 30% | 35,000 | 18 | 3 | 단계 -3 + 신비석 -3 |

> 시도당 자원은 성공/실패 모두 동일하게 소비. 실패 결과의 "신비석 -N" 은 시도당 소비와 별개 (실패 시 추가 손실).

**천장 (pity)**: +11 이상은 25회, +13 이상은 10회 연속 실패 시 자동 성공 (운빨 보호).

> 위 수치는 1차 시안. 시뮬레이터 도착 (M2) 이후 1000회 환생 도달 시간 분포로 보정 예정.

---

## 3. 데미지 / 전투 공식

### 3.1 기본 데미지
```
일반 공격 데미지 = (캐릭터 ATK - 몬스터 DEF × 0.6) × (1 + 강화 보너스) × 무기 계수
```
- 최소 보장: `ATK × 0.05` (방어가 압도해도 1 이상)
- 크리티컬 시: × (1 + CRIT DMG)

### 3.2 몬스터 스탯 곡선
```
몬스터 HP  = round(권장레벨 × 80 × (1.08^권장레벨))
몬스터 ATK = round(권장레벨 × 6 × (1.05^권장레벨))
몬스터 EXP 보상 = round(권장레벨 × 12)
몬스터 골드 보상 = round(권장레벨 × 8)
```

### 3.3 보스 스탯
- 일반 몬스터 × 50 (HP), × 3 (ATK), × 200 (보상)
- 보스는 패턴(텔레그래프) + 페이즈 (50% / 25%) 보유

### 3.4 직업별 V1 전투 포지션

PR #21의 마법사/궁수 추가는 같은 자동 전투 루프 안에서 역할 차이를
만들되, 한 직업이 모든 지표를 독점하지 않도록 다음 기준을 둔다.

<!-- markdownlint-disable MD013 -->

| 직업 | 핵심 축 | V1 스킬 방향 | 밸런스 리스크 | 가드레일 |
| --- | --- | --- | --- | --- |
| 전사 | STR/CON, 물리 안정성 | 단일 피해, 방어 버프, 돌진형 피해 | 기본 생존이 너무 높아 보스 패턴을 무시 | 방어 버프는 짧은 지속과 명확한 쿨다운 유지 |
| 마법사 | INT/WIS, 마법 DPS | 단일 마법, 광역 연쇄, 보호막, 광역 궁극기 | 광역 계수가 과하면 사냥터 EXP/골드가 과속 | AoE 계수는 단일기보다 낮게 두고 쿨다운을 길게 유지 |
| 궁수 | DEX/LUK, 치명/공속 | 단일 사격, 광역 화살비, 집중 버프, 단일 궁극기 | 치명 기대값과 공속 버프가 곱연산으로 폭주 | 치명 패시브는 고정 p.p. 증가, 공속 버프는 짧은 지속으로 제한 |

V1 기준 마법사는 "최고 AoE DPS 후보"지만 방어 버프 수치와 평타
안정성은 전사보다 낮게 둔다. 궁수는 "단일 목표 기대 DPS 후보"지만
치명/공속이 모두 붙는 대신 광역 계수와 방어 보정은 낮게 유지한다.
세 직업의 최종 수치는 `server/src/core/data/skills.ts`와
`USkillComponent::LoadDefault*Skills`의 DefinitionParity 테스트가 같은
값을 검증한다.

PR #29 extends this parity rule to all five V1 classes. Warrior, Mage, Archer,
Thief, and Cleric each own exactly seven skill definitions, for 35 total
definitions across server TypeScript and client `USkillComponent` automation.
The parity guard checks `id`, `classId`, `type`, `effectType`, cooldown,
coefficients, buff fields, and ultimate gauge gain fields.

Heal is a Cleric-only V1 effect type. `heal` restores
`round(MaxHp * 0.20)` on a 6 second cooldown, while `sanctuary` spends ultimate
gauge and restores `round(MaxHp * 0.40)`. Heal effects must clamp at `MaxHp`,
must not damage the selected target, and must remain budgeted as recovery
rather than DPS when comparing the five class kits.

---

### 3.5 Combat Class Differential V1 Anchors

PR #26 keeps the physical mitigation curve unchanged and adds class-specific
inputs before mitigation:

```text
physicalDamage = max(Atk * 0.05, Atk - Def * 0.6)
magicDamage = max(MagicAtk * 0.05, MagicAtk - MagicDef * 0.6)
critRate = clamp(LUK * 0.002 + equipmentCritRate + passiveCritRate, 0, 1)
critDmg = clamp(1.5 + LUK * 0.001 + equipmentCritDmg, 1, 3)
finalDamage = baseDamage * (isCrit ? critDmg : 1)
expectedDamage = baseDamage * (1 + critRate * (critDmg - 1))
```

Server `computeClassDamage` mirrors client
`FCombatFormulas::ComputeDamage(FDerivedStats, EClassId, PhysDef, MagicDef)`:
Mage uses `MagicAtk` vs `MagicDef`; Warrior and Archer use `PhysAtk` vs
`PhysDef`. Basic attacks remain physical for every class. Mage damage skills use
the magic path; non-Mage damage skills use the physical path. All damage skills
apply the same crit helper after mitigation.

| Anchor | Inputs | Base damage | Crit expectation |
| --- | --- | ---: | ---: |
| Warrior physical | `PhysAtk=40`, `PhysDef=10` | 34.0 | unchanged unless crit stats are present |
| Mage spell | `MagicAtk=40`, `MagicDef=10` | 34.0 | same crit helper after magic mitigation |
| Archer physical | `PhysAtk=40`, `PhysDef=10`, `CritRate=0.25`, `CritDmg=1.8` | 34.0 | 40.8 expected average |
| Guaranteed crit | `baseDamage=34`, `CritRate=1.0`, `CritDmg=1.8` | 34.0 | 61.2 |

DPS review guardrails:

- Mage should gain practical AoE/elemental value from `MagicAtk` without making
  `MagicDef` irrelevant; single-target magic anchors should stay close to
  physical anchors at equal offensive/defensive ratings.
- Archer may exceed Warrior single-target expected DPS through crit, but only by
  the expected-value curve above. If `CritRate` and `AtkSpeed` buffs stack past
  the Warrior baseline by more than 25% in simulator output, review coefficients
  before shipping.
- Warrior remains the stable physical baseline; defense and survival buffs are
  compared against this row before any class-wide nerf.

Automation coverage:

- `server/src/core/formulas/combat.test.ts` verifies physical, magic, crit, and
  class-damage parity anchors.
- `client/Source/IdleProject/Tests/CombatTests.cpp` verifies
  `ComputeMagicDamage`, `ApplyCrit`, class damage routing, skill magic damage,
  skill crit damage, and `InitializeCombat` extended stats.

---

### 3.6 Thief / Cleric Class V1 Anchors

PR #29 completes the five-class V1 client skill set. Thief uses the existing
physical path with DEX/LUK identity: direct physical skills, one AoE, one
dash-style back attack, passive dodge, passive crit, and an attack-speed ultimate
buff. Cleric uses the magic path with WIS/INT identity: a magic damage skill,
self heal, attack-speed blessing, defense purification, WIS-driven magic attack
passive, max-HP passive, and a heal ultimate.

Client skill values are mirrored in `client/Content/Data/SkillDB.csv` and
`USkillComponent::LoadDefault*Skills`.

| Class | Anchor | Value |
| --- | --- | ---: |
| Thief passive dodge | `nimble_hands` adds flat Dodge | +0.05 |
| Thief passive crit | `lucky_instinct` adds flat CritRate | +0.05 |
| Thief ultimate | `assassinate` damage coefficient | 5.3 |
| Cleric heal | `heal` restores MaxHp ratio | 0.20 |
| Cleric ultimate heal | `sanctuary` restores MaxHp ratio | 0.40 |
| Cleric passive HP | `divine_vitality` multiplies max HP | 1.20 |

Routing guardrails:

- Cleric damage skills use `MagicAtk` vs `MagicDef`, matching Mage mitigation.
- Thief damage skills use `Atk` vs `Def`, matching Warrior/Archer mitigation.
- Heal never enters the damage path and clamps at `MaxHp`.
- Server SkillDB parity remains a backend handoff item unless that slice owns
  `server/src/core/data/skills.ts`.

### 3.7 Stat Allocation V1 Anchors

PR #34 defines the client V1 stat allocation rule: each level-up to level 2 or
higher grants 5 allocatable primary stat points. The cumulative allocation
budget for level `n` is `5 * max(n - 1, 0)`.

Allocated points are added to `DefaultPrimaryStats(ClassId, Level)` before
`DeriveStats`, equipment bonuses, rebirth bonus points, and skill passives are
folded into combat stats. This keeps the existing class growth curve intact
when allocation is zero, while making STR/DEX/INT/WIS/CON/LUK choices affect
the same derived formulas used by class and equipment progression.

V1 reset is free and returns allocated points to the available pool. Rebirth
resets both available and allocated stat points because it resets character
level to 1; rebirth bonus points remain a separate permanent progression input.

Per-level grant is fixed at 5 points for every level-up after level 1 in V1.
The server mirror in `server/src/core/formulas/statPoints.ts` and the client
formula in `FStatPointFormula` must stay definition-parity guarded. Invalid
fractional server levels are rejected before formula evaluation so backend
simulation cannot drift from the client `int32` boundary.

Primary allocation impact uses the existing `DeriveStats` paths:

- STR: `PhysAtk += 2` per point before equipment and rebirth bonuses.
- DEX: raises `AtkSpeed`, `MoveSpeed`, `Dodge`, and `Accuracy` through the
  existing rounded rate formulas.
- INT: raises `MagicAtk`, `Mp`, and `MagicDef` through the existing formulas.
- WIS: raises `MagicAtk`, `Mp`, and `MagicDef` through the existing formulas.
- CON: raises `Hp` by 10 per point and also contributes to `PhysDef`.
- LUK: raises `CritRate`, `CritDmg`, and `Dodge` through the existing formulas.

No paid reset, refund penalty, allocation cap, or server-authoritative spend
validation ships in this V1. Those remain follow-up economy and persistence
items after player telemetry exists.

### 3.8 Status / Element V1 Anchors

PR #30 adds deterministic status effects and element multipliers to the existing
damage path.

Rules:

- Status effects are `None`, `Poison`, `Burn`, and `Freeze`.
- `Poison` and `Burn` deal flat magic DoT once per second while active.
- `Freeze` applies an attack-speed slow by `StatusMagnitude` and removes the
  exact applied penalty when the status expires.
- Skill elements are `None`, `Fire`, `Ice`, `Lightning`, and `Holy`.
- `ComputeElementMultiplier(skillElement, targetWeakElement)` returns `1.5`
  when the skill matches the target weakness, `0.5` for opposed pairs
  (`Fire`/`Ice`, `Lightning`/`Holy`), and `1.0` for neutral or `None`.

V1 skill assignments:

| Skill | Status | Element |
| --- | --- | --- |
| `arcane_bolt` | Burn, 3s, 4 damage/tick | Fire |
| `chain_lightning` | None | Lightning |
| `meteor` | Freeze, 2s, 25% slow | Ice |
| `shadow_stab` | Poison, 3s, 3 damage/tick | None |
| `smoke_bomb` | Poison, 3s, 2 damage/tick | None |
| `holy_smite` | None | Holy |

Monster anchors:

- Normal slime placeholder weakness: `Fire`.
- Chapter 1 boss placeholder weakness: `Holy`.

Automation coverage:

- Client: element multiplier, DoT ticking/expiry, freeze slow/expiry, and skill
  element/status application.
- Server: formula multiplier anchors and SkillDB status/element parity.

---

## 4. 환생 (Rebirth) 경제

### 4.1 환생 1회 보상
- 환생 포인트: 5점 (능력치 분배, 영구)
- 환생 전용 스킬 슬롯 1칸 해금 (1회당)
- 환생 횟수에 따른 시간당 보상 보너스: +5% (누적 가산)

### 4.2 환생 곡선
- 환생 N회 후 동일 챕터 도달 시간: `0.85^N` (감소 → 환생 4회 시 약 절반)
- PR #46부터 환생 보상은 감소형이 아니라 선형 prestige 보상이다:
  `5 + max(0, rebirthCount) * 2 + floor((max(100, levelAtRebirth) - 100) / 10)`.
- 1회차 Lv100 보상은 기존과 같은 5점이며, 이전 환생 횟수와 Lv100 초과
  레벨을 반영해 장기 반복 보상을 만든다. 예: `(0,100)=5`, `(4,100)=13`,
  `(0,150)=10`, `(4,150)=18`.
- `RebirthBonusPoints`는 영구 스탯 입력으로 유지된다. 각 포인트는 기존
  `DeriveStats` 경로에서 HP +10, PhysAtk +2를 제공한다.
- 첫 환생 도달 시간은 PR #32 balance-sim seed `23` 1000회 결과의 중앙값
  5.324h를 기준선으로 둔다. PR #46은 보상 계산만 바꾸므로 EXP/골드 곡선을
  직접 변경하지 않지만, prestige 반복 구간의 체감 가속이 커질 수 있어
  후속 보스/보상/강화 변경 시 `npm run balance:sim` 결과를 함께 첨부한다.

### 4.3 보존 / 초기화
**환생 후 보존**:
- 환생 횟수, 환생 포인트 (영구)
- 환생 전용 스킬 슬롯
- 도감(컬렉션)
- 골드 일부 (보유의 10%, 환생 횟수당 +1% 까지)
- 펫 (전부 보존)
- 시즌 패스 진행

**환생 후 초기화**:
- 캐릭터 레벨 → 1
- 일반 스킬 트리 → 0
- 장비 (단, Mythic 1개 보존 가능)
- 일일 퀘스트 진행

---

## 5. 자원 / 화폐

| 자원 | 획득 | 사용처 |
| --- | --- | --- |
| 골드 | 사냥, 판매, 일일 보상 | 강화, 상점 |
| 강화석 | 드롭 (5%), 일일 던전 | +1~+10 강화 |
| 신비석 | 드롭 (0.5%), 보스 보상 | +11~+15 강화 |
| 도감 포인트 | 도감 완료 | 영구 능력치 보너스 |
| 환생 포인트 | 환생 | 영구 능력치 |
| 시즌 토큰 | 일일/주간 미션 | 시즌 패스 보상 |

**가챠/유료 자원 없음** (MVP/1차 출시 기준). 추후 검토.

### 5.1 펫 + 시즌 패스 V1 수치

PR #22의 V1 수치는 서버 `server/src/core/data/pets.ts`,
`server/src/core/data/season.ts`와 UE C++ 미러가 같은 값을 유지한다.
펫은 한 번에 1마리만 장착하며, 기본 보유 목록은 다음 2종이다.

| PetId | 이름 | 보너스 | 적용 |
| --- | --- | --- | --- |
| `dog` | 강아지 | 골드 +20% | 골드 획득량에 곱한 뒤 내림 |
| `bird` | 새 | 드롭률 +15% | 기본 드롭 확률에 곱하고 100% 상한 |

시즌 패스 베타는 시즌 1 무료 트랙 10티어만 제공한다. 퀘스트 보상 수령
성공 시 V1 클라이언트는 시즌 토큰 +10을 적립하며, 서버 권위 보정은 후속
운영 단계에서 확장한다. 티어 보상은 한 번만 수령 가능하다.

| 티어 | 필요 토큰 | 보상 |
| ---: | ---: | --- |
| 1 | 10 | 500 gold |
| 2 | 25 | 1,000 gold |
| 3 | 45 | 300 EXP |
| 4 | 70 | 1,800 gold |
| 5 | 100 | 650 EXP |
| 6 | 135 | 3,000 gold |
| 7 | 175 | 1,100 EXP |
| 8 | 220 | 4,800 gold |
| 9 | 270 | 1,750 EXP |
| 10 | 325 | 7,500 gold |

운영 리스크: 골드 +20%는 초반 강화 비용 체감에 직접 영향을 주므로
인벤토리/강화 골드 소모와 함께 회귀 확인한다. 드롭 +15%는 장비 공급량을
늘리지만 기본 드롭률 5% 기준 5.75%로 유지되어 V1 경제를 즉시 깨지 않는
범위로 둔다.

---

## 6. 인플레이션 관리

| 위험 | 대응 |
| --- | --- |
| 무한 환생으로 능력치 폭주 | 환생당 보상 점진 감소, 능력치 분배 상한 |
| 후반 장비가 초반을 무력화 | 사냥터별 권장 레벨로 효율 감소 (오버레벨 -50% EXP) |
| 신비석 시장 폭락 | 보스/일일 보상 외 추가 발생 차단 |
| 도감 완전 컴플리트 후 동기 상실 | 시즌 도감 + 환생 도감 분리, 영구 갱신 |

---

## 7. 액티브 vs 오프라인 균형

| 항목 | 액티브 | 오프라인 |
| --- | --- | --- |
| 시간당 EXP | 100% | 70% (환생 보너스로 최대 80%) |
| 시간당 골드 | 100% | 70% |
| 드롭률 | 100% | 70% |
| 보스 진행 | 액티브 전용 | 자동 진행 불가 |
| 일일 퀘스트 | 액티브 전용 | — |
| 환생 | 액티브 전용 | — |

→ **액티브로만 가능한 것** (보스, 환생, 일일 퀘스트) 으로 접속 동기 유지.

---

## 8. 밸런스 검증 절차

1. **시뮬레이터**: `tools/balance-sim/` (M2 이후 추가) — 1000회 시뮬레이션으로 환생 도달 시간 분포 계산.
2. **이상치 알람**: 환생 도달 < 3시간 또는 > 20시간 시 경고.
3. **PR 게이트**: 밸런스 슬라이스 PR 은 시뮬레이션 결과 첨부 의무.

---

## 9. 변경 정책

- 핵심 공식 변경은 PM 승인 필수.
- 배포된 빌드에 대한 너프(nerf) 는 환영 모달로 사전 공지 + 환생 포인트 보너스 보상.
- 버프(buff) 는 자유 (단, 인플레이션 부담 확인).

---

## 10. 참고 수치 (벤치마크)

- AFK Arena: 환생 ≈ 7일 (모바일)
- Idle Heroes: 영웅 진화 ≈ 매일
- 메이플스토리 키우기: 환생 ≈ 2~5시간

본 게임은 **환생 5~10시간** 을 1차 타깃 (PC 세션 기반, 짧지만 자주).

---

## 부록 — 공식 구현 참조

PR #2 백엔드 V1부터 서버에서 사용하는 공식의 단일 source of truth는 아래 TypeScript 미러 파일이다.

- `server/src/core/formulas/level.ts`: 레벨 경험치, 누적 경험치, 강화 성공률, 강화 자원 소비
- `server/src/core/formulas/stats.ts`: 직업별 1차 능력치와 2차 능력치 파생 공식
- `client/Source/IdleProject/CharacterSystem/LevelFormulas.h/cpp`: UE5 클라이언트 레벨 경험치/강화 성공률 미러
- `client/Content/Data/LevelCurveDB.csv`: UE5 DataTable 임포트용 레벨 경험치 앵커 CSV

기획 문서의 수식 또는 표를 조정할 때는 위 파일의 단위 테스트를 함께 갱신한다. M2 밸런스 시뮬레이터가 도착하면 `level.ts`의 경험치 곡선 보정항과 `stats.ts`의 직업별 성장 보너스를 시뮬레이션 결과에 맞춰 재보정한다.

---

## 부록 — M1 자동 전투 V1 수치 (PR #6)

| 항목 | 기준 수치 | 비고 |
| --- | --- | --- |
| Monster 기본 수치 (슬라임) | HP 50 / Atk 8 / Def 5 / Reward Gold 10~15 | `AIdleMonster` placeholder 기본값 |
| 전사 레벨 1 공격/방어 | PhysAtk 24 / PhysDef 16.0 | `StatFormulas` 결과 |
| 1마리 처치 시간 추정 | `(50 HP) / (24 - 5 * 0.6 = 21 dmg)` ≈ 3회 공격 → `3 * atkSpeed(1.0s)` = 약 3초 | `CombatFormulas::ComputeDamage` 기준 |
| 시간당 처치 수 | 약 1200마리 | 5마리 spawn, 무한 respawn 단순 가정 |
| 시간당 골드 | 약 14400~18000 gold | 1200마리 * 10~15 gold |
| 시간당 EXP | 약 14400 EXP | monster 1마리 = `level * 12 = 12 EXP` (PR #1 §3.2.2 미러) |
| 레벨업 시간 | 레벨 1→2 필요치 3506 EXP 기준 약 14분 | M1 단순 환경 기준. 후속 PR 인벤토리/스킬 추가 후 보정 |

M1 자동 전투 V1은 “움직이고 보상이 누적되는 체감” 검증이 목적이다. 위 수치는 실제 경제 밸런스 확정값이 아니라, M2 밸런스 시뮬레이터와 인벤토리/스킬/오프라인 보상 도입 후 재보정할 1차 기준선이다.

---

## PR #23 Balance Simulator V1 Result

`tools/balance-sim/` now runs a deterministic 1000-sample simulation for the
first rebirth target: Lv100 plus one boss clear. The simulator imports the
server formula sources directly:

- `server/src/core/formulas/level.ts`
- `server/src/core/formulas/combat.ts`
- `server/src/core/formulas/stats.ts`
- `server/src/core/formulas/offline.ts`

Current seed `23` result:

| Metric | Hours |
| --- | ---: |
| p10 | 6.074 |
| median | 6.529 |
| p90 | 6.979 |

The median is inside the 5-10h target and the distribution remains inside the
3-20h review band. No EXP curve change is applied in this slice. In particular,
do not raise the `level.ts` exponent toward 1.85-2.0 yet; keep the current curve
and re-run the simulator after the next equipment, enhancement, or monster-data
change.

## PR #33 Equipment Enhancement V1

Client V1 now uses a long-tail gold sink for the existing `EnhanceLevel`
equipment multiplier, expanded from 0..5 to 0..50 in PR #44.

| Current level | Cost | Success rate |
| ---: | ---: | ---: |
| 0 | 100 | 95% |
| 1 | 400 | 93.2% |
| 4 | 2500 | 87.8% |
| 10 | 12100 | 77% |
| 25 | 67600 | 50% |
| 40 | 168100 | 23% |
| 49 | 250000 | 6.8% |
| 50 | 0 | 0% |

The cost formula is `100 * (CurrentLevel + 1)^2` before max level. The success
curve is `clamp(0.95 - CurrentLevel * 0.018, 0.05, 0.95)`, with level 50
returning 0 because there is no next enhancement. Failure does not destroy or
downgrade equipment in V1; it consumes the attempt cost and records enhancement
quest progress. The stat payoff remains the existing `1 + EnhanceLevel * 0.1`
equipment bonus multiplier, making Lv50 equipment apply a 6.0x multiplier.

Sensitivity notes:

- EXP curve: current curve is acceptable for the first rebirth target.
- Gold per hour: keep active gold tuning stable until enhancement spend data is
  available.
- Offline efficiency: current sampled 70-80% band keeps idle progress below
  active play while staying relevant.
- Simulator pressure check: the deterministic 1000-run balance report imports
  `server/src/core/formulas/enhance.ts` and models +0 to +50 enhancement spend.
  The minimum all-success cost is 4,292,500 gold, while expected cost using
  `cost / successRate` is 22,717,602.46 gold. Against the sampled median Lv50
  active/idle blended gold rate, a single Common +0 to +50 path is about 34.7h
  of income. Treat this as the first long-tail gold sink baseline; high-rarity
  and multi-slot enhancement intentionally extend beyond first-rebirth pacing.
- Reward parity check: PR #32 reward scaling remains aligned with monster HP
  scaling because both use `1 + globalStageIndex * 0.15`. Chapter 1 normal
  reward-per-HP pressure stays stable across 1-1 to 1-5; the 8x boss reward
  bonus remains the intentional stage-cap spike.

---

## PR #36 Loot Depth V1

Client drop rarity now affects generated equipment stats through
`FDropFormula::GetRarityStatMultiplier`:

| Rarity | Stat multiplier |
| --- | ---: |
| None | 0.0 |
| Common | 1.0 |
| Rare | 1.7 |
| Epic | 2.3 |
| Unique | 2.75 |
| Legendary | 3.2 |
| Transcendent | 3.85 |
| Mythic | 4.5 |

`FDropFormula::RollRarityForLevel` keeps early drops Common-heavy while moving
some Common weight into Rare, Epic, Unique, Legendary, Transcendent, and Mythic
as monster level approaches 100. The current level 1 distribution is
2% none, 70% common, and 28% rare. At level 100 the intended distribution is
2% none, 56.8% common, 30% rare, 6% epic, 2.5% unique, 1.5% legendary,
0.7% transcendent, and 0.5% Mythic. The seven active rarity rows plus None sum
to exactly 1.0, Unique remains below Epic drop pressure, and Transcendent
remains below Legendary drop pressure.

`FDropFormula::ComputeItemBonus` preserves the existing slot split: weapons put
100% of the scaled bonus into ATK, armor slots put 70% into DEF and 300% into
HP, and accessories split 50% ATK, 30% DEF, and 200% HP. PowerScore remains the
existing `(ATK + DEF + HP / 10) * (1 + EnhanceLevel * 0.1)` formula, so rarity
improves auto-equip decisions through the generated stat bonuses rather than a
separate score override.

Parity anchors:

- Lv1 Common with variance `1.0` keeps the legacy base bonus: weapon ATK `1.0`,
  armor DEF `0.7` / HP `3.0`, accessory ATK `0.5` / DEF `0.3` / HP `2.0`.
- Lv100 variance `1.0` produces base bonuses of Rare `170`, Epic `230`,
  Unique `275`, Legendary `320`, Transcendent `385`, and Mythic `450`
  before slot split.
- Enhancement remains a separate #33/#44 multiplier. A Legendary Lv100 weapon
  at +50 has PowerScore `round(320 * 6.0) = 1920`; rarity does not bypass the
  existing `EnhanceLevel` formula.
- A Mythic Lv100 weapon at +50 has PowerScore `round(450 * 6.0) = 2700`; this
  extends the named item ceiling while keeping infinite growth on stage level
  and enhancement.
- Server `drop.ts` intentionally uses `Math.fround` at the same public formula
  boundaries as client `float` arithmetic so parity tests can compare exact
  generated bonus values, not just rounded display numbers.

Server parity mirror:

- `server/src/core/formulas/drop.ts` mirrors rarity multipliers, level-scaled
  rarity rolls, and slot-based item bonus generation.
- `server/src/core/formulas/drop.test.ts` locks level 1 and level 100 rarity
  anchors plus weapon, armor, and accessory bonus anchors against
  `FDropFormula`.

Automation coverage:

- `IdleProject.Inventory.DropFormula.RarityMultiplier`
- `IdleProject.Inventory.DropFormula.LevelRarityTrend`
- `IdleProject.Inventory.Rarity.MythicHudMapping`
- `IdleProject.Inventory.DropFormula.ComputeItemBonus`
- `IdleProject.Inventory.ItemFactory.HighLevelExpandedRarity`

---

## PR #27 Skill Rank Points V1 Anchors

Skill rank points are a client-combat V1 progression layer, separate from stat
points and rebirth points.

Rules:

- `AIdleCharacter::HandleLevelUp` grants `USkillComponent` exactly 1 skill
  point per level-up event.
- Each skill has an independent rank stored by `SkillId`.
- `MaxRank` is 50.
- `RankUpSkill(SkillId)` spends 1 skill point and increases that skill by 1
  rank when the skill is loaded, points are available, and rank is below max.
- Damage skills use
  `effectiveDamageCoeff = baseDamageCoeff * (1 + rank * 0.10)`.
- Cooldown lookups use
  `effectiveCooldown = max(0.1, baseCooldown * (1 - rank * 0.05))` for skills
  with positive base cooldown. Zero-cooldown skills stay zero.

Worked examples:

| Rank | Damage multiplier | Cooldown multiplier |
| ---: | ---: | ---: |
| 0 | 1.00x | 1.00x |
| 1 | 1.10x | 0.95x |
| 2 | 1.20x | 0.90x |
| 3 | 1.30x | 0.85x |
| 4 | 1.40x | 0.80x |
| 5 | 1.50x | 0.75x |
| 20 | 3.00x | floor 0.1s |
| 50 | 6.00x | floor 0.1s |

For `heavy_strike` (`baseDamageCoeff=2.5`, `baseCooldown=4.0s`), rank 5
therefore resolves to `effectiveDamageCoeff=3.75` and
`effectiveCooldown=3.0s`, preserving the original low-rank behavior. Rank 20
reaches the cooldown floor (`0.1s`), and rank 50 resolves to
`effectiveDamageCoeff=15.0` with the same cooldown floor. This extends the
skill sink to 50 points while keeping the rank 0 through 5 curve unchanged.

V1 policy:

- Class switching keeps the global point/rank state in memory. A rank only
  applies when its matching `SkillId` exists in the currently loaded skill set.
- Rebirth-specific reset or preservation rules are deferred to a later slice.
- Server persistence is still deferred, but `server/src/core/formulas/skillRank.ts`
  mirrors the client damage/cooldown rank formulas for balance and parity tests.

Automation coverage:

- `IdleProject.Combat.Skills.RankPoints` verifies point grants, rank-up spend,
  missing-skill rejection, no-point rejection, and max-rank clamp.
- `IdleProject.Combat.Skills.RankEffectiveValues` verifies rank 2, rank 5,
  rank 20 cooldown floor, and rank 50 damage/cooldown values plus cooldown
  remaining/ratio routing.
- `IdleProject.Combat.Skills.RankDamageApplication` verifies ranked damage uses
  the effective damage coefficient in `TickSkills`/damage application.
- `IdleProject.Character.LevelUp.GrantsSkillPoint` verifies the character
  level-up handler grants one skill point.
- `server/src/core/formulas/skillRank.test.ts` verifies the TypeScript mirror
  for MaxRank, rank 0/5/50 damage, cooldown floor, and zero-cooldown skills.

---

## PR #31 스테이지 진행 V1 수치

챕터 1은 V1 기준 5개 스테이지로 고정한다. 전역 스테이지 인덱스는
`(chapter - 1) * 5 + (stage - 1)`이며, 몬스터 HP/ATK와 보상 배율은
`1.0 + globalStageIndex * 0.15`를 사용한다. 1-1은 1.00배, 1-5는 1.60배다.

| 스테이지 | 처치 목표 | 보스 | 약점 |
| --- | ---: | --- | --- |
| 1-1 | 5 | 아니오 | None |
| 1-2 | 8 | 아니오 | None |
| 1-3 | 12 | 아니오 | Ice |
| 1-4 | 16 | 아니오 | Holy |
| 1-5 | 1 | 예 | Fire |

1-5 보스 처치 시 `MarkChapter1BossDefeated()`와 연계해 환생 조건의
"챕터 1 보스 격파" 플래그를 세운 뒤 2-1로 전환한다. 최종 챕터 보스는
현재 최종 스테이지에 머무르며 추가 처치를 무시한다.

PR #37 Chapter 2는 같은 처치 목표와 같은 전역 인덱스 공식을 사용한다.
서버 `stages.ts`와 클라이언트 `FStageFormula::GetStageWeakElement()`는
2-1~2-5를 전역 인덱스 5~9로 맞춘다.

| 스테이지 | idx | 처치 목표 | 보스 | 약점 |
| --- | ---: | ---: | --- | --- |
| 2-1 | 5 | 5 | 아니오 | None |
| 2-2 | 6 | 8 | 아니오 | Lightning |
| 2-3 | 7 | 12 | 아니오 | Ice |
| 2-4 | 8 | 16 | 아니오 | Fire |
| 2-5 | 9 | 1 | 예 | Holy |
<!-- markdownlint-disable MD003 MD026 -->

---

<!-- markdownlint-enable MD003 MD026 -->

## PR #32 Kill Reward Scaling V1 Result

Kill EXP and gold now scale with the same stage progression ramp used by
monster HP/ATK:

```text
RewardMultiplier(globalStageIndex) = 1 + globalStageIndex * 0.15
KillReward = round(baseReward * RewardMultiplier * (isBoss ? 8 : 1))
MonsterDropLevel = 1 + globalStageIndex
```

Chapter 1 uses global stage indexes 0 through 4:

<!-- markdownlint-disable MD013 -->

| Stage | idx | HP x | Reward x | Normal EXP | Normal Gold | Boss EXP | Boss Gold |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1-1 | 0 | 1.00 | 1.00 | 12 | 10-15 | 96 | 80-120 |
| 1-2 | 1 | 1.15 | 1.15 | 14 | 11-17 | 110 | 92-138 |
| 1-3 | 2 | 1.30 | 1.30 | 16 | 13-19 | 125 | 104-156 |
| 1-4 | 3 | 1.45 | 1.45 | 17 | 15-22 | 139 | 116-174 |
| 1-5 | 4 | 1.60 | 1.60 | 19 | 16-24 | 154 | 128-192 |

Chapter 2 continues the same ramp at global stage indexes 5 through 9:

| Stage | idx | HP x | Reward x | Normal EXP | Normal Gold | Boss EXP | Boss Gold |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 2-1 | 5 | 1.75 | 1.75 | 21 | 18-26 | 168 | 140-210 |
| 2-2 | 6 | 1.90 | 1.90 | 23 | 19-29 | 182 | 152-228 |
| 2-3 | 7 | 2.05 | 2.05 | 25 | 21-31 | 197 | 164-246 |
| 2-4 | 8 | 2.20 | 2.20 | 26 | 22-33 | 211 | 176-264 |
| 2-5 | 9 | 2.35 | 2.35 | 28 | 24-35 | 226 | 188-282 |

Because HP and normal rewards share the same `1 + idx * 0.15` ramp, normal
reward-per-HP pressure stays stable across 1-1 to 2-5. The 8x boss bonus is the
intentional spike for stage cap clears and should be reviewed separately from
normal farm pacing.

Gold ranges mirror the current client drop baseline, `10 + RandRange(0, 5)`,
before pet gold bonuses are applied. Equipment drop level remains
`1 + globalStageIndex`, so stage 1-1 drops level 1 items and stage 2-5 drops
level 10 items.

PR #32 simulator result, seed `23`, 1000 runs:

| Metric | Hours |
| --- | ---: |
| p10 | 4.900 |
| median | 5.324 |
| p90 | 5.758 |
| min | 4.582 |
| max | 6.128 |

The median remains inside the 5-10h first-rebirth target, and every sampled run
remains inside the 3-20h review band. No EXP curve change is applied in this
slice. Keep BossBonus at 8x and re-run `npm run balance:sim` after enhancement
spend or Chapter 2 reward data changes.

## PR #38 Gold Shop Sink V1

Gold shop gear rolls reuse the PR #32 stage reward multiplier:

```text
GearRollCost = max(1, round(300 * RewardMultiplier(globalStageIndex)))
RewardMultiplier(globalStageIndex) = 1 + globalStageIndex * 0.15
```

This keeps the shop cost curve on the same progression axis as kill gold and
monster stats. The V1 shop does not change kill rewards or enhancement costs;
it adds a repeatable gold-to-gear outlet that can feed the existing
drop-to-enhance loop.

Representative costs:

| Stage | idx | Reward x | Roll cost |
| --- | ---: | ---: | ---: |
| 1-1 | 0 | 1.00 | 300 |
| 1-5 | 4 | 1.60 | 480 |
| 2-5 | 9 | 2.35 | 705 |

Pressure check against PR #33:

- Enhancement is now a long-tail single-item sink: expected Common +0 to +50
  cost is 22,717,602.46 gold, or 34.7h at the sampled median Lv50 blended
  income of 654,689 gold/hour.
- At that same sampled income, the V1 shop can absorb about 2,182 rolls/hour
  at idx 0, 1,364 rolls/hour at idx 4, or 929 rolls/hour at idx 9 if the player
  repeatedly buys gear.
- A single roll is therefore not a hard endgame cap. The shop becomes the main
  gold sink because it is repeatable and directly converts surplus gold into
  more equipment comparison and enhancement candidates.
- Keep the base cost at 300 for this slice to avoid blocking early gear
  recovery. If telemetry shows excess rolling or continued gold hoarding,
  raise `BaseCost` in both `FShopFormula` and the server `shop.ts` mirror, then
  update parity tests in the same change.

## PR #39 Rarity-Scaled Enhancement Cost V1

Enhancement cost keeps the PR #33 level curve and multiplies it by equipped item
rarity:

```text
EnhanceCost = 100 * (CurrentLevel + 1)^2 * RarityCostMultiplier
```

| Rarity | Cost multiplier |
| --- | ---: |
| None | 0 |
| Common | 1 |
| Rare | 2 |
| Epic | 4 |
| Unique | 8 |
| Legendary | 16 |
| Transcendent | 32 |
| Mythic | 64 |

The design intent is to keep Common enhancement as the low-friction early-game
path while making high-rarity gear a deliberate gold sink. The single-argument
client/server helper remains Common-compatible so existing Common anchors keep
their PR #33 values. Equipped-item enhancement and the HUD panel must pass the
actual equipped rarity. A Rare +0 attempt costs 200 gold, a Legendary +0
attempt costs 1,600 gold, and a Mythic +0 attempt costs 6,400 gold; max-level
items still cost 0. PR #44 updates the success-rate curve and max level, while failure behavior and the
`1 + EnhanceLevel * 0.1` stat payoff remain intact.

Side effects to monitor:

- Early Common gear should remain at the PR #33 cost curve with no new onboarding
  tax.
- Rare+ items can fail the gold gate even when the player has enough gold for a
  Common item at the same level; HUD cost display and `TryEnhanceEquipped` spend
  must use the same rarity.
- Legendary enhancement creates meaningful midgame/endgame pressure, but the
  repeatable PR #38 shop remains the broader surplus-gold outlet.

Pressure check using the PR #44 expected Common +0 to +50 cost of
22,717,602.46 gold: a single Legendary item costs 363,481,639.40 expected gold,
and eight Legendary slots cost 2,907,853,115.20 expected gold. Against the
sampled PR #33 median Lv50 blended income of 654,689 gold/hour, that full
Legendary pass is about 4441.579h of income. The PR #65 seven-rarity x2 curve
puts Mythic at 64x: 1,453,926,557.61 expected gold per item and
11,631,412,460.88 expected gold for eight slots, about 17766.317h at the same
income anchor. Common early enhancement still starts at the PR #33 cost curve
while high-rarity enhancement becomes an open-ended midgame/endgame sink
alongside the repeatable shop roll outlet from PR #38.

The balance simulator reports these rarity scenarios:

| Rarity | Multiplier | Minimum +0 to +50 gold | Expected +0 to +50 gold |
| --- | ---: | ---: | ---: |
| Common | 1 | 4,292,500 | 22,717,602.46 |
| Rare | 2 | 8,585,000 | 45,435,204.93 |
| Epic | 4 | 17,170,000 | 90,870,409.85 |
| Unique | 8 | 34,340,000 | 181,740,819.70 |
| Legendary | 16 | 68,680,000 | 363,481,639.40 |
| Transcendent | 32 | 137,360,000 | 726,963,278.80 |
| Mythic | 64 | 274,720,000 | 1,453,926,557.61 |

## PR #40 Item Affix V1

Affixes add build variety without changing the drop rate, gold reward, or
enhancement cost curves. Base item stats stay on the PR #36 rarity multiplier
path; affixes are extra item fields with zero defaults, so legacy items keep
their existing PowerScore and equipment bonuses.

Affix count by rarity:

| Rarity | Affix count |
| --- | ---: |
| None | 0 |
| Common | 0 |
| Rare | 1 |
| Epic | 2 |
| Unique | 2 |
| Legendary | 2-3 |
| Transcendent | 2-3 |
| Mythic | 3 |

V1 affix types:

| Affix | Roll range |
| --- | ---: |
| CritRate | +0.01 to +0.05 |
| AtkSpeed | +0.05 to +0.15 |
| MagicAtk | round(Level times 0.5 to Level times 1.5) |

`FDropFormula::RollAffixes` uses an injected `FRandomStream`, clears previous
affix fields before every roll, and picks unique affix types per item. Common
and None therefore always resolve to zero affixes even if an item instance is
reused by a test, reroll, or future shop path. `UInventoryComponent::ComputeEquipmentBonus`
applies the same `1 + EnhanceLevel * 0.1` multiplier to affix bonuses as to
ATK/DEF/HP, then `FStatFormulas::DeriveStats` keeps the existing final clamps:
AtkSpeed 0.5-3.0 and CritRate 0-1.

PowerScore now values affixes before enhancement:

```text
PowerScore = round((ATK + DEF + HP / 10 + CritRate times 1000 + AtkSpeed times 100 + MagicAtk)
  times (1 + EnhanceLevel times 0.1))
```

Automation coverage:

- `IdleProject.Inventory.DropFormula.RollAffixes`
- `IdleProject.Inventory.ItemFactory.RandomDropRange`
- `IdleProject.Inventory.ItemFactory.HighLevelExpandedRarity`
- `IdleProject.Inventory.Bonus.Affixes`
- `IdleProject.Inventory.ItemPowerScore.Deterministic`
- `IdleProject.Character.StatFormulas.DeriveStats`

HUD copy stays compact and scan-friendly: only positive affixes are shown, in
CritRate / AtkSpeed / MagicAtk order, using localized labels such as
`Crit +3% / ASPD +0.10 / MATK +12`. Prefix/suffix names, reroll currencies,
potential lines, set effects, and additional affix types remain follow-up scope.

## PR #43 Equipment Sets V1

Equipment sets add flat bonuses after per-item equipment bonuses and before the
final `DeriveStats` clamp path. Set membership never changes
`FItemPowerScore::Compute` or `computeItemPowerScore`; automatic equip still
compares item-level PowerScore only.

Set pieces are counted only when the equipped item has a non-None slot,
non-None rarity, and one of the three V1 sets. A 4-piece set includes both the
2-piece and 4-piece rows:

| Set | 2-piece bonus | Additional 4-piece bonus | 4-piece total |
| --- | --- | --- | --- |
| Warrior | PhysAtk +20 | PhysAtk +50, CritRate +0.05 | PhysAtk +70, CritRate +0.05 |
| Guardian | PhysDef +15, HP +100 | PhysDef +35, HP +250 | PhysDef +50, HP +350 |
| Arcane | MagicAtk +20 | MagicAtk +50, CritDmg +0.10 | MagicAtk +70, CritDmg +0.10 |

Current set assignment is intentionally simple and mirrors client/server
behavior:

| Rarity | Set assignment |
| --- | --- |
| None | None |
| Common | None |
| Rare | Warrior, Guardian, or Arcane |
| Epic | Warrior, Guardian, or Arcane |
| Unique | Warrior, Guardian, or Arcane |
| Legendary | Warrior, Guardian, or Arcane |
| Transcendent | Warrior, Guardian, or Arcane |
| Mythic | Warrior, Guardian, or Arcane |

Eligible rarities pick one of the three V1 sets with the injected RNG. This
creates a clear early rule: Common equipment preserves legacy behavior, while
Rare and above, including Unique, Transcendent, and Mythic, can contribute to
set goals without changing stat rolls, affixes, enhancement cost, or PowerScore.

Build impact is intentionally moderate. Warrior and Arcane push attack lanes
with small crit modifiers at 4 pieces, while Guardian converts set completion
into survival. Because these are flat values, late-game scaling can eventually
make the 2/4-piece reward feel too small; if telemetry shows set completion is
ignored after higher level bands, the follow-up should evaluate percentage
bonuses or class-specific set families rather than inflating every flat value.

V1 auto-equip does not optimize for set completion. A lower-PowerScore item that
would complete a 2-piece or 4-piece set will not replace a higher-PowerScore
item automatically. Manual set targeting, set-aware compare UI, and optional
set-weighted auto-equip remain follow-up scope.

Automation coverage:

- `IdleProject.Inventory.DropFormula.RollItemSet`
- `IdleProject.Inventory.SetBonusFormula.DefinitionParity`
- `IdleProject.Inventory.SetBonusFormula.Thresholds`
- `IdleProject.Inventory.Bonus.SetBonus`
- `IdleProject.UI.HUD.EquipmentSetSummary`
- `server/src/core/formulas/setBonus.test.ts`
- `server/src/core/formulas/equipment.test.ts`

## PR #58 Item Variety Expansion

PR #58 keeps the PR #40 affix count curve and PR #43 set thresholds, but expands
the content pool used by those systems.

Base item generation now chooses a deterministic slot-local base id before
building `DisplayName` and `ItemId`. Weapons have six bases
(`longsword`, `greatsword`, `dagger`, `bow`, `staff`, `wand`); every armor and
accessory slot has three bases. Base definitions may apply small stat-bias
multipliers, but rarity multiplier, level, variance, enhancement, and PowerScore
remain the primary balance drivers.

Additional set families:

| Set | 2-piece bonus | Additional 4-piece bonus |
| --- | --- | --- |
| Assassin | CritRate +0.03 | CritDmg +0.15 |
| Hunter | AtkSpeed +0.05 | PhysAtk +35, AtkSpeed +0.03 |
| Holy | HP +120 | PhysDef +20, MagicDef +30 |
| Berserker | PhysAtk +30 | CritRate +0.04, CritDmg +0.10 |

Additional affixes:

| Affix | Roll range |
| --- | ---: |
| PhysDef | round(Level times 0.3 to Level times 1.0) |
| MagicDef | round(Level times 0.3 to Level times 1.0) |
| Hp | round(Level times 2.0 to Level times 5.0) |
| CritDmg | +0.05 to +0.20 |

PowerScore now includes the expanded affixes:

```text
PowerScore = round((ATK + DEF + HP / 10 + CritRate times 1000 + AtkSpeed times 100
  + MagicAtk + PhysDef + MagicDef + AffixHp / 10 + CritDmg times 100)
  times (1 + EnhanceLevel times 0.1))
```

## PR #60 Class Balance Philosophy

PR #60 keeps the eight-class expansion but removes strict-better growth from
the new damage classes. The rule is parity by role, not identical stats:
Warrior, Mage, Archer, Thief, Berserker, and Summoner target effective DPS
within +/-15% at Lv50 and Lv100 under the shared review loadout, while Paladin
and Cleric are allowed to sit below the DPS band because their value budget is
survival and support.

Class identity anchors:

| Class | Role budget | Trade-off |
| --- | --- | --- |
| Warrior | physical baseline | stable STR/CON, no extreme spike |
| Mage | magic baseline | high INT/WIS, lower HP |
| Archer | physical crit/ASPD | lower raw ATK, higher DEX/LUK scaling |
| Thief | physical crit/evasion | lower raw ATK, high LUK/DEX pressure |
| Cleric | healer/support | lower DPS, high WIS/MagicDef and recovery |
| Paladin | tank | lower DPS, highest HP/defense budget |
| Berserker | physical high-risk DPS | strong STR/LUK, lower defense than Warrior/Paladin |
| Summoner | magic status DPS | INT/WIS magic route, less raw INT growth than Mage |

The simulator effective DPS row uses:

```text
EffectiveAttack = MagicAtk for Mage/Cleric/Summoner, otherwise PhysAtk
ReviewDefense = Level * 5
SkillDpsRate = sum(active damageCoeff / cooldown)
BaseHit = computeClassDamage(DerivedStats, ClassId, ReviewDefense, ReviewDefense)
SkillDps = sum(
  computeClassDamage(Attack * damageCoeff, ClassId, ReviewDefense, ReviewDefense)
  / cooldown
)
EffectiveDps = (BaseHit * effective attack-speed expectation + SkillDps)
  * effective crit expectation
```

The review row intentionally uses the same physical/magic mitigation routing as
runtime combat instead of raw attack multiplication. Attack speed and crit are
partially weighted in the review model to represent real idle uptime instead of
assuming perfect burst conversion. Combat Power is reported next to DPS, but CP
is not the DPS gate; it is a separate survivability and aggregate-stat signal
from PR #49.

PR #60 tuning anchors:

- Berserker STR growth is reduced from 1.8 to 1.5 and LUK growth rises to 0.7,
  so the class keeps high-risk physical burst identity without simply replacing
  Warrior's STR curve.
- Summoner INT growth is reduced from 1.7 to 1.45 and CON growth rises to 0.52,
  so Summoner remains a magic/status DPS class without being a strict Mage
  upgrade.
- Berserker active coefficients are reduced to `2.35 / 1.65 / 2.05`.
- Summoner active coefficients are reduced to `1.9 / 1.45 / 2.0`.
- Client `SkillDB.csv`, client `USkillComponent`, and server `skills.ts` must
  keep those tuned coefficients in parity.
- `tools/balance-sim` samples all eight classes and reports Lv50/Lv100 class
  DPS, CP, HP, and effective attack rows in
  `tools/balance-sim/reports/balance-sim-report.md`.

The generated PR #60 report shows Lv100 DPS class deltas from the median at
Warrior -7%, Mage +3%, Archer -4%, Thief +1%, Berserker 0%, and Summoner -8%.
Paladin remains below the DPS band while having the highest HP/CP, and Cleric
remains below the DPS band while preserving the healer MagicDef/support budget.

## PR #35 Boss Patterns V1

Boss phase thresholds are shared by client combat, HUD, and the server mirror:

- `HpRatio > 0.66`: phase 1.
- `HpRatio > 0.33`: phase 2, so exactly `0.66` is phase 2.
- Otherwise: phase 3, so exactly `0.33` is phase 3.

Phase enrage is applied only at the attack calculation point. Boss base
`Atk` and `AtkSpeed` remain unchanged, while effective attack uses
`1.0 / 1.25 / 1.6` and effective cooldown uses `1.0 / 1.15 / 1.3`.
Normal monsters do not enter this branch.

The special attack cadence starts when boss combat starts, or on the first
direct attack if combat was invoked without `StartBattle`. The opening attack
is not a special attack. After 6 seconds have elapsed, the next boss attack
applies the `2.5x` special damage multiplier and broadcasts the HUD warning.

PR #31 currently anchors the Chapter 1 boss at stage 1-5 and PR #32 keeps boss
rewards on the same stage reward ramp with the 8x boss reward spike. These
phase and special-attack constants do not change EXP or reward formulas by
themselves. Re-run `npm run balance:sim` after boss HP, boss ATK, reward
scaling, or equipment/enhancement pressure changes, and compare the result to
the 5-10h first-rebirth target.

## PR #42 Pet Growth Gold Sink V1

Pet growth adds a gold-only sink to the existing PR #22 pet bonuses. The shared
client/server formula is:

```text
MaxPetLevel = 10
FeedCost(currentLevel) = 500 * (currentLevel + 1)^2, for Lv0 through Lv9
FeedCost(currentLevel >= 10) = 0
BonusMultiplier(level) = 1 + level * 0.1, clamped to Lv0 through Lv10
```

The full Lv0 to Lv10 feed path costs 192,500 gold:

| Current | Next | Feed cost | Cumulative cost | Dog gold bonus | Bird drop bonus |
| ---: | ---: | ---: | ---: | ---: | ---: |
| Lv0 | Lv1 | 500 | 500 | 22% | 16.5% |
| Lv1 | Lv2 | 2,000 | 2,500 | 24% | 18% |
| Lv2 | Lv3 | 4,500 | 7,000 | 26% | 19.5% |
| Lv3 | Lv4 | 8,000 | 15,000 | 28% | 21% |
| Lv4 | Lv5 | 12,500 | 27,500 | 30% | 22.5% |
| Lv5 | Lv6 | 18,000 | 45,500 | 32% | 24% |
| Lv6 | Lv7 | 24,500 | 70,000 | 34% | 25.5% |
| Lv7 | Lv8 | 32,000 | 102,000 | 36% | 27% |
| Lv8 | Lv9 | 40,500 | 142,500 | 38% | 28.5% |
| Lv9 | Lv10 | 50,000 | 192,500 | 40% | 30% |

Against the PR #32 sampled median Lv50 blended gold income of 654,689 gold/hour,
maxing the dog increases the gold bonus from 20% to 40%. The incremental
20 percentage points are worth about 130,938 gold/hour at that reference rate,
so the 192,500 gold investment pays back in about 1.47h. This is intentionally
shorter than the first-rebirth 5-10h median target: the pet sink is an early
recoupable investment, not a hard progression wall.

Balance guardrails:

- Dog growth compounds future gold income, so avoid making the Lv0 to Lv10
  curve steeper unless telemetry shows gold hoarding after shop and enhancement
  spend.
- Bird growth doubles the drop bonus from 15% to 30%, but it does not directly
  generate gold. Treat it as loot-depth pressure and compare against equipment
  quality metrics, not only gold/hour.
- Pet feeding competes with enhancement and shop gold. Since it does not add
  direct combat power or EXP, it should not shorten the first-rebirth EXP curve
  by itself.
- Re-run `npm run balance:sim` after changing `petLevel.ts`,
  `FPetLevelFormula`, kill gold, shop cost, or enhancement cost. The report must
  include `Pet Feed Gold Pressure` with the 1000-run Lv50 gold/hour reference.

## PR #47 Transcendence V1

Transcendence is the second prestige layer above rebirth. V1 unlocks
`Transcend()` at `RebirthCount >= 5`. A successful transcend increments
`TranscendCount`, resets rebirth progress and short-cycle progression to zero or
level 1, and grants a permanent global combat-stat multiplier:

```text
TranscendStatMultiplier = 1 + max(0, TranscendCount) * 0.25
```

The multiplier is intentionally linear and uncapped in V1. The first transcend
therefore gives 1.25x, four transcends give 2.00x, and ten transcends give
3.50x. It applies only to HP, physical attack, magic attack, physical defense,
and magic defense. Rate-style stats such as attack speed, crit rate, crit
damage, dodge, and accuracy stay on their existing clamps and item/class
systems.

The threshold of five rebirths is a pacing trade-off: it asks the player to
complete a meaningful rebirth loop before the deeper reset, but it is still
short enough that the 25% permanent multiplier can be felt on the next climb.
Because transcend resets `RebirthCount` and `RebirthBonusPoints`, the PR #46
rebirth reward formula restarts from count 0 after each transcend. This prevents
the rebirth count scaler and transcend multiplier from compounding in the same
loop without a reset cost.

V1 guardrails:

- Count 0 must be a neutral 1.00x multiplier and preserve existing combat stats.
- Server `transcend.ts` must use `Math.fround` at the same formula boundary as
  the client float formula.
- HUD copy must show locked/ready state, current rebirths versus threshold,
  current multiplier, next multiplier, count, and a disabled action below the
  threshold.
- Re-run UE automation and server Vitest when changing the threshold,
  multiplier step, reset policy, or affected stat list.

## PR #49 Combat Power V1

Combat Power is a display and validation aggregate over final derived stats. V1
does not add a separate CP stat, ranking authority, content gate, or direct
skill-rank contribution. Any growth source must first change
`GetCurrentDerivedStats()`, then CP is recomputed from that final cache.

Formula:

```text
CP = round(
  PhysAtk * 1
  + MagicAtk * 1
  + PhysDef * 2
  + MagicDef * 2
  + Hp * 0.1
  + CritRate * 500
  + CritDmg * 100
  + AtkSpeed * 200
)
CP = max(0, CP)
```

Server and client parity use double precision accumulation before the final
round. Do not use `Math.fround` or float-step rounding in the TypeScript mirror;
large CP anchors intentionally detect that drift.

Representative examples:

| Build state | Inputs summarized | CP |
| --- | --- | ---: |
| Early warrior | HP 130, PATK 24, MATK 5, PDEF 14, MDEF 4, crit 0.006/x1.503, ASPD 1.024 | 436 |
| Mid growth | HP 1234, PATK 100, MATK 50, PDEF 30, MDEF 20, crit 0.25/x1.8, ASPD 1.5 | 978 |
| Large parity anchor | HP 1234567, PATK 10000000, MATK 3000000, PDEF 100000, MDEF 50000, crit 0.333/x2.75, ASPD 2.25 | 13424348 |

Guardrails:

- CP is monotonic for positive stat allocation, equipment affix/set bonuses,
  enhancement, rebirth bonus points, and transcend multipliers because all of
  them flow through final derived stats.
- CP may eventually require coefficient tuning if HP or defense dominates player
  perception, but V1 keeps the weights simple and transparent for QA.
- HUD copy must stay compact: `전투력 {Amount}` and `Combat Power {Amount}`.

## PR #50 Infinity Tower V1

Infinity Tower is an optional infinite-progress track keyed only by current
combat power. It must not mutate combat stats, stage progress, rebirth state, or
transcend state. The client character slice uses the following V1 anchors:

```text
RequiredPower(floor) = round(100 * 1.15^(max(1, floor) - 1))
CanClearFloor(cp, floor) = cp >= RequiredPower(floor)
FloorReward(floor) = round(50 * max(1, floor))
```

Representative client anchors:

| Floor | Required CP | Reward gold |
| ---: | ---: | ---: |
| 1 | 100 | 50 |
| 2 | 115 | 100 |
| 3 | 132 | 150 |
| 10 | 352 | 500 |
| 50 | 94,231 | 2,500 |
| 100 | 102,114,213 | 5,000 |

Guardrails:

- `TryClimbTower` may clear several consecutive floors in one call, but one
  call is capped at 100 floors to prevent runaway loops when CP is very high.
- Rewards are granted only for newly cleared floors; calling again with the same
  CP cannot reclaim previous rewards.
- CP shortage leaves `HighestFloor` unchanged and pays zero gold.
- Client and server mirrors use double precision `pow(1.15, floor - 1)` before
  the final round. Do not introduce float-step rounding or lookup drift unless
  both client and server parity anchors are intentionally updated together.
- Oversized required-power calculations clamp to int64 max, and gold grants
  saturate rather than wrapping when tower rewards are applied near the int64
  boundary.

## PR #51 Tower Milestone Bonus V1

Tower milestone bonus turns the highest cleared Infinity Tower floor into a
small permanent combat-stat multiplier:

```text
TowerMilestoneMultiplier = 1 + floor(max(0, HighestFloor) / 10) * 0.02
```

Every 10 cleared floors adds +2% to HP, physical attack, magic attack, physical
defense, and magic defense. Floors 0 through 9 stay neutral at 1.00x, floor 10
is 1.02x, floor 25 is 1.04x, and floor 100 is 1.20x. Rate-style stats such as
attack speed, crit rate, crit damage, dodge, and accuracy are intentionally not
multiplied, matching the PR #47 transcend stat-list boundary.

The bonus is economic-neutral by itself. It does not change tower gold rewards,
stage rewards, drop rates, enhancement costs, shop costs, pet growth, rebirth
rewards, or season rewards. Any pacing impact must come through final derived
stats and the PR #49 Combat Power formula, which then lets the player clear
more tower floors or other CP-gated checks.

Transcendence and tower milestones multiply together at the final derived-stat
cache boundary:

```text
StatMultiplier = TranscendStatMultiplier * TowerMilestoneMultiplier
```

Guardrails:

- `HighestFloor <= 0` and missing tower state must resolve to 1.00x.
- Client `FTowerMilestoneFormula` and server `getTowerMilestoneMultiplier`
  must keep parity anchors for 0, 9, 10, 25, and 100.
- HUD copy should stay compact as `xN.NN` and show the next 10-floor milestone.
- Re-run UE automation and server Vitest when changing the step size, bonus
  rate, affected stat list, or CP formula weights.

## PR #61 Rune / Relic Balance Review

PR #61 keeps the rune formula values from the implementation plan and validates
them through `tools/balance-sim`.

The 1000-run rebirth distribution remains inside the pacing target after adding
rune pressure reporting:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

The median remains inside the 5-10h target, and every sampled run remains inside
the 3-20h review band. Because rune acquisition and enhancement are not injected
into the sampled first-rebirth run yet, this result is a baseline pacing guard:
future rune drop-rate, shop-cost, or essence-income changes must rerun the
simulator and compare against this distribution.

Core rune growth is intentionally uncapped. A single Mythic core rune reaches
+168% at +50 and +318% at +100. Six same-lane Mythic +50 core runes therefore
produce an x11.08 stat multiplier on that lane. Under the shared Lv100 review
loadout this reports:

| Rune set | Class | CP x | DPS x |
| --- | --- | ---: | ---: |
| 6x Mythic +50 PhysAtk | Warrior | 4.703 | 12.356 |
| 6x Mythic +50 MagicAtk | Mage | 4.806 | 12.233 |

This confirms core runes are a long-tail infinite-growth system, not a small
sidegrade. The risk is acceptable only while essence and gold costs remain the
primary throttle and while early drop/shop rates do not inject high-rarity,
high-enhance runes into the first-rebirth window.

Utility runes keep per-rune caps and then apply an aggregate cap after summing
the equipped six-slot pressure:

| Utility | Mythic cap enhance | One-rune cap | Six-slot total | Effective multiplier |
| --- | ---: | ---: | ---: | ---: |
| CritDamage | +177 | 100% | 600% | x2 |
| GoldFind | +377 | 200% | 1200% | x3 |
| ExpBoost | +377 | 200% | 1200% | x3 |
| OfflineEff | +127 | 50% | 300% | x1.5 |

GoldFind, ExpBoost, and OfflineEff aggregate bonuses are capped after summing
all equipped rune slots, matching `URuneService::GetEquippedUtilValues`. Keep
them as aspirational late-game builds unless future telemetry shows a need for
lower account-level caps. If first-rebirth median time drops below 5h after rune
acquisition is wired into the simulator, the first tuning candidates are rune
drop rate, shop roll cost, and essence income rather than the level curve.

## PR #62 Rune Codex Collection Balance Review

PR #62 adds rune codex collection pressure reporting to `tools/balance-sim`
without injecting codex bonuses into the sampled first-rebirth run. The sampled
1000-run distribution therefore remains the PR #61 baseline guard:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

Rune codex core-stat bonuses are additive before being applied to the five core
stat lanes:

```text
0.4% per unlocked cell
+1% / +2% / +3% / +5% / +8% / +12% for completed rarity rows
+5% for completing the core category
+10% util cap extension for completing the util category
```

A full 54-cell codex gives +21.6% from cells, +31% from all completed rarity
rows, and +5% from core category completion, for +57.6% core stat add. The util
category adds +10 percentage points to utility rune caps, but it does not change
the first-rebirth baseline unless rune acquisition and equipped utility runes
are explicitly modeled.

If the full +57.6% core codex bonus were applied directly as first-rebirth DPS
pressure, the 5.328h median projects to about 3.381h, a -36.5% shift. That is
inside the broad 3-20h review band but below the 5-10h median target. Treat this
as a risk signal rather than a live rebirth result: until rune drop rates,
codex unlock pacing, and early equipped rune availability are wired into the
simulator, codex bonuses should stay documented as collection pressure only.

Guardrails:

- Do not tune the level curve against the projected full-codex median unless
  codex acquisition is actually active during the first-rebirth simulation.
- If future acquisition modeling pushes the sampled median below 5h, tune rune
  drop rate, shop roll cost, unlock pacing, or essence income before reducing
  the codex formula.
- Keep `FRuneCodexFormula`, `runeCodex.ts`, and `tools/balance-sim` parity
  tests aligned with `Math.fround` boundaries.

## PR #63 Class Mastery Rune Balance Review

PR #63 adds one dedicated ClassMastery rune slot on top of the six regular rune
slots. The balance simulator treats this as a separate pressure row: one Mythic
+50 class rune is applied to each Lv100 review loadout, while the six regular
core slots stay empty. This keeps class mastery pressure visible without
changing the sampled first-rebirth distribution.

The PR #63 sampled 1000-run first-rebirth distribution is unchanged from the
PR #61/#62 baseline because class mastery acquisition is not injected into the
first-rebirth model:

| Metric | Hours |
| --- | ---: |
| p10 | 4.919 |
| median | 5.328 |
| p90 | 5.751 |
| min | 4.564 |
| max | 6.144 |

ClassMastery stat mapping:

| Class | Mastery stats |
| --- | --- |
| Warrior | PhysAtk, PhysDef |
| Mage | MagicAtk |
| Archer | PhysAtk |
| Thief | PhysAtk |
| Cleric | MagicAtk, Hp |
| Paladin | PhysDef, Hp |
| Berserker | PhysAtk |
| Summoner | MagicAtk |

At the Lv100 Mythic +50 review point, DPS classes remain inside the PR #60
+/-15% band after applying their class rune. Warrior receives a higher CP
multiplier than single-lane Berserker because its second lane is PhysDef, not
additional DPS. Cleric converts its second lane into HP support pressure.
Paladin receives no DPS multiplier from ClassMastery because its dedicated rune
is PhysDef + HP only; this preserves the tank budget and prevents the support
slot from becoming a hidden damage lane.

Guardrails:

- Keep ClassMastery as the seventh dedicated slot; regular slots 0-5 remain
  core/utility only.
- ClassMastery multipliers are additive bonus values returned from
  `FClassRuneFormula` / `classRune.ts`, then applied as `1 + bonus` to the
  affected stat lanes.
- Do not tune first-rebirth pacing from class mastery pressure until class rune
  drop/craft acquisition is modeled in `tools/balance-sim`.
- Re-run `npm run balance:sim` and `npm test -- tests/balance-sim.test.ts`
  after changing class mastery stat mapping, rarity scaling, or enhancement
  scaling.

## PR #64 Rune Set Bonus Balance Review

PR #64 adds rune set identity to the six regular rune slots only. The dedicated
ClassMastery slot remains outside set counting even if legacy or malformed save
data carries a `RuneSet` value there. Set bonuses are pure additive stat
bonuses and use the highest reached tier, not cumulative tiers:

| Same set count | Bonus |
| ---: | ---: |
| 0-1 | 0% |
| 2-3 | 5% |
| 4-5 | 12% |
| 6+ | 25% |

Set mapping:

| Rune set | Bonus lanes |
| --- | --- |
| Offense | PhysAtk, MagicAtk |
| Bastion | PhysDef, MagicDef |
| Vitality | Hp, OfflineEff |
| Fortune | GoldFind, ExpBoost, CritDamage |

Core set bonuses join the same additive multiplier path as regular core runes:
`Result.Stat += SetBonus.Stat`. Utility set bonuses are added after the existing
utility cap, so Fortune and Vitality remain visible even when rune utility values
are already capped. This is intentional; tune set tier values before changing
the shared utility caps.

The PR #64 simulator pass keeps rune set acquisition out of the sampled
first-rebirth run, so the 1000-run distribution remains the PR #61 baseline:
p10 4.919h, median 5.328h, p90 5.751h, min 4.564h, and max 6.144h. The set
table is pressure analysis against the shared Lv100 Warrior review loadout:

| Rune set | Count | Bonus | CP x | DPS x |
| --- | ---: | ---: | ---: | ---: |
| Offense | 2 | 5% | 1.035 | 1.056 |
| Offense | 4 | 12% | 1.083 | 1.135 |
| Offense | 6 | 25% | 1.173 | 1.282 |
| Bastion | 2 | 5% | 1.007 | 1.000 |
| Bastion | 4 | 12% | 1.017 | 1.000 |
| Bastion | 6 | 25% | 1.036 | 1.000 |
| Vitality | 2 | 5% | 1.003 | 1.000 |
| Vitality | 4 | 12% | 1.008 | 1.000 |
| Vitality | 6 | 25% | 1.017 | 1.000 |
| Fortune | 2 | 5% | 1.001 | 1.004 |
| Fortune | 4 | 12% | 1.002 | 1.011 |
| Fortune | 6 | 25% | 1.005 | 1.022 |

Offense is the only 6-piece set that materially changes the Warrior DPS row in
this review model. Bastion and Vitality are CP/survival pressure, while Fortune
is mostly economy pressure with a small CritDamage DPS signal. Do not nerf the
2/4/6 tier values from this table alone; first model set acquisition timing,
drop rates, and early regular-slot availability.

Guardrails:

- `FRuneSetFormula`, `runeSet.ts`, and tests must keep `Math.fround`/float
  parity at the 0.05, 0.12, and 0.25 tier boundaries.
- Common and `None` rarity shop/drop rolls produce `RuneSet=None`; higher
  rarities may still roll `None` to preserve non-set outcomes.
- SaveVersion 6 must default missing or legacy rune-set data to `None`.
- Do not tune first-rebirth pacing from full six-piece set pressure until rune
  set acquisition is modeled in `tools/balance-sim`.

## PR #67 Unique Trait Balance Review

PR #67 adds equipment unique traits as a rarity-gated pressure layer on top of
the existing item stat, affix, and rune set systems. Traits are available only
on Unique and Transcendent equipment: Unique rolls one trait, Transcendent rolls
two distinct traits, and Mythic is explicitly excluded from this trait budget.

The server/client formula values are:

| Trait | Unique | Transcendent |
| --- | ---: | ---: |
| AllStatSurge | 8% | 12% |
| CritDamageSurge | 15% | 22.5% |
| CritRateSurge | 5% | 7.5% |
| LifeSurge | 10% | 15% |
| SwiftSurge | 8% | 12% |
| PhysMastery | 12% | 18% |
| MagicMastery | 12% | 18% |
| GuardMastery | 10% | 15% |

`tools/balance-sim` imports `server/src/core/formulas/uniqueTrait.ts` and
reports trait pressure against the shared Lv100 Warrior review loadout:

| Rarity | Trait count | Traits | CP x | DPS x |
| --- | ---: | --- | ---: | ---: |
| Unique | 1 | PhysMastery | 1.044 | 1.135 |
| Transcendent | 2 | AllStatSurge, PhysMastery | 1.175 | 1.362 |

This pressure stacks after existing affix and rune-set reporting, but it is not
injected into the sampled first-rebirth run. The 1000-run first-rebirth
distribution remains at p10 4.919h, median 5.328h, p90 5.751h, min 4.564h,
and max 6.144h. Median remains inside the 5-10h target and every sampled run
remains inside the 3-20h review band.

Guardrails:

- Keep `FUniqueTraitFormula`, `uniqueTrait.ts`, and `tools/balance-sim` aligned
  at the `Math.fround` parity boundary.
- Core-stat traits (`AllStatSurge`, `LifeSurge`, `PhysMastery`,
  `MagicMastery`, `GuardMastery`) are percent multipliers applied after
  `DeriveStats` has combined base stats, equipment flats, set flats, rebirth
  flats, and utility trait flats. They do not enter `EquipmentBonus` as flat
  `FDerivedStats` values.
- Utility traits (`CritDamageSurge`, `CritRateSurge`, `SwiftSurge`) remain flat
  additions through the equipment bonus path and are clamped by the existing
  derived-stat formulas.
- Do not tune first-rebirth pacing from trait pressure until Unique/
  Transcendent acquisition timing is modeled in `tools/balance-sim`.
- If future acquisition modeling pushes the sampled median below 5h, tune drop
  availability or trait roll timing before reducing the trait value table.

## PR #68 Dungeon Reward Balance Review

PR #68 adds three daily dungeons: Gold, Exp, and Essence. Each dungeon has a
daily entry limit of three runs and a minimum combat-power gate:

| Dungeon | Minimum CP | Base reward at minimum CP |
| --- | ---: | ---: |
| Gold | 100 | 20,000 gold |
| Exp | 250 | 20,000 EXP |
| Essence | 500 | 12 rune essence |

Dungeon reward scaling is intentionally sublinear:

```text
Reward = round(BaseReward * sqrt(max(1, CombatPower / MinimumCp)))
```

The TypeScript mirror keeps the same `Math.fround` boundary as
`FDungeonFormula`: the CP ratio is rounded to float precision before `sqrt`,
and the multiplier is rounded to float precision before reward rounding. This
keeps server/client parity while avoiding linear high-CP reward pressure.

The balance simulator reports dungeon pressure against sampled Lv50 income
without injecting dungeon rewards into the first-rebirth run:

| Dungeon | CP | Reward/run | 3-run daily reward | Lv50 income equivalent |
| --- | ---: | ---: | ---: | ---: |
| Gold | 100 | 20,000 gold | 60,000 gold | 0.092h |
| Gold | 5,500 | 148,324 gold | 444,972 gold | 0.68h |
| Exp | 5,500 | 93,808 EXP | 281,424 EXP | 0.385h |
| Essence | 5,500 | 40 essence | 120 essence | n/a |

Seed `23`, 1000 runs, still reports the same first-rebirth baseline:
p10 4.919h, median 5.328h, p90 5.751h, min 4.564h, max 6.144h. The median
remains inside the 5-10h target and every sampled run remains inside the
3-20h review band.

Guardrails:

- Keep dungeon acquisition out of the sampled first-rebirth timing model until
  daily unlock timing and expected player usage are modeled explicitly.
- Gold and EXP daily totals should stay below one sampled Lv50 income hour at
  the Lv100 review CP around 5,500.
- Essence pressure must remain quantity-based until rune-essence/hour exists in
  the simulator; avoid tuning rune enhancement costs from this row alone.
- Re-run `npm run balance:sim`, `npm test -- tests/balance-sim.test.ts`, and
  dungeon parity tests after changing base rewards, CP gates, entry limits, or
  the `sqrt` scaling formula.

---

## PR #66 Chapter 3 Stage/Element Balance Addendum

PR #66 expands the stage model to 3 chapters with 10 stages per chapter. The
global stage index is one-based and must be computed as:

```text
GlobalStageIndex = (Chapter - 1) * 10 + Stage
```

This yields chapter 1 as 1-10, chapter 2 as 11-20, and chapter 3 as 21-30.
Stage 5 is the elite encounter in every chapter, and stage 10 is the chapter
boss. Elite rewards use a 3x bonus; boss rewards keep the 8x bonus and take
priority if a stage is ever classified as both.

Monster and reward scaling use the shared stage multiplier:

```text
Multiplier = 1 + max(0, GlobalStageIndex - 1) * 0.15
```

The `- 1` progression step keeps stage 1-1 at the exact baseline multiplier
while preserving the prior early-game balance curve for the simulator's first
rebirth review band.

Chapter 3 introduces Dark as the fifth combat element. Holy attacks deal 1.5x
damage to Dark targets, and Dark attacks deal 1.5x damage to Holy targets.
Unrelated Holy/Dark pairings remain 1.0x. Weakness coverage is:

| Global stages | Weak element notes |
| --- | --- |
| 1-10 | Fire/Ice/Lightning/Holy/Dark cycle, boss weak to Dark |
| 11-20 | Mixed second-chapter cycle, elite and boss weak to Dark |
| 21-30 | Dark-heavy chapter, alternating non-Dark counters for variety |

Save migration moves to SaveVersion 8. Legacy SaveVersion < 8 data preserves
the saved current chapter/stage where possible, carries chapter 1 boss defeat
into the highest-cleared chapter floor, and prevents legacy two-chapter saves
from accidentally marking chapter 3 complete.

Balance simulator evidence for PR #66:

- `tools/balance-sim` imports `stage.ts`, `reward.ts`, and `combat.ts` for the
  30-stage review table instead of duplicating stage, reward, or element
  constants.
- The generated report covers all chapter 1-3 stages, global indexes 1-30,
  encounter type, weak element, normal rewards, elite 3x rewards, and boss 8x
  rewards.
- Chapter 3 is Dark-heavy without changing the sampled first-rebirth timing
  model: Dark weakness appears on 9 of 30 stages overall and 5 of 10 chapter 3
  stages.
- Dark matchup pressure stays bounded to the existing element formula:
  Holy->Dark, Dark->Holy, and Dark->Dark are x1.5 weakness hits; unrelated
  Dark matchups such as Dark->Fire remain neutral x1.0.
- The 1000-run first-rebirth distribution remains at p10 4.919h, median
  5.328h, p90 5.751h, min 4.564h, and max 6.144h. Median remains inside the
  5-10h target and every sampled run remains inside the 3-20h review band.
