# IdleProject UE5 Client

UE 5.7 기반 클라이언트 코어 부트 프로젝트입니다. 로직은 C++, 표현은 Blueprint/UMG에서 확장하는 구조를 따릅니다.

## 열기

```powershell
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\game\idle game\repo\client\IdleProject.uproject"
```

## 빌드

```powershell
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" `
  IdleProjectEditor Win64 Development `
  -Project="C:\game\idle game\repo\client\IdleProject.uproject" `
  -WaitMutex
```

## Automation 테스트

```powershell
"C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "C:\game\idle game\repo\client\IdleProject.uproject" `
  -ExecCmds="Automation RunTests IdleProject; Quit" `
  -unattended -nop4 -nosplash
```

## 현재 범위

- `AIdleCharacter`: 사이드뷰 카메라, 큐브 placeholder, EnhancedInput 이동/공격/메뉴 토글
- `FStatFormulas`: `server/src/core/formulas/stats.ts` 미러
- `FLevelFormulas`: PR #4 명시 레벨 곡선 앵커값 기반 미러
- `ClassDB.csv`, `LevelCurveDB.csv`: DataTable 임포트용 CSV
- `UMainMenuController`: 추후 `W_MainMenu` Blueprint의 C++ 베이스
- `UApiClient`: PR #5 인증 연동 전 HTTP GET/POST 골격

## UI 테마 토큰

`docs/planning/ui-tokens.json`이 UI 색상의 source of truth이며, UE5 C++ 미러는 `Source/IdleProject/UI/UIThemeTokens.h`에 둡니다. `UMainMenuController`는 `BackgroundColor`, `PanelColor`, `PrimaryTextColor`, `PrimaryActionColor` 등을 `EditDefaultsOnly`로 노출하므로 `W_MainMenu` Blueprint에서 같은 토큰을 바로 바인딩합니다.

## 알려진 한계

Blueprint `.uasset`와 맵 에셋은 Codex가 직접 생성하지 않았습니다. PM이 UE 에디터에서 `W_MainMenu`, `MainMenu`, `Game` 맵을 생성해 C++ 베이스에 연결해야 합니다.
