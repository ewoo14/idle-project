# PR #76 리더보드 클라이언트 QA 노트

## 범위

- 대상 PR: #76 리더보드 클라이언트 연동 및 UI
- QA 역할: Section 7 테스트/회귀 검증
- 기준 문서: `docs/workflow/03-review-checklist.md` Section 7, `docs/planning/slices/76-leaderboard-client.md` 2.5, `docs/superpowers/plans/2026-05-29-leaderboard-client.md` Task 5

## Given / When / Then

### 1. Top-N JSON 파싱

- Given: 서버가 `ok:true`와 top-N 배열을 반환하고 score가 문자열 int64 또는 number로 온다.
- When: `ULeaderboardService::ParseListJson`이 Power/Rebirth 응답을 파싱한다.
- Then: `CharacterId`, `Score`, `Rank`가 보존되고 Power/Rebirth 캐시가 서로 섞이지 않는다.

자동화:
- `IdleProject.Leaderboard.ParseListJson`
- `IdleProject.Leaderboard.RankBoundaryJson`

### 2. 내 순위 JSON 파싱

- Given: `/power/me` 또는 `/rebirth/me`가 내 `rank`와 `score`를 반환한다.
- When: `ULeaderboardService::ParseMyRankJson`이 응답을 파싱한다.
- Then: 내 순위 캐시에 값이 저장되고 미등록 캐릭터의 rank 0/score 0은 unranked sentinel로 유지된다.

자동화:
- `IdleProject.Leaderboard.ParseMyRankJson`
- `IdleProject.Leaderboard.RankBoundaryJson`

### 3. 빈 배열 및 malformed JSON

- Given: 서버 응답이 빈 배열, `ok:false`, malformed JSON, 또는 `data:null`이다.
- When: 클라이언트 파서와 `RefreshLeaderboard` 실패 경로가 실행된다.
- Then: 크래시 없이 빈 top-N과 rank 0/score 0으로 수렴하며 UI 모델은 오프라인/로딩 상태를 표시할 수 있다.

자동화:
- `IdleProject.Leaderboard.GracefulJson`
- `IdleProject.Leaderboard.RefreshNoApiClientGraceful`
- `IdleProject.UI.HUD.LeaderboardPanelViewModel`

### 4. 서버 내 순위 parity 및 시즌 경계

- Given: Power/Rebirth 랭킹 테이블에 동점, 0점, 미등록 캐릭터, 시즌별 데이터가 있다.
- When: `getPowerRank`, `getRebirthRank`, `getMyRank`, `/power/me`, `/rebirth/me`를 호출한다.
- Then: `rank()` window 결과를 그대로 반환하고, 미등록 캐릭터는 rank 0/score 0으로 직렬화되며, 쿼리는 `season_id = $1`과 `character_id = $2` 경계를 유지한다.

자동화:
- `server/src/modules/leaderboard/leaderboard.test.ts`

## 회귀 체크

- 기존 leaderboard 기록/upsert, top-N 조회, Redis fallback 테스트 유지
- 기존 HUD view model localization 테스트 유지
- SaveVersion 및 save payload 변경 없음
- `.env` 및 시크릿 변경 없음

## 실행 명령

```powershell
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development "C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload -DisableUnity
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\game\idle game\repo\client\IdleProject.uproject" -ExecCmds="Automation RunTests IdleProject.Leaderboard; Quit" -unattended -nop4 -nosplash -NullRHI -log
Push-Location server; npm run lint; npm run test -- leaderboard; Pop-Location
```
