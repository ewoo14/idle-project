PR #24 로컬라이즈 한/영 V1 의 Codex TM 종합 검토 + fix (v3 [4]). 먼저 7파트 종합 검토 + Claude TM 대조를 stdout 출력(PM 게시). 그 다음 fix:

1. **영문 번역 품질 보강**: client/Content/Localization/Game/en/ 의 Story.csv/StoryText.csv/Skill.csv/Quest.csv/UI.csv 영문을 자연스러운 게임 영어로 교정(직역/placeholder 제거). 고유명사 일관(스토리 바이블 용어집 기준 — 루미나=Lumina, 안개 군주=Mist Lord 등 일관 표기). ko 키셋과 100% 동일 유지(미번역/누락 0).
2. **Story.csv ↔ StoryText.csv 중복 정리**: 둘 다 19키로 중복 → 역할 구분(예: StoryText=대사 라인, Story=시놉시스 요약) 명확화하거나 하나로 통합. 중복이면 통합 + 참조 갱신.
3. **QA 시나리오**: docs/qa/scenarios/ 에 언어 전환(ko↔en) → UI/스토리 표시 언어 변경 Given/When/Then.
4. 검토 중 진짜 결함(키 누락/중복, 컬처 적용 누락, 깨진 placeholder {PlayerName} 등) 발견 시 fix.

제약: 전부 정합(ko↔en 키 동일). 한글 주석/문서(번역 산출은 en). 가능하면 Build.bat + Automation(키정합/조회) + CSV 검증 GREEN. markdownlint 신규 문서 통과. push 금지. 커밋 prefix: codex(story): 또는 codex(qa):.
