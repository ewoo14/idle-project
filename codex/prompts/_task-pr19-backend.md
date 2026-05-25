환생 V1 의 서버 persist + 검증을 구현하라. 기획서: docs/planning/slices/19-boss-rebirth-v1.md §2.2. 기존: server/src/modules/character/ (repo/service/routes/test), db/schema.ts(이미 rebirthCount 존재), migrations(0001~0003), core/formulas/stats.ts(이미 환생 보너스 반영됨 — character 커밋 9cc8509).

구현 범위 (이번 backend 호출):
1. db/schema.ts: characters 에 rebirth_bonus_points(int, default 0) 컬럼 추가(rebirthCount 는 기존). migration 0004_rebirth_bonus_points(+ down). migrate.ts 다중 마이그레이션 패턴.
2. character 모듈: 환생 적용 — `POST /v1/characters/rebirth` (또는 service 메서드). 서버 권위 검증: 조건(bossDefeated 플래그 + level>=100)을 요청/세이브 상태로 검증 → rebirthCount++, rebirth_bonus_points += 5, level=1 리셋, gold = floor(gold*0.1) 보존. (보스 격파 플래그를 characters 또는 save 상태에서 받음 — 최소 구현: 요청 바디 또는 기존 save 필드.)
3. Vitest(character.test 확장 또는 신규): 조건 미충족 reject, 환생 적용(카운트/포인트/레벨/골드 보존율), 다회 환생 누적, deriveStats 환생 보너스 반영 정합.

제약: TypeScript 5.5, 한글 주석, 기존 에러/auth/응답 컨벤션, server.ts 라우트 등록. stats.ts deriveStats 의 환생 보너스(포인트당 HP+10/PhysAtk+2)는 이미 반영됨 — 수치 변경 금지, persist/검증만. 클라/UI 범위 외. 가능하면 npm test + tsc + lint. push 금지. 커밋 prefix: codex(backend):.
