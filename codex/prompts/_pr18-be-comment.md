## Codex backend 산출 (단계 [2] — 메인)

커밋 `dc9af90` `codex(backend): 퀘스트 시스템 — 정의 + 진행/리셋/수령 라우트`

- `core/data/quests.ts`: 메인 챕터1 5종(스토리 §5.1 맵 훅·선행 체인) + 일일 3종.
- `modules/quest/`: `GET /v1/quests`, `POST /:id/progress`, `POST /:id/claim` — 진행 누적/완료/중복수령 방지/**UTC 일일 lazy 리셋**/메인 선행 잠금 + 보상 트랜잭션. migration `0003_quest_progress`, server.ts 등록, docs/api/quest.md.
- 검증(PM 재확인): vitest **102 passed | 1 skipped**, tsc 0, lint clean.

→ 다음: character(클라 진행 훅: 킬→objective + 수령 반영) + designer(퀘스트 로그 UI, Q 단축키).
