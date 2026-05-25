PR #18 퀘스트 시스템 V1 의 Codex TM 종합 검토 + fix (v3 [4], 리뷰+fix 일체). 먼저 7파트 관점 종합 검토 + Claude TM 리뷰 대조를 stdout 출력(PM 게시). 그 다음 fix:

fix (Claude TM [3] 권장):
1. **서버↔클라 퀘스트 정의 값 parity 테스트** 추가: 서버 server/src/core/data/quests.ts 와 클라 QuestService 정의가 questId 별 targetCount/rewardGold/rewardExp/objective/type/prerequisite 동일한지 검증. 서버 Vitest 1개 + 클라 Automation 1개(같은 기대표). 불일치 발견 시 한쪽을 기준으로 정합(서버 quests.ts 를 기준으로 클라 보정).
2. 진행 objective 회귀 보강(가능 범위): kill_monster 외 claim_offline/enhance 진행 누적 + 메인 체인 해금 경계 테스트.
3. 검토 중 진짜 결함(리셋 날짜 경계 off-by-one, 중복수령, 보상 합산 오류, 메인/일일 혼선) 발견 시 fix. 수치 정의 임의 변경 금지(parity 정합 목적의 보정만).

제약: 서버 TS + 클라 C++ 정합 유지. 한글 주석. 가능하면 npm test + Build.bat + Automation RunTests IdleProject 로 서버+UE 전부 GREEN 유지. push 금지. 커밋 prefix: codex(qa): 또는 codex(backend):.
