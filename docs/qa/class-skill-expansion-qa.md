# 클래스 스킬 확장 QA 검증 노트

> 작성 2026-05-30 · 검증: PM · **세이브 무변경**(SaveVer 24)

## 1. 자동 검증 게이트

| 게이트 | 범위 | 결과 |
| --- | --- | --- |
| 서버 vitest (`skills.test`, class-balance) | 신규 8종 parity + DPS 밴드 ±15% | **전체 841/0** (class-balance GREEN) |
| 서버 lint/build | 전체 | **GREEN** |
| UE jumbo 빌드 | 전체(ODR) | **Succeeded** |
| UE Automation | Combat/Skill(parity)/Localization(CsvIntegrity)/GameCore(SaveSystem v24) | **131/0** |

## 2. 시나리오 커버리지

- **신규 스킬 8종**: 클래스별 1종(effect DamageSingle/Aoe/SelfBuff), generic ExecuteSkill 실행. 클라 CombatTests(스킬 카운트 7→8, HasSkill, ParityTest 56→64).
- **클라↔서버 parity**: SkillComponent ↔ skills.ts ↔ SkillDB.csv 8종 id/effect/계수/속성 1:1(skills.test).
- **DPS 밴드 ±15%(#60)**: class-balance.test GREEN(최대 |delta| Lv50 7.8%/Lv100 4.6%), Tank/Healer 분리(Paladin/Cleric 0.8 임계) 유지.
- **로컬라이즈**: Skill.csv ko/en 8키(Thief/Cleric 기존 누락분 보완 포함) CsvIntegrity.
- **세이브 무변경**: SkillRanks 전방호환(신규 id 기본 랭크), SaveVer 24 유지.

## 3. 후속/비고

- 신규 ESkillEffectType·스킬 슬롯/장착·스킬 트리는 후속. 계수 재튜닝.
- PR 본문 신규 스킬 전투 PIE 스크린샷.
