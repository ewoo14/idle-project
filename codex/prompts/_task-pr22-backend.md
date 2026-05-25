펫 2종 + 시즌 패스 베타 V1 의 서버를 구현하라. 기획서: docs/planning/slices/22-pets-season-pass-v1.md. 기존 패턴 모사: server/src/modules/offline|quest/ (repo/routes/schema/service/test), core/data/skills.ts|quests.ts(읽기 전용 데이터), db/schema.ts + migrations(0001~0004), migrate.ts 다중 마이그레이션.

구현 범위 (이번 backend 호출):
1. core/data/pets.ts: 펫 2종 — { petId, name(한글), bonusType: "gold"|"drop", bonusPercent }. 강아지=gold +20, 새=drop +15 (1차).
2. core/data/season.ts: 무료 트랙 티어 정의 — { tier, requiredTokens, rewardType: "gold"|"exp", rewardAmount } 10티어(누적 토큰 증가).
3. modules/pet/: 펫 보유/장착(V1 1마리) + 보너스 조회. modules/season/: 시즌 토큰 잔량/누적, 티어 진행, 보상 수령(claim, 중복 방지, 재화 반영), 시즌 토큰 적립 API(POST, 퀘스트 완료 등에서 +N). db/schema.ts + migration 0005_pets_season(pet_state, season_state). server.ts 라우트 등록(/v1/pets, /v1/season).
4. Vitest: 펫 보너스 계산, 시즌 티어 도달/수령/중복 방지/토큰 누적.

제약: TypeScript 5.5, 한글 주석, 기존 에러/auth/응답 컨벤션. 클라/UI는 범위 외. **npm test + npm run build + npm run lint(biome) 전부 GREEN 확인(lint 누락 금지)**. push 금지. 커밋 prefix: codex(backend):.
