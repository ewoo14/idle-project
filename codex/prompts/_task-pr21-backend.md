마법사+궁수 직업 V1 의 서버 SkillDB 미러를 확장하라. 기획서: docs/planning/slices/21-mage-archer-classes-v1.md §2.4. 기존: server/src/core/data/skills.ts (현재 전사 7종 읽기 전용 미러 + skills.test.ts), 클라 USkillComponent 의 마법사/궁수 스킬(커밋 7277615 — LoadDefaultMageSkills/LoadDefaultArcherSkills).

구현 범위 (이번 backend 호출):
1. server/src/core/data/skills.ts: classId 별로 마법사(classId 2)·궁수(classId 3) 스킬 7종씩 추가(전사 1과 동일 스키마: skillId/classId/displayName/type/effectType/cooldown/damageCoeff/buffMagnitude/buffDuration/gaugeGainOnHit/gaugeGainOnTakeDamage). 클라 USkillComponent 의 마법사/궁수 정의(client/Source/IdleProject/CombatSystem/SkillComponent.cpp)와 **id/type/effectType/cooldown/계수 동일**하게 미러.
2. skills.test.ts: 마법사/궁수 스킬 존재·필드 검증 + 직업별 7종 카운트. (가능하면 클라와의 parity 는 클라 DefinitionParity 테스트가 담당 — 서버는 자체 일관성.)

제약: TypeScript 5.5, 한글 주석, 기존 skills.ts 스타일. 클라/UI/스토리는 범위 외. 가능하면 npm test + tsc + lint(biome) 전부 확인(lint 누락 금지!). push 금지. 커밋 prefix: codex(backend):.
