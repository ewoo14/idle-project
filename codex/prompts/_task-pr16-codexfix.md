PR #16 오프라인 보상 V1 의 Codex TM 종합 검토 + fix 를 수행하라(v3 단계 [4], 리뷰+fix 일체). 기획서: docs/planning/slices/16-offline-rewards-v1.md.

먼저 7파트 관점 종합 검토(블로커/중요/권장/질문/칭찬, 한글) + 이미 게시된 Claude TM 리뷰 3개 권장과 대조 — 동의/이견/누락. 검토 결과를 stdout 으로 출력(PM 이 PR 코멘트 게시). 그 다음 아래 fix:

fix 범위 (경미 — 진짜 이슈 발견 시 추가):
1. 엣지/회귀 테스트 보강(서버 offline.test.ts + 클라 OfflineRewardFormula 테스트):
   - cap 정확 경계(정확히 12h = 43200s, 43199/43201s 비교),
   - 매우 큰 level·시간 오버플로 안전성(서버 number / 클라 int64),
   - 음수/미래 lastSeen → 0,
   - 서버 결과와 클라 결과 동일성 스폿체크(동일 입력 → gold/exp 동일).
2. Claude TM 가 동의한 항목 중 진짜 결함만 fix. **밸런스 수치(0.75 / 0.005 / 0.05 / level×4 / 12h)는 변경 금지**(시뮬레이터 후속 보정 대상).
3. baseGold/baseExp 분리는 밸런스 후속 — 이번엔 하지 말 것(수치 동결).

제약: 서버 TS + 클라 C++ 양쪽 공식 일치 유지. 한글 주석. 가능하면 npm test(vitest) + Build.bat + Automation RunTests IdleProject 로 검증(서버+UE 전부 GREEN 유지). push 금지. 커밋 prefix: codex(qa): 또는 codex(backend):.
