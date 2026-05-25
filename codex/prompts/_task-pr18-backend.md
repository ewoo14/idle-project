퀘스트 시스템 V1 의 서버를 구현하라. 기획서: docs/planning/slices/18-quest-system-v1.md (먼저 읽고 따른다). 기존 패턴 정확히 모사: server/src/modules/offline/ (repo/routes/schema/service/test 구조), server/src/core/data/skills.ts (읽기 전용 데이터), server/src/db/schema.ts + migrations, server/src/core/formulas/.

구현 범위 (이번 backend 호출):
1. server/src/core/data/quests.ts: 퀘스트 정의(읽기 전용).
   - 타입: QuestType = "main" | "daily"; QuestObjective = "kill_monster" | "clear_map" | "claim_offline" | "enhance".
   - Quest: { questId, type, title(한글), objective, targetCount, rewardGold, rewardExp, prerequisiteQuestId?(메인 체인), chapterMapId?(메인) }.
   - 데이터: 메인 챕터1 퀘스트(스토리바이블 docs/planning/06-story-bible.md §5.1 맵 훅 기반 — 1-1 슬라임 처치 … 1-5 보스, 체인 prerequisite 연결) + 일일 3종(kill_monster N / claim_offline 1 / enhance N).
2. server/src/modules/quest/: offline 모듈 패턴.
   - schema(quest.schema.ts): 요청/응답 zod. service: 진행 누적/완료 판정/수령(보상→characters.gold/total_exp 반영)/일일 lazy 리셋(UTC 자정 기준, 마지막 리셋일 != 오늘이면 daily 초기화). repo: quest_progress(userId, questId, progress, completed, claimed, dailyResetDate). routes: GET /v1/quests, POST /v1/quests/:id/progress, POST /v1/quests/:id/claim.
   - db/schema.ts + drizzle migration(0003_quest_progress) — migrate.ts 다중 마이그레이션 패턴.
3. Vitest(service.test): 진행 누적/완료/중복수령 방지/일일 리셋(날짜 경계)/메인 체인 해금/보상 반영.

제약: TypeScript 5.5, 한글 주석, 기존 에러/응답/auth 플러그인 컨벤션. 라우트 server.ts 등록. 클라(UE)/UI/진행 훅은 범위 외(후속). 가능하면 npm test + tsc + lint 검증. push 금지. 커밋 prefix: codex(backend):.
