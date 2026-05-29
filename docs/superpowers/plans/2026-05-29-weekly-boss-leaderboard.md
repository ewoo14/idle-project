# 주간 보스 데미지 리더보드 구현 계획 (선행, PR #79)

> 선행 기획. Codex 복구(6/1) 후 v3 디스패치. 디스패치 시 현행 코드 재검증.

**Goal:** #77 주간 보스 누적 데미지를 #76 리더보드에 ISO week 키 랭킹으로 추가.

## Task 1: backend (backend)
- [ ] 마이그레이션: `leaderboard_weekly_damage(week_id text, character_id uuid, damage bigint, updated_at, pk(week_id,character_id))`.
- [ ] repo: `upsertWeeklyDamage(weekId, characterId, damage)`, `listWeeklyDamage(weekId, limit)`, `getWeeklyDamageRank(weekId, characterId)`(rank window).
- [ ] service: `updateWeeklyDamage`, `getWeekly(weekId, limit)`, `getMyWeeklyRank(weekId, characterId)`. Redis 캐시는 선택(week 키).
- [ ] routes: `GET /leaderboard/weekly?week=&limit=` , `/weekly/me?week=&characterId=`. schema(week string 필수).
- [ ] save.service: 업로드 시 payload.weeklyBossWeekId/weeklyBossDamage 있으면 `updateWeeklyDamage` 호출(기존 updatePower/updateRebirth 옆).
- [ ] vitest: 순위 정확/미등록/주 분리/제출. `npm run lint && test -- leaderboard save && build` GREEN(lint 필수).
- [ ] 커밋 `feat: 주간 보스 데미지 리더보드 backend (PR #79)`.

## Task 2: client (character)
- [ ] ELeaderboardKind에 WeeklyDamage 추가. ApiClient `FetchWeeklyDamageLeaderboard(week, cb)`/`FetchMyWeeklyRank(week, characterId, cb)`.
- [ ] ULeaderboardService 주간 종류 파싱/보관(기존 FLeaderboardEntry 재사용, score=damage). GameInstance `RefreshLeaderboard(WeeklyDamage)`=`UQuestService::GetCurrentUtcWeekString()` 사용.
- [ ] Automation: 주간 JSON 파싱/내 순위(서버 무의존).
- [ ] 커밋 `feat: 주간 데미지 리더보드 클라 (PR #79)`.

## Task 3: UI (designer)
- [ ] 리더보드 패널 "주간 보스" 탭(현재 주 라벨, top-N, 내 순위) + ko/en + CsvIntegrity. 표준 jumbo 빌드.
- [ ] 커밋 `feat: 주간 데미지 리더보드 탭 UI (PR #79)`.

## Task 4: balance (balance)
- [ ] `docs/planning/weekly-boss-leaderboard-note.md`(랭킹=주간 데미지=CP×도전, 공식 무변경).
- [ ] 커밋 `docs: 주간 데미지 리더보드 노트 (PR #79)`.

## Task 5: qa (qa)
- [ ] 제출→조회→표시, 내 주간 순위(등록/미등록), 주 경계(다음 주 빈), parity. 표준 jumbo 빌드 + Automation.
- [ ] 커밋 `test: 주간 데미지 리더보드 E2E (PR #79)`.

## Self-Review
- 세이브 변경 없음. 기존 리더보드 불변(추가만). jumbo ODR 주의. 명칭 일관(WeeklyDamage/getWeeklyDamageRank).

## 매핑: 1→backend, 2→character(메인), 3→designer, 4→balance, 5→qa.
