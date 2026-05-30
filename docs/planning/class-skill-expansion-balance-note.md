# 클래스 스킬 확장 밸런스 노트

> 대상: 클래스 스킬 확장 슬라이스 · 작성 2026-05-30 · 스펙 [`2026-05-30-class-skill-expansion-design.md`](../superpowers/specs/2026-05-30-class-skill-expansion-design.md)
> **세이브 무변경**(SaveVer 24, SkillRanks 전방호환).

## 1. 신규 스킬 8종 (클래스별 1종)

| 클래스 | id | 이름 | Effect | CD | Coeff | 속성/상태 |
| --- | --- | --- | --- | --- | --- | --- |
| Warrior | earthen_cleave | 대지 가르기 | DamageAoe | 6.0 | 3.2 | — |
| Mage | flame_storm | 화염 폭풍 | DamageAoe | 7.0 | 3.0 | Fire/Burn |
| Archer | multi_shot | 다중 사격 | DamageAoe | 5.5 | 3.2 | — |
| Thief | shadow_strike | 그림자 일격 | DamageSingle | 6.0 | 3.0 | Dark/Curse |
| Cleric | divine_grace | 신성 가호 | SelfBuff | 12.0 | (버프 0.3/5s) | Holy |
| Paladin | judgment | 심판 | DamageSingle | 8.0 | 2.4 | Holy |
| Berserker | blood_rage | 피의 격노 | DamageSingle | 6.0 | 3.0 | — |
| Summoner | spirit_burst | 정령 폭발 | DamageAoe | 6.0 | 3.4 | — |

- 전부 기존 ESkillEffectType 재사용(신규 effect 코드 없음). 데이터 구동(generic ExecuteSkill).

## 2. DPS 밴드 ±15% (#60) 유지 — 검증 GREEN

- **Lv50**: 최대 |median 대비 delta| = Archer −7.8% (≪ ±15%). Mage +4.3%/Thief −5.8%/Warrior −3.7%/Berserker +1.1%/Summoner 0%.
- **Lv100**: 최대 |delta| = Warrior 4.6%, 나머지 ≤3% — 신규 전 대비 오히려 압축.
- **Tank/Healer 분리 유지**: Paladin(심판 CD8.0/coeff2.4 보수) Lv100 eff 4223 < median×0.8=4504, Cleric 3378 < 4504 — 0.8 임계 통과(딜러 밴드 미편입).
- `class-balance.test`(서버 vitest) GREEN — 신규 스킬 반영 후 8 클래스 균형 유지.

## 3. 경제/페이싱

- 신규 스킬은 전 클래스 균등 비율 보강 → 절대 DPS는 상승(무한 성장 정합)하되 **상대 밸런스 불변**. median 페이싱 영향은 통합 재측정.
- 계수 초기값 — 신규 스킬 사용 빈도(쿨다운/AI 로테이션) 데이터로 재튜닝.

## 4. 세이브

- **무변경**(SaveVer 24). 신규 스킬 id는 SkillRanks 미존재 시 기본 랭크(전방호환), 마이그레이션 불필요.
