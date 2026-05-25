밸런스 시뮬레이터 V1 을 구현하라. 기획서: docs/planning/slices/23-balance-simulator-v1.md. 기존 공식 재사용: server/src/core/formulas/ (level.ts EXP/누적/강화, combat.ts ComputeDamage, stats.ts 직업 성장, offline.ts), core/data/(skills/quests/pets/season). 기존 도구 패턴: tools/balance/gen-level-curve.ts.

구현 범위 (이번 balance 호출):
1. tools/balance-sim/ (TS): 진행 시뮬레이터.
   - 모델: 시간 t 진행 → 사냥 DPS(직업/레벨/장비·펫·스킬 보너스 근사)로 킬/초 → EXP/골드 누적(권장레벨 사냥터 기준) → 레벨업(level.ts 누적 EXP 곡선) → Lv100 + 챕터1 보스 격파 가정 시점 = 환생 도달 시간. 오프라인 비중(offline.ts 효율) 반영.
   - 1000회 시뮬(결정적 시드 + 변동) → 환생 도달 시간 분포: median/p10/p90 (시간 단위).
   - 출력: 리포트(tools/balance-sim 결과 md 또는 json) — 현 수치 vs 타깃 5~10h, 항목별 민감도(EXP 지수/골드율/오프라인 효율).
   - 실행 스크립트(npm script 또는 ts-node/tsx) + README.
2. 보정 패스: 분포가 5~10h 벗어나면 보정 제안. **타깃 범위 내 최소 조정만 자동 적용**(예 level.ts EXP 지수 1.85~2.0 범위), 큰 변경은 리포트에 제안만. 공식 상수 보정 시 해당 *.test.ts 동기 갱신.
3. 시뮬 단위테스트(vitest): 결정적 시드 → 분포 재현, 경계.
4. docs/planning/05-balance-philosophy.md 의 "1차/시뮬레이터 도착 시" 표기를 시뮬 결과 반영으로 갱신 + 시뮬 사용법.

제약: TypeScript, 한글 주석/문서, 기존 formulas import 재사용(중복 구현 금지). 게임 코드 회귀 금지(기존 *.test.ts 유지). **npm test + npm run build + npm run lint(biome) 전부 GREEN 확인(lint 누락 금지)**. push 금지. 커밋 prefix: codex(balance):.
