# 밸런스 철학 — idle-project

> 본 문서는 수치 자체보다 **수치를 결정하는 원칙** 을 다룹니다. 구체 곡선/계수는 밸런스 슬라이스 PR (마일스톤별) 에서 데이터 테이블에 반영됩니다.

---

## 부록: M2 인벤토리 V1 수치

`client/Content/Data/ItemDB.csv`는 PR #9 범위의 8슬롯 × 3등급 기본 장비 24종을 정의한다. Weapon은 ATK 100%, Helmet/Top/Bottom/Shoes/Gloves/Cloak은 DEF 70% + HP 30%, Accessory는 ATK/DEF/HP 균형형으로 둔다.

| 등급 | ATK 평균 | DEF 평균 | HP 평균 | MaxEnhance |
| --- | ---: | ---: | ---: | ---: |
| Common | 1.0 | 2.5 | 11.25 | 5 |
| Uncommon | 2.0 | 5.25 | 26.63 | 8 |
| Rare | 3.88 | 9.75 | 49.38 | 10 |

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
- 드롭률 (장비): 마리당 5% (Common 70 / Uncommon 20 / Rare 8 / Epic 1.8 / Legendary 0.2 / Mythic 0.001%)

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

<!-- markdownlint-enable MD013 -->

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
- 단, 환생 보상 자체는 점진적 감소 (5점 → 4점 → 4점 → 3점 ...) — 무한 환생으로 인플레 방지

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

Client V1 uses a conservative gold sink for the existing `EnhanceLevel` 0..5
equipment multiplier.

| Current level | Cost | Success rate |
| ---: | ---: | ---: |
| 0 | 100 | 95% |
| 1 | 400 | 85% |
| 2 | 900 | 70% |
| 3 | 1600 | 55% |
| 4 | 2500 | 40% |
| 5 | 0 | 0% |

The cost formula is `100 * (CurrentLevel + 1)^2` before max level. Failure
does not destroy or downgrade equipment in V1; it consumes the attempt cost and
records enhancement quest progress. The stat payoff remains the existing
`1 + EnhanceLevel * 0.1` equipment bonus multiplier.

Sensitivity notes:

- EXP curve: current curve is acceptable for the first rebirth target.
- Gold per hour: keep active gold tuning stable until enhancement spend data is
  available.
- Offline efficiency: current sampled 70-80% band keeps idle progress below
  active play while staying relevant.
- Simulator pressure check: the deterministic 1000-run balance report imports
  `server/src/core/formulas/enhance.ts` and models +0 to +5 enhancement spend.
  The minimum all-success cost is 5,500 gold, while expected cost using
  `cost / successRate` is 11,020.66 gold. Against the sampled median Lv50
  active/idle blended gold rate, a single +0 to +5 path is only about 0.017h
  of income. Treat this as a baseline pressure check, not evidence that V1 is
  a meaningful midgame sink. V1 deliberately avoids blocking first rebirth;
  later slices should raise pressure through higher-level cost bands, material
  requirements, or repeated-slot enhancement demand after real gold telemetry.
- Reward parity check: PR #32 reward scaling remains aligned with monster HP
  scaling because both use `1 + globalStageIndex * 0.15`. Chapter 1 normal
  reward-per-HP pressure stays stable across 1-1 to 1-5; the 8x boss reward
  bonus remains the intentional stage-cap spike.

---

## PR #27 Skill Rank Points V1 Anchors

Skill rank points are a client-combat V1 progression layer, separate from stat
points and rebirth points.

Rules:

- `AIdleCharacter::HandleLevelUp` grants `USkillComponent` exactly 1 skill
  point per level-up event.
- Each skill has an independent rank stored by `SkillId`.
- `MaxRank` is 5.
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

For `heavy_strike` (`baseDamageCoeff=2.5`, `baseCooldown=4.0s`), rank 5
therefore resolves to `effectiveDamageCoeff=3.75` and
`effectiveCooldown=3.0s`. This is the V1 cap: +50% damage coefficient and -25%
cooldown from five level-up skill points spent into one skill.

V1 policy:

- Class switching keeps the global point/rank state in memory. A rank only
  applies when its matching `SkillId` exists in the currently loaded skill set.
- Rebirth-specific reset or preservation rules are deferred to a later slice.
- No server persistence or formula mirror is shipped in this C++-only step.

Automation coverage:

- `IdleProject.Combat.Skills.RankPoints` verifies point grants, rank-up spend,
  missing-skill rejection, no-point rejection, and max-rank clamp.
- `IdleProject.Combat.Skills.RankEffectiveValues` verifies rank 2 damage and
  cooldown effective values plus cooldown remaining/ratio routing.
- `IdleProject.Combat.Skills.RankDamageApplication` verifies ranked damage uses
  the effective damage coefficient in `TickSkills`/damage application.
- `IdleProject.Character.LevelUp.GrantsSkillPoint` verifies the character
  level-up handler grants one skill point.

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
"챕터 1 보스 격파" 플래그를 세운다. V1은 다음 챕터 전환 없이 1-5 클리어
상태를 유지한다.
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

<!-- markdownlint-enable MD013 -->

Because HP and normal rewards share the same `1 + idx * 0.15` ramp, normal
reward-per-HP pressure stays stable across 1-1 to 1-5. The 8x boss bonus is the
intentional spike for stage cap clears and should be reviewed separately from
normal farm pacing.

Gold ranges mirror the current client drop baseline, `10 + RandRange(0, 5)`,
before pet gold bonuses are applied. Equipment drop level remains
`1 + globalStageIndex`, so stage 1-1 drops level 1 items and stage 1-5 drops
level 5 items.

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
