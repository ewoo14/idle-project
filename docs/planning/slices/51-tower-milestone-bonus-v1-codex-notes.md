# PR #51 Codex character 구현 노트

## 범위
- `GameCore`에 `FTowerMilestoneFormula`를 추가하고 `MilestoneStep = 10`, `MilestoneBonusPerStep = 0.02`를 고정했다.
- `UTowerService::GetMilestoneMultiplier()`와 `UIdleGameInstance::GetTowerMilestoneMultiplier()`로 탑 마일스톤 배수를 노출했다.
- `AIdleCharacter::RefreshDerivedStats()`에서 초월 배수와 탑 마일스톤 배수를 함께 곱하되, HP/물리공격/마법공격/물리방어/마법방어에만 적용했다.

## 검증
- RED: 구현 전 `Build.bat IdleProjectEditor Win64 Development`가 `GameCore/TowerMilestoneFormula.h` 부재로 실패했다.
- GREEN: `Build.bat IdleProjectEditor Win64 Development` 종료 코드 0.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.GameCore.Tower; Quit"` 종료 코드 0, 탑 테스트 4개 성공.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject.Character.Stats; Quit"` 종료 코드 0, 캐릭터 스탯 테스트 5개 성공.
- GREEN: `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests IdleProject; Quit"` 종료 코드 0, 전체 133개 테스트 발견 및 완료.
