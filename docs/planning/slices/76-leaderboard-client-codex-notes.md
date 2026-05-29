# PR #76 Character Codex Notes

## Scope
- Added UE leaderboard read models: `FLeaderboardEntry` and `ELeaderboardKind`.
- Added `ULeaderboardService` for top-N and `/me` response parsing/caching.
- Added `UApiClient::FetchLeaderboard` and `FetchMyRank` for
  `/v1/leaderboard/power|rebirth` and `/me` calls with `season` and `limit=100`.
- Added `UIdleGameInstance::RefreshLeaderboard` and `GetLeaderboardService`.
- Added `IdleProject.Leaderboard` automation coverage for list parsing,
  `/me` parsing, bigint score strings, and graceful invalid/offline fallback.

## Verification
- RED: UE build failed on missing `GameCore/LeaderboardService.h` after adding
  `LeaderboardTests.cpp`.
- GREEN: `Build.bat IdleProjectEditor Win64 Development -Project=client/IdleProject.uproject -WaitMutex -DisableUnity` exited 0.
- GREEN: `Automation RunTests IdleProject.Leaderboard; Quit` found 3 tests and
  logged all three as `Result={Success}`.
- GREEN: `cd server; npm run lint` checked 139 files with no fixes.

## Notes
- Normal unity build currently fails before/after this slice because
  `MasteryTests.cpp` and `DungeonServiceTests.cpp` both define
  `FIdleGameInstanceWorldContextAccessor` in anonymous namespaces inside the
  same generated unity translation unit. Non-unity build verifies this slice.
- Save payload/version remains unchanged.
