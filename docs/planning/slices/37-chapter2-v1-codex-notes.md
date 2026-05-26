# PR #37 Codex Character Notes

## Scope
- `UStageService` now owns chapter progression for `TotalChapters = 2`.
- Chapter 1 boss clear advances from 1-5 to 2-1 and broadcasts `OnChapterBossDefeated(1)`.
- Chapter 2 boss clear records `HighestClearedChapter = 2`, freezes progress on 2-5, and ignores further kills.
- `UIdleGameInstance` listens for `OnChapterBossDefeated(1)` and sets the rebirth gate through idempotent `MarkChapter1BossDefeated()`.
- `AIdleProjectGameModeBase::ScheduleRespawn()` records kills and quest progress only; it no longer directly marks the chapter 1 boss gate.
- `FStageFormula::GetStageWeakElement()` maps chapter 2 global indices 5-9 to `None`, `Lightning`, `Ice`, `Fire`, and `Holy`.

## Automation Coverage
- `IdleProject.GameCore.StageFormula.ScalingAndWeakElements`
- `IdleProject.GameCore.StageService.BossCompletion`
- `IdleProject.GameCore.IdleGameInstance.StageServiceHooks`

## Verification
- RED: `Build.bat IdleProjectEditor Win64 Development` failed on missing `OnChapterBossDefeated`, `HasClearedChapterBoss`, and `GetHighestClearedChapter`.
- Build: `Build.bat IdleProjectEditor Win64 Development` exited 0.
- Automation: `UnrealEditor-Cmd.exe ... Automation RunTests IdleProject` exited 0.
