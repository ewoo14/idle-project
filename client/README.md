# client/ — Unreal Engine 5 클라이언트

> 본 디렉터리는 **PR #2 (M1 클라이언트 코어 부트)** 시점에 UE5 5.4 프로젝트로 스캐폴드됩니다. 현재는 자리만 잡고 있습니다.

## 예정 구조

```
client/
├── IdleProject.uproject
├── Source/
│   └── IdleProject/
│       ├── GameCore/
│       ├── CombatSystem/
│       ├── ItemSystem/
│       ├── CharacterSystem/
│       ├── QuestSystem/
│       ├── SaveSystem/
│       ├── NetworkClient/
│       ├── UI/
│       └── DataAssets/
├── Content/
│   ├── Art/
│   ├── UI/
│   ├── Data/
│   ├── Localization/
│   └── Audio/
├── Config/
└── Plugins/
```

## 빌드 (PR #2 이후)

```powershell
# Editor
"C:\Program Files\Epic Games\UE_5.4\Engine\Binaries\Win64\UnrealEditor.exe" `
  "C:\game\idle game\repo\client\IdleProject.uproject"

# 패키지
"C:\Program Files\Epic Games\UE_5.4\Engine\Build\BatchFiles\RunUAT.bat" `
  BuildCookRun -project="C:\game\idle game\repo\client\IdleProject.uproject" `
  -platform=Win64 -clientconfig=Development -build -cook -stage -package
```

## 코드 정책

- 로직 C++, 표현 Blueprint (자세히는 `docs/planning/02-architecture.md §2.2`).
- 모든 신규 모듈은 `Source/IdleProject/<Module>/` 하위.
- 데이터는 DataTable / CSV 로.

## 라이선스

UE5 코드는 Epic Games EULA 적용. 본 프로젝트의 자체 코드는 MIT (저장소 루트 LICENSE).
