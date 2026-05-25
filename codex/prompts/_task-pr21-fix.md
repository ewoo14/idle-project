PR #21 마법사+궁수 직업 V1 의 Codex TM 종합 검토 + fix (v3 [4]). 먼저 7파트 종합 검토 + Claude TM 대조를 stdout 출력(PM 게시). 그 다음 fix(Claude TM [3] 권장 흡수):

1. **서버↔클라 스킬 parity 확장**: 기존 DefinitionParity 테스트(전사)를 마법사(classId 2)·궁수(classId 3)까지 확장 — server/src/core/data/skills.ts ↔ 클라 USkillComponent(LoadDefaultMage/ArcherSkills) 의 3직업 전체 스킬 id/type/effectType/cooldown/계수 동일성 검증(서버 Vitest + UE Automation 양쪽).
2. **직업 스토리 분기**(콘텐츠): docs/planning/06-story-bible.md §3(주인공) 또는 신규 직업 섹션에 마법사/궁수 배경·동기 1문단씩 + client/Content/Localization/Game/ko/StoryText.csv 에 직업 인트로 키. GDD 톤 정합.
3. **밸런스 문서**: docs/planning/05-balance-philosophy.md 에 마법사(고DPS·종이방어)/궁수(크리·중거리) 직업 밸런스 1차 표 + 시뮬레이터 후속 보정 명시.
4. 검토 중 진짜 결함(직업 분기 누락, 스킬 effectType 오류, 마법/크리 계산 이상) 발견 시 fix. 스킬 수치는 parity 정합 목적 외 임의 변경 금지.

제약: 서버 TS + 클라 C++ 정합. 전부 한글. **npm test + npm run build + npm run lint(biome) + Build.bat + Automation 전부 GREEN 확인**(lint 누락 금지). markdownlint 통과. push 금지. 커밋 prefix: codex(qa): 또는 codex(story):.
