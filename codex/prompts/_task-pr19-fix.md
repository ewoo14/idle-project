PR #19 챕터1 보스 + 환생 V1 의 Codex TM 종합 검토 + fix (v3 [4]). 먼저 7파트 종합 검토 + Claude TM 대조를 stdout 출력(PM 게시). 그 다음 fix:

fix (Claude TM [3] 권장):
1. **환생 스탯 보너스 parity 테스트**: 서버 server/src/core/formulas/stats.test.ts ↔ 클라 StatFormulas Automation 에서 동일 RebirthBonusPoints(예 0/5/10) 입력 시 HP(+10/pt)·PhysAtk(+2/pt) 동일 결과 검증.
2. **다회 환생 + 경계 회귀**: 환생 2~3회 누적(포인트/카운트), 골드 보존율 0.1 경계(소수 버림), 레벨 리셋 후 deriveStats 정합.
3. **보스 격파→환생 자격 전이** 통합 시나리오 테스트(플래그 set → CanRebirth true / 미격파 false / Lv 미달 false).
4. 검토 중 진짜 결함(환생 후 rebirthCount 오프라인 미반영, 보존율 오류, 중복 환생, deriveStats parity 어긋남) 발견 시 fix. 수치(포인트 5/보존 0.1/HP10/Atk2) 임의 변경 금지(parity 정합만).

제약: 서버 TS + 클라 C++ 정합 유지. 한글 주석. npm test + Build.bat + Automation RunTests IdleProject 로 서버+UE 전부 GREEN 유지. push 금지. 커밋 prefix: codex(qa):.
