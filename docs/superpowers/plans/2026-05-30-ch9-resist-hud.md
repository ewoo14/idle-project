# 챕터 9 마감 — 저항 표기 HUD 구현 플랜

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** #107 부 저항(`StageInfo.ResistElement`)을 HUD 우상단 저항 배지로 표시하고(ch9+만), ViewModel·렌더·로컬라이즈·테스트를 추가한다.

**Architecture:** `FIdleHUDStageViewModel`에 저항 필드 추가, `BuildStageViewModel`이 `StageInfo.ResistElement`로 채움(generic element 헬퍼 재사용), DrawHUD가 `bHasResist` 가드로 약점 배지 아래 외곽선 저항 배지 렌더. 클라 전용, 세이브/서버 무변경.

**Tech Stack:** UE5.7 C++ HUD(Canvas DrawRect/DrawText), CSV 로컬라이즈, Automation 테스트.

---

## 파일 구조
- `client/Source/IdleProject/UI/IdleHUD.h` — ViewModel 저항 필드.
- `client/Source/IdleProject/UI/IdleHUD.cpp` — BuildStageViewModel 저항 설정 + DrawHUD 저항 배지.
- `client/Content/Localization/Game/ko/UI.csv`, `en/UI.csv` — STAGE_RESIST_FORMAT.
- `client/Source/IdleProject/Tests/LocalizationTests.cpp` — 키 무결성.
- `client/Source/IdleProject/Tests/StageViewModelHudTests.cpp` — 신규 ViewModel 회귀.

---

### Task 1: ViewModel 저항 필드 + BuildStageViewModel 설정

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h:223~`
- Modify: `client/Source/IdleProject/UI/IdleHUD.cpp:2237~2273`

- [ ] **Step 1: ViewModel 필드 추가**

`IdleHUD.h`의 `FIdleHUDStageViewModel`에서 `FLinearColor WeaknessColor = FLinearColor::White;` 다음에 추가:
```cpp
	FText ResistLabel;
	FText ResistIconLabel;
	FLinearColor ResistColor = FLinearColor::White;
	bool bHasResist = false;
```

- [ ] **Step 2: BuildStageViewModel 저항 설정**

`IdleHUD.cpp`의 `BuildStageViewModel`에서 `ViewModel.WeaknessColor = StageWeakElementToColor(StageInfo.WeakElement);`(2272행) 다음, `return ViewModel;` 앞에 추가:
```cpp
	ViewModel.bHasResist = StageInfo.ResistElement != ESkillElement::None;
	if (ViewModel.bHasResist)
	{
		ViewModel.ResistLabel = FormatLocalizedUI(TEXT("STAGE_RESIST_FORMAT"), [&StageInfo](FFormatNamedArguments& Args)
		{
			Args.Add(TEXT("Element"), StageWeakElementToLabel(StageInfo.ResistElement));
		});
		ViewModel.ResistIconLabel = StageWeakElementToIconLabel(StageInfo.ResistElement);
		ViewModel.ResistColor = StageWeakElementToColor(StageInfo.ResistElement);
	}
```

- [ ] **Step 3: 에디터 빌드**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공.

- [ ] **Step 4: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/UI/IdleHUD.h client/Source/IdleProject/UI/IdleHUD.cpp
git commit -m @'
챕터 9 마감(1) — HUD ViewModel 저항 필드 + BuildStageViewModel 설정

FIdleHUDStageViewModel에 ResistLabel/IconLabel/Color/bHasResist, BuildStageViewModel
이 StageInfo.ResistElement로 채움(generic 헬퍼 재사용). ResistElement None이면 미설정.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 2: DrawHUD 저항 배지 렌더

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.cpp:5559~5564` 영역

- [ ] **Step 1: 약점 배지 다음에 저항 배지 추가**

`IdleHUD.cpp`의 약점 라벨 렌더(`DrawText(ViewModel.WeaknessLabel.ToString(), ...)`, 5564행) 다음, 보스/엘리트 배지 `if` 앞에 추가:
```cpp
	if (ViewModel.bHasResist)
	{
		const float ResistIconY = WeaknessIconY + 24.0f * Scale;
		const float Edge = 1.5f * Scale;
		DrawRect(ViewModel.ResistColor.CopyWithNewOpacity(0.30f), WeaknessIconX, ResistIconY, WeaknessIconSize, WeaknessIconSize);
		DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY, WeaknessIconSize, Edge);
		DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY + WeaknessIconSize - Edge, WeaknessIconSize, Edge);
		DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY, Edge, WeaknessIconSize);
		DrawRect(ViewModel.ResistColor, WeaknessIconX + WeaknessIconSize - Edge, ResistIconY, Edge, WeaknessIconSize);
		DrawText(ViewModel.ResistIconLabel.ToString(), ViewModel.ResistColor, WeaknessIconX + 7.0f * Scale, ResistIconY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
		DrawText(ViewModel.ResistLabel.ToString(), ViewModel.ResistColor, X + PanelWidth - 128.0f * Scale, ResistIconY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
	}
```
> 보스/엘리트 배지는 `Y + 42.0f * Scale`(BadgeY)에 그려진다. 저항 배지는 `WeaknessIconY(=Y+12) + 24 = Y+36`에서 시작해 우측 아이콘 열(WeaknessIconX)에 위치 — 보스 배지(우측 패딩 정렬)와 X가 다르거나 겹치면 ResistIconY/X 미세조정. 패널 높이가 부족하면 PanelHeight도 확인(겹침 시 저항 배지를 약점 라벨과 같은 행 우측으로 이동).

- [ ] **Step 2: 에디터 빌드 + 시각 위치 점검**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공. (렌더 좌표가 보스/엘리트 배지와 겹치지 않는지 코드상 확인 — BadgeY=Y+42, ResistIconY=Y+36, ResistIconX=우측 아이콘 열로 보스 배지보다 위·우측. 겹치면 조정.)

- [ ] **Step 3: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/UI/IdleHUD.cpp
git commit -m @'
챕터 9 마감(2) — HUD 저항 배지 렌더(약점 아래 외곽선 배지)

bHasResist 가드로 ch9+에서만 약점 배지 아래 외곽선+디밍 저항 배지 + 아이콘/라벨.
약점=채움/저항=외곽선으로 시각 구분. 이전 챕터 HUD 불변.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 3: 로컬라이즈 STAGE_RESIST_FORMAT

**Files:**
- Modify: `client/Content/Localization/Game/ko/UI.csv`, `en/UI.csv`
- Modify: `client/Source/IdleProject/Tests/LocalizationTests.cpp`

- [ ] **Step 1: UI.csv ko/en에 키 추가**

`ko/UI.csv`의 `STAGE_WEAKNESS_FORMAT` 행을 찾아 그 다음 행에 추가(기존 CSV 포맷/이스케이프 동일):
```
STAGE_RESIST_FORMAT,"저항: {Element}"
```
`en/UI.csv`에 동일 키:
```
STAGE_RESIST_FORMAT,"Resist: {Element}"
```
(실제 CSV 컬럼 구조 — Key,Text 또는 Key,Source 등 — 을 `STAGE_WEAKNESS_FORMAT` 행을 보고 정확히 맞춰라. `{Element}` 플레이스홀더 유지.)

- [ ] **Step 2: LocalizationTests에 키 단언**

`LocalizationTests.cpp`에서 `STAGE_WEAKNESS_FORMAT` 단언을 찾아 동일 패턴으로 `STAGE_RESIST_FORMAT` ko·en 존재/orphan 0 단언 추가.

- [ ] **Step 3: 빌드 + 로컬라이즈 게이트**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.Localization"
```
Expected: `[ue-automation] GREEN.`(STAGE_RESIST_FORMAT ko·en 무결성).

- [ ] **Step 4: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Content/Localization/Game/ko/UI.csv client/Content/Localization/Game/en/UI.csv client/Source/IdleProject/Tests/LocalizationTests.cpp
git commit -m @'
챕터 9 마감(3) — STAGE_RESIST_FORMAT ko·en 로컬라이즈

저항 배지 라벨 포맷 ko("저항: {Element}")·en("Resist: {Element}") + LocalizationTests
무결성 단언.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 4: HUD ViewModel 회귀 테스트

**Files:**
- Create: `client/Source/IdleProject/Tests/StageViewModelHudTests.cpp`

- [ ] **Step 1: 신규 테스트 작성**

`StageViewModelHudTests.cpp`:
```cpp
#include "Misc/AutomationTest.h"
#include "UI/IdleHUD.h"
#include "GameCore/StageService.h"
#include "GameCore/SkillTypes.h" // ESkillElement (실제 enum 헤더 경로 확인 후 맞춤)

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStageViewModelResistTest,
	"IdleProject.UI.HUD.StageViewModelResist",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageViewModelResistTest::RunTest(const FString& Parameters)
{
	// ch9 스테이지: 저항 설정 → bHasResist + 색 일치.
	FStageInfo Ch9;
	Ch9.WeakElement = ESkillElement::Dark;
	Ch9.ResistElement = ESkillElement::Ice;
	const FIdleHUDStageViewModel VmCh9 = IdleProject::UI::BuildStageViewModel(Ch9);
	TestTrue(TEXT("ch9 has resist"), VmCh9.bHasResist);
	TestFalse(TEXT("ch9 resist label non-empty"), VmCh9.ResistLabel.IsEmpty());
	TestFalse(TEXT("ch9 resist icon non-empty"), VmCh9.ResistIconLabel.IsEmpty());

	// 이전 챕터: 저항 None → bHasResist false.
	FStageInfo Early;
	Early.WeakElement = ESkillElement::Fire;
	Early.ResistElement = ESkillElement::None;
	const FIdleHUDStageViewModel VmEarly = IdleProject::UI::BuildStageViewModel(Early);
	TestFalse(TEXT("early no resist"), VmEarly.bHasResist);
	return true;
}

#endif
```
> `ESkillElement` 실제 헤더 경로와 `FStageInfo` 필드 접근을 코드에서 확인해 include/필드를 맞춰라. 색 일치 단언(`VmCh9.ResistColor == StageWeakElementToColor(Ice)`)은 `StageWeakElementToColor`가 IdleHUD.cpp 내부 정적이라 테스트에서 직접 못 부르면 생략하고 bHasResist/라벨로 충분.

- [ ] **Step 2: HUD 게이트**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.UI.HUD"
```
Expected: `[ue-automation] GREEN.`(StageViewModelResist 포함).

- [ ] **Step 3: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Tests/StageViewModelHudTests.cpp
git commit -m @'
챕터 9 마감(4) — HUD ViewModel 저항 회귀 테스트

BuildStageViewModel 저항 설정(ch9 ResistElement→bHasResist/라벨) + 이전 챕터
None→bHasResist false 단언.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 5: 전체 게이트 + 무변경 확인 (PM)

**Files:** (검증)

- [ ] **Step 1: 전체 Automation + jumbo**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1
```
Expected: `[ue-automation] GREEN.`

- [ ] **Step 2: 세이브/서버 무변경**

Run:
```powershell
cd "C:\game\idle game\repo"; git diff origin/main --stat -- server/ client/Source/IdleProject/GameCore/IdleSaveGame.h
```
Expected: 출력 없음.

- [ ] **Step 3: PR 발행**

`superpowers:finishing-a-development-branch` 옵션 2(push + PR). 본문에 클라 전용·ch9+만 표시·세이브 무변경 명시.

---

## Self-Review

- **스펙 커버리지:** ViewModel 필드(§3)=Task 1 / 렌더(§4)=Task 2 / 로컬라이즈(§5)=Task 3 / 테스트(§6)=Task 4 / 무변경(§7)=Task 5 Step 2. 전부 매핑.
- **플레이스홀더:** ViewModel/빌드/렌더 코드 구체. CSV 포맷·ESkillElement 헤더는 기존 행/코드 확인 지시(불가피한 환경 의존).
- **타입 일관성:** `ResistLabel`/`ResistIconLabel`/`ResistColor`/`bHasResist`(IdleHUD.h) ↔ BuildStageViewModel 설정(Task1) ↔ 렌더(Task2) ↔ 테스트(Task4) 일치. `STAGE_RESIST_FORMAT` 키가 CSV(Task3)↔빌드(Task1) 동일.
- **회귀 가드:** `bHasResist` 가드로 이전 챕터(ResistElement None) HUD 불변 — Task4가 명시 단언.
