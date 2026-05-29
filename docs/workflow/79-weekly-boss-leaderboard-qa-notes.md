# PR #79 주간 보스 데미지 리더보드 — QA 노트 (Given/When/Then)

> PM/Claude 직접 작성(Codex 한도 + API 과부하 대체). 테스트는 backend/character 파트에 이미 포함, 본 노트는 커버리지 종합.

## 시나리오

### S1. 제출 (세이브 업로드 경유)
- Given 캐릭터가 주간 보스 도전으로 weeklyBossDamage>0·weeklyBossWeekId 보유, When 세이브 업로드, Then `save.service`가 `updateWeeklyDamage(characterId, weekId, BigInt(damage))` 호출 → `leaderboard_weekly_damage` upsert. (커버: `save.test.ts` 훅 테스트)

### S2. 조회 (top-N / 내 순위)
- Given 같은 week_id에 여러 캐릭터 기록, When `GET /v1/leaderboard/weekly?week=&limit=`, Then damage desc top-N. When `/weekly/me?week=&characterId=`, Then 내 rank/score(미등록 {rank:0,score:"0"}). score는 bigint→문자열. (커버: `leaderboard.test.ts` 주간 순위/미등록/주 분리)

### S3. 클라 파싱 (서버 무의존)
- Given 주간 top-N JSON, When `ULeaderboardService` 파싱, Then `FLeaderboardEntry`(score 문자열→int64=damage, rank). 내 순위 파싱. 빈/오류 graceful. (커버: `IdleProject.Leaderboard` Automation 4 신규: ParseWeeklyDamageListJson/ParseMyWeeklyRankJson/WeeklyDamageGracefulJson/RefreshWeeklyDamageNoApiClientGraceful)

### S4. 표시 (UI 탭)
- Given 리더보드 패널, When "주간 보스" 탭 선택, Then 현재 주(`GetCurrentUtcWeekString`) 라벨 + top-N(데미지 콤마) + 내 순위 하이라이트. `RefreshLeaderboard(WeeklyDamage)` 호출. (커버: `IdleProject.UI.HUD.LeaderboardPanelViewModel` + `CsvIntegrity`)

### S5. 주 경계
- Given 다음 주 진입, When 조회, Then 새 week_id 보드(이전 주와 분리). (커버: `leaderboard.test.ts` 주 분리 케이스)

### S6. 회귀
- 기존 Power/Rebirth 리더보드·세이브·던전 패널 불변. SaveVersion 15 유지(세이브 변경 없음). (커버: 기존 Automation + #75 던전 패널 stale 테스트 정정 포함)

## 검증 명령
- 서버: `cd server; npm run lint && npm run test -- leaderboard save` (56 tests)
- 클라: 표준 jumbo(unity) 빌드(`-DisableUnity` 없이) + `Automation RunTests IdleProject.Leaderboard` (9) + `IdleProject.UI.HUD` + `IdleProject.Localization.CsvIntegrity`

## 비고
- Codex 한도 소진(6/1) + balance/qa 서브에이전트 API 529 과부하로 balance/qa 문서는 PM/Claude 직접 작성. 구현/테스트 코드는 backend(claude)·character(claude)·designer(claude) 서브에이전트 산출.
- 주간 랭크 보상(주간 종료 보상)은 후속(#78 시즌 보상 패턴 적용).
