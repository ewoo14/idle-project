# M11 리더보드 클라이언트 V1 QA 시나리오

## 목적

PR #76 리더보드 클라이언트 연동에서 Power/Rebirth top-N, 내 순위, 오프라인 graceful 경로, 서버 rank parity를 회귀 검증한다.

## 시나리오 1: Power top-N 표시 모델

- Given: 서버가 Power top-N JSON을 `ok:true`와 함께 반환하고 score는 int64 문자열 또는 number로 포함된다.
- When: 클라이언트가 응답을 파싱하고 HUD 리더보드 패널 view model을 생성한다.
- Then: 순위, 점수, 캐릭터 id가 표시 행으로 변환되고 내 캐릭터 행은 highlight 상태가 된다.

자동화: `IdleProject.Leaderboard.ParseListJson`, `IdleProject.UI.HUD.LeaderboardPanelViewModel`

## 시나리오 2: Rebirth 내 순위

- Given: `/rebirth/me`가 내 rank/score를 반환하거나 미등록 캐릭터에 대해 데이터 없음으로 응답한다.
- When: 서버 `getMyRank("rebirth")`와 클라이언트 `ParseMyRankJson`을 실행한다.
- Then: 등록 캐릭터는 서버 rank/score를 유지하고, 미등록 캐릭터는 rank 0/score 0으로 표시된다.

자동화: `IdleProject.Leaderboard.ParseMyRankJson`, `server/src/modules/leaderboard/leaderboard.test.ts`

## 시나리오 3: 오프라인 및 malformed 응답

- Given: 네트워크 실패, malformed JSON, `ok:false`, `data:null` 응답이 발생한다.
- When: `RefreshLeaderboard` 또는 파서가 실패 응답을 처리한다.
- Then: 크래시 없이 빈 top-N, rank 0, score 0으로 수렴하고 HUD는 오프라인/로딩 라벨을 노출한다.

자동화: `IdleProject.Leaderboard.GracefulJson`, `IdleProject.Leaderboard.RefreshNoApiClientGraceful`, `IdleProject.UI.HUD.LeaderboardPanelViewModel`

## 시나리오 4: 서버 rank parity와 시즌 경계

- Given: Power/Rebirth 랭킹에 동점, 0점, 미등록 캐릭터, 다른 시즌 데이터가 존재한다.
- When: repo `getPowerRank`/`getRebirthRank`와 `/power/me`/`/rebirth/me` 라우트를 호출한다.
- Then: `rank()` window 기준 rank가 반환되고, 쿼리는 요청 season과 characterId로 제한되며, 미등록 캐릭터는 rank 0/score 0으로 직렬화된다.

자동화: `server/src/modules/leaderboard/leaderboard.test.ts`

## 회귀 기준

- SaveVersion, save payload, 밸런스 CSV, reward formula 변경 없음
- Redis top-N fallback과 PostgreSQL fallback 유지
- ko/en HUD 라벨 회귀 없음
- API 입력 검증은 season >= 1, limit 1..100, characterId UUID 유지
