# 클래스 스킬 확장 (Class Skill Expansion) — 설계 문서

> 작성일: 2026-05-30 · 작성: PM/Claude · 분류: 전투 콘텐츠(스킬 #57 후속) · 데이터 구동
> 대상 PR: 단일 슬라이스 · 상태: PM 자율(사용자 "계속")

## 0. 한 줄 요약
8개 클래스에 **신규 스킬 1종씩**(클래스 정체성 맞춤)을 추가해 전투 로테이션을 확장. 데이터 구동
(generic ExecuteSkill 재사용)·**DPS 밴드(±15%, #60) 유지**·세이브 무변경.

## 1. 배경
- 스킬(#57): FSkillDefinition 완전 데이터 구동(ESkillEffectType DamageSingle/Aoe/SelfBuff/DashDamage/Heal,
  쿨다운/계수/버프/상태이상/속성). 클래스당 ~6~7 스킬, generic ExecuteSkill 실행.
- 클래스별 스킬 풀 확장 = 전투 다양성([[project_content_richness]]). 신규 effect 코드 없이 **데이터만**.
- class-balance.test ±15% DPS 밴드(#60) — 신규 스킬은 밴드 유지하도록 균등 보강.

## 2. 핵심 결정 (PM)
| 항목 | 결정 |
| --- | --- |
| 신규 스킬 | **클래스당 1종**(8종) — 클래스 정체성·속성 맞춤(예 Archer 다중타, Cleric 힐/버프, Berserker 고계수). |
| effect | 기존 ESkillEffectType 재사용(신규 effect 코드 없음 — 데이터 구동). |
| 밸런스 | 각 클래스 DPS 기여가 균등 비율이 되도록 계수 튜닝 → **class-balance.test ±15% 밴드 유지**(서버 테스트가 안전망). |
| 획득 | 기존과 동일(클래스 스킬 자동 보유, AI 로테이션). 신규 unlock/포인트 없음. |
| 세이브 | **무변경**(SkillRanks 맵은 전방호환 — 신규 스킬 id 기본 랭크). |

## 3. 데이터 (초기값 — balance-note 확정)
```
클래스별 신규 스킬 1종(SkillId/DisplayName/Type/EffectType/Cooldown/DamageCoeff/Buff/Status/Element):
 Warrior: 대지 가르기(Active, DamageAoe, 고계수 광역)
 Mage: 메테오(Ultimate 또는 Active, DamageAoe, Fire Burn)
 Archer: 다중 사격(Active, DamageAoe, 중계수)
 Thief: 그림자 일격(Active, DamageSingle, 고계수 단일 + Dark)
 Cleric: 신성 가호(Active, SelfBuff/Heal)
 Paladin: 심판(Active, DamageSingle, Holy)
 Berserker: 피의 격노(Active, DamageSingle, 고계수)
 Summoner: 정령 폭발(Active, DamageAoe, 속성)
 (effect/계수는 클래스 정체성·DPS 밴드 맞춤, balance 확정)
```

## 4. 통합 지점 (5-team)
| 파트 | 작업 |
| --- | --- |
| character | 클라 `SkillComponent.cpp` 8 클래스 신규 스킬 `Skills.Add(MakeSkill(...))` + 서버 `skills.ts` 미러(classXxxSkillDefinitions). 클라↔서버 skills parity(skills.test.ts). backend 흡수. |
| balance | DPS 계수 튜닝 — **class-balance.test ±15% 밴드 유지**(balance-sim/class-balance 재실행). `docs/planning/class-skill-expansion-balance-note.md`. |
| designer | 신규 스킬 표시명/아이콘 로컬라이즈(스킬명 CSV) ko/en + CsvIntegrity. 표준 jumbo. |
| qa | 신규 스킬 실행(effect별)·클래스 parity·DPS 밴드·세이브 무변경. jumbo+게이트. |

## 5. 스코프
**In:** 8 클래스 신규 스킬 1종 + parity + DPS 밴드 유지 + 로컬라이즈. **세이브/effect 코드 무변경**.
**Out:** 신규 ESkillEffectType, 스킬 슬롯/장착 시스템, 스킬 트리, 스킬 RNG.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| DPS 밴드 붕괴(±15%) | 균등 비율 보강 + **class-balance.test 서버 게이트**(밴드 위반 시 red → 튜닝). |
| 클라↔서버 skills parity | skills.ts ↔ SkillComponent 미러, skills.test.ts parity. |
| effect 미지원 | 기존 ESkillEffectType만 사용(신규 코드 없음). |
| 세이브 호환 | SkillRanks 전방호환(신규 id 기본 랭크), SaveVer 무변경. |
