# PR #57 기획서 — 캐릭터 직업 추가 (콘텐츠 볼륨)

> **사용자 지시([[project-content-richness]]): 캐릭터 종류를 많이.** 현재 5직업(전사/마법사/궁수/도적/성직자, #21/#29). **신규 3직업 추가 — 성기사(Paladin)/광전사(Berserker)/소환사(Summoner)** → 8직업. 각 직업별 7스킬(액티브/패시브/궁극기) + 스탯 성장 프로필 + 서버 SkillDB 미러(parity) + 직업 선택 HUD + 한글 로컬. server + client 멀티시스템(+designer/qa).

## 1. 목표 / DoD
플레이어가 8직업 중 선택하고, 신규 3직업이 고유 스킬 7종 + 차별화된 스탯 성장으로 자동 전투한다. 클라/서버 정의 parity.

### DoD 검증
1. **EClassId 확장**: Warrior~Cleric(1~5) → +**Paladin=6, Berserker=7, Summoner=8**. 클라 StatFormulas.h + 서버 정합.
2. **직업별 스탯 성장(FClassGrowth)**: 신규 3직업 FClassGrowth 프로필(클라 StatFormulas.cpp + 서버 stat 미러). 차별화 — 성기사(STR/CON 탱커·물방/HP↑), 광전사(STR/물공 극공·방어↓), 소환사(INT/WIS 마공·지속딜). GetClassGrowth 분기 추가.
3. **직업별 스킬 7종**: 클라 SkillComponent LoadDefaultPaladin/Berserker/SummonerSkills + LoadSkillsForClass switch. 각 직업 테마 스킬(성기사=신성/방어버프/도발, 광전사=격노/광폭화/자가피해딜, 소환사=소환수/지속마법/광역). ESkillEffectType/Element/Status 기존 재사용.
4. **서버 SkillDB 미러**: server skills.ts 에 paladin/berserker/summonerSkillDefinitions + skills.test.ts parity(클라↔서버 skillId/type/effect/coeff/cooldown 정합). expectedSkillDefinitionsByClass 확장.
5. **직업 선택 HUD**: 직업 선택 UI 에 신규 3직업 추가(이름/설명). 한글 로컬라이즈 ko/en.
6. **직업 스토리(선택)**: 06-story-bible 또는 직업 분기 텍스트에 신규 직업 소개(간단). 로컬.
7. **테스트**: 클라 Automation(직업별 스킬 로딩·스탯 성장·DefinitionParity 8직업) + 서버 vitest(skills parity 8직업·스탯) + CsvIntegrity. UE 빌드/Automation + 서버 build/test/lint GREEN.

## 2. 범위 (In Scope)
### 2.1 직업 정의 (character + backend 미러)
- EClassId +3. FClassGrowth +3(클라 StatFormulas.cpp + 서버 stat 미러). SkillComponent LoadDefault×3 + switch + 7스킬×3. 서버 skills.ts +3직업 정의 + parity.
### 2.2 UI (디자이너)
- 직업 선택 HUD 8직업 + 신규 이름/설명 로컬라이즈 ko/en.
### 2.3 스토리/로컬 (character/designer)
- 신규 직업 이름/설명/스킬명 ko/en. (스토리 분기 간단.)
### 2.4 테스트 — 위 DoD 7.

## 3. 범위 외 (후속)
- 추가 직업(네크로맨서/격투가/총사/음유시인 등 — 후속 슬라이스), 직업 전직/승급 시스템, 직업별 전용 장비/세트, 직업 밸런스 정밀 튜닝.

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EClassId+3 + FClassGrowth+3 + SkillComponent LoadDefault×3+switch + 7스킬×3 + Automation | ✅ 메인 (`character`) |
| 백엔드 | 서버 skills.ts +3직업 정의 + 스탯 성장 미러 + parity vitest | ✅ 보조 (`backend`) |
| 디자이너 | 직업 선택 HUD 8직업 + 이름/설명 로컬라이즈 | ✅ 보조 (`designer`) |
| QA | 신규 3직업 선택→스킬 로딩→자동전투→스탯 차별화 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증(UE+서버) → [N] **CI 그린 확정**(server-ci 포함) + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). [[project-content-richness]].

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 클라/서버 스킬 정의 불일치 | skills.test parity(skillId/type/effect/coeff/cooldown) 8직업 + DefinitionParity |
| 스탯 성장 클라/서버 미러 불일치 | FClassGrowth 클라↔서버 동일 값 + parity 테스트 |
| 신규 직업 밸런스 극단 | 기존 5직업 패턴 참조한 합리적 계수, 정밀 튜닝은 후속 |
| 직업 선택/스토리 분기 누락 | LoadSkillsForClass switch 전 직업 + HUD 8직업 + 미선택 기본 Warrior |
| 기존 5직업 회귀 | 기존 EClassId/스킬/성장 무변경, 신규만 추가 |
| 로컬라이즈 영문 잔존 | 직업명/스킬명/설명 ko/en + CsvIntegrity |

## 7. 후속 (콘텐츠 확장 계속 — 사용자 지시)
- **다음**: 아이템 종류 확대(ID/세트/고유옵션 다양화). 이후 추가 직업, 전직/승급, 직업 전용 장비.
