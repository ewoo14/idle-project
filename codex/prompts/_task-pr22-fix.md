PR #22 펫+시즌패스 베타 V1 의 Codex TM 종합 검토 + fix (v3 [4]). 먼저 7파트 종합 검토 + Claude TM 대조를 stdout 출력(PM 게시). 그 다음 fix:

1. **밸런스 문서**: docs/planning/05-balance-philosophy.md 에 펫 보너스 %(강아지 골드+20/새 드롭+15) + 시즌 패스 무료 10티어 요구 토큰·보상 1차 표 + 시뮬레이터 후속 보정 명시.
2. **QA 시나리오**: docs/qa/scenarios/ 에 펫 장착→골드/드롭 보너스 / 시즌 토큰 누적→티어 도달→수령(중복 방지) Given/When/Then.
3. **parity/경계 보강**: 펫 보너스 서버↔클라 동일 % parity 테스트(서버 Vitest + UE Automation), 시즌 최종 티어 도달/토큰 초과/이미 수령 티어 경계 테스트.
4. 검토 중 진짜 결함(펫 보너스 미적용 경로, 시즌 중복 수령, 토큰 음수, parity 불일치) 발견 시 fix. 수치는 parity 정합 외 변경 금지.

제약: 서버 TS + 클라 C++ 정합. 전부 한글. **npm test + npm run build + npm run lint(biome) + Build.bat + Automation 전부 GREEN 확인(lint 누락 절대 금지)**. markdownlint 신규 문서 통과. push 금지. 커밋 prefix: codex(qa):.
