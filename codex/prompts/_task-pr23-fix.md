PR #23 밸런스 시뮬레이터 V1 의 Codex TM 종합 검토 + fix (v3 [4]). 먼저 7파트 종합 검토 + Claude TM 대조를 stdout 출력(PM 게시). 그 다음 fix:

1. **리포트 생성물 리포 bloat 정리**: tools/balance-sim/reports/balance-sim-report.json (12000+줄)은 재생성 가능 산출물 → .gitignore 에 `tools/balance-sim/reports/*.json` 추가하고 git 추적에서 제거(git rm --cached). 요약 .md 는 유지(또는 .md 도 ignore 후 README 에 최신 분포 수치만 기록 — 둘 중 일관되게). README 에 "리포트는 npm run balance:sim 로 재생성" 명시.
2. **시뮬 가정/한계 문서화**: tools/balance-sim/README.md 에 V1 모델 가정(장비/펫/스킬 보너스 근사 방식, 사냥터 가정, 난수 시드)·한계·후속 정밀화 항목 명확화.
3. 검토 중 진짜 결함(시뮬 로직 오류, 공식 import 오용, 분포 비결정성) 발견 시 fix. 공식 상수는 타깃 내이므로 변경하지 말 것.

제약: TypeScript, 한글 문서/주석. 게임 코드 회귀 금지. **npm test + npm run build + npm run lint(biome) 전부 GREEN 확인**. push 금지. 커밋 prefix: codex(balance): 또는 codex(qa):.
