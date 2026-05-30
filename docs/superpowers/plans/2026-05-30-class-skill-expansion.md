# 클래스 스킬 확장 구현 계획

> 스펙: [`2026-05-30-class-skill-expansion-design.md`](../specs/2026-05-30-class-skill-expansion-design.md). v3 디스패치, 현행 재검증. **세이브 무변경**.

**Goal:** 8 클래스에 신규 스킬 1종씩 추가(데이터 구동). DPS 밴드 ±15% 유지.

**Architecture:** 클라 SkillComponent + 서버 skills.ts 데이터 미러 + class-balance 튜닝. generic ExecuteSkill 재사용.

## Task 1: character+backend (character)
- [ ] 클라 `CombatSystem/SkillComponent.cpp`: 8 클래스 각 신규 스킬 1종 `Skills.Add(MakeSkill(...))`(기존 `MakeSkill` 시그니처·effect 재사용, 클래스 정체성·속성 맞춤). 서버 `server/src/core/data/skills.ts`: 동일 8종 미러(classXxxSkillDefinitions에 추가, id/effect/계수 1:1).
- [ ] 클라↔서버 skills parity: `skills.test.ts`(클라 정의와 일치 검증 — 기존 parity 테스트 확장), 클라 스킬 테스트(신규 스킬 존재/effect/클래스).
- [ ] `cd server; npm run lint && npx vitest run src/core/data && npm run build` GREEN.
- [ ] 커밋 `feat: 클래스 신규 스킬 8종 (스킬 확장)`.

## Task 2: balance (balance)
- [ ] 신규 스킬 DamageCoeff/쿨다운 튜닝 → **class-balance.test ±15% 밴드 유지**(balance-sim에 신규 스킬 반영, 8 클래스 DPS 재계산). 밴드 위반 클래스 계수 조정.
- [ ] `npx vitest run`(class-balance 포함) GREEN. `docs/planning/class-skill-expansion-balance-note.md`(8종 계수·DPS 밴드 검증·median 영향).
- [ ] 커밋 `balance: 클래스 스킬 확장 DPS 밴드 튜닝 (스킬 확장)`.

## Task 3: designer (designer)
- [ ] 신규 스킬 8종 표시명 ko/en(스킬명 로컬라이즈 위치 확인 — SkillDB.csv/스킬 CSV) + CsvIntegrity. 스킬 HUD가 데이터 구동 표시되는지 확인(신규 코드 최소). 표준 jumbo.
- [ ] 커밋 `feat: 클래스 신규 스킬 로컬라이즈 (스킬 확장)`.

## Task 4: qa (qa)
- [ ] E2E: 신규 스킬 8종 실행(effect별 DamageSingle/Aoe/SelfBuff/Dash/Heal)·클래스 parity·DPS 밴드(class-balance GREEN)·세이브 무변경(SkillRanks 전방호환). 표준 jumbo + ue-automation.ps1 게이트. 커밋 `test: 클래스 스킬 확장 E2E (스킬 확장)`.
- [ ] [[project_pr_order]]/[[project_session_progress]] 갱신.

## Self-Review
- 스펙 §4 전부 매핑 ✓. parity: skills.ts ↔ SkillComponent(8종 id/effect/계수) — skills.test TM cross-check.
- **DPS 밴드 ±15%(#60)**: class-balance.test 서버 게이트가 안전망 — balance가 계수 튜닝으로 유지.
- **세이브 무변경**: SkillRanks 전방호환(신규 id 기본 랭크). effect 코드 무변경(기존 ESkillEffectType만).
- jumbo ODR(신규 익명 헬퍼 없으면 무관).

## 매핑: 1→character(메인+backend), 2→balance, 3→designer, 4→qa.
