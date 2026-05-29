# PR #84 Dark 저주(Curse) — QA 노트 (Given/When/Then)

> PM/Claude 직접 작성(Codex 한도 대체). 테스트는 character 파트에 포함.

## 시나리오
### S1. 저주 피해 증폭
- Given 대상에 Curse(Magnitude m) 활성, When 피해 적용(TakeDamageTyped), Then 받는 피해 ×(1+m). (커버: `Combat.CurseAmplifiesDamage`)

### S2. 회귀 (저주 없음)
- Given 저주 비활성, When 피해, Then 피해 불변(증폭 없음). (커버: `Combat.CurseAbsentDamageUnchanged`)

### S3. 다른 상태와 독립
- Given Burn/Poison/Freeze 동시, When 저주 활성, Then 각 상태 독립 동작(DoT는 틱, 슬로우는 공속, 저주는 받는 피해 증폭) — 저주 증폭은 단일 지점, 중복 없음. (커버: `Combat.CurseIndependentOfOtherStatuses`)

### S4. Dark 스킬 부여
- Given Dark 스킬(shadow_stab/void_call) 적중, Then 대상에 Curse 부여(지속/Magnitude). (커버: `Combat.DarkSkillAppliesCurse`)

### S5. 만료
- Given 저주 지속 경과, Then TickStatuses 만료 → 증폭 원복(DoT 틱 없음).

### S6. parity
- 서버 `applyCurseAmplification` + skills.ts Dark→curse 부여가 클라와 일치. (커버: server combat/skills vitest)

## 검증 명령
- 서버: `cd server; npm run lint && npm run test -- combat skills` (40)
- 클라: `tools/ci/ue-automation.ps1 -Filter "IdleProject.Combat+IdleProject.UI.HUD"` (또는 전체)

## 비고
- 저주 HUD는 글리프 인디케이터(Burn/Poison/Freeze와 동일 방식, 별도 로컬라이즈 키 불필요) — character가 추가. designer 파트 충족.
- **세이브 변경 없음**. [5] PM 검증은 `tools/ci/ue-automation.ps1`(전체 IdleProject) 게이트.
