# 속성 UX 후속 — 스킬 속성 배지 + 상성 범례 구현 플랜

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 각 액티브 스킬 슬롯에 속성 배지를 표시하고, 5속성 상성 범례 패널(약점 +50%/저항 −50%)을 추가한다.

**Architecture:** `FIdleHUDSkillSlotViewModel`에 `Element` 추가(빌더가 `Skill.Element`로 채움) + 신규 범례 ViewModel/빌더. `DrawSkillSlot`이 속성 배지, 신규 `DrawElementLegend`가 범례 패널 렌더. generic 속성 헬퍼 재사용. 클라 전용, 세이브/서버 무변경.

**Tech Stack:** UE5.7 C++ HUD(Canvas), CSV 로컬라이즈, Automation 테스트.

---

## 파일 구조
- `client/Source/IdleProject/UI/IdleHUD.h` — 스킬 슬롯 Element + 범례 ViewModel 구조체/선언.
- `client/Source/IdleProject/UI/IdleHUD.cpp` — 빌더(Element 설정 + BuildElementLegendViewModel) + 렌더(배지 + DrawElementLegend).
- `client/Content/Localization/Game/ko/UI.csv`, `en/UI.csv` — ELEMENT_LEGEND_*.
- `client/Source/IdleProject/Tests/CombatTests.cpp` — 스킬 Element + 범례 회귀.
- `client/Source/IdleProject/Tests/LocalizationTests.cpp` — 키 무결성.

---

### Task 1: ViewModel + 빌더 (스킬 Element + 범례)

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.h`
- Modify: `client/Source/IdleProject/UI/IdleHUD.cpp:2212~`, 신규 `BuildElementLegendViewModel`

- [ ] **Step 1: 스킬 슬롯 ViewModel에 Element 추가**

`IdleHUD.h`의 `FIdleHUDSkillSlotViewModel`(38행)에서 `bool bCanRankUp = false;` 다음에 추가:
```cpp
	ESkillElement Element = ESkillElement::None;
```
(`ESkillElement`가 IdleHUD.h에서 미가시면 상단 include에 `#include "CombatSystem/StatusElementTypes.h"` 추가. 이미 StageViewModel이 ESkillElement 사용하므로 보통 가시.)

- [ ] **Step 2: 범례 ViewModel 구조체 추가**

`IdleHUD.h`의 `FIdleHUDSkillSlotViewModel` 구조체 다음에 추가:
```cpp
struct IDLEPROJECT_API FIdleHUDElementLegendEntry
{
	FText Label;
	FText IconLabel;
	FLinearColor Color = FLinearColor::White;
};

struct IDLEPROJECT_API FIdleHUDElementLegendViewModel
{
	TArray<FIdleHUDElementLegendEntry> Elements;
	FText WeakNote;
	FText ResistNote;
};
```
그리고 `BuildSkillSlotViewModels` 선언(1046행) 다음에:
```cpp
IDLEPROJECT_API FIdleHUDElementLegendViewModel BuildElementLegendViewModel();
```

- [ ] **Step 3: 빌더 — 스킬 Element 설정**

`IdleHUD.cpp`의 `BuildSkillSlotViewModels`(2202)에서 `Slot.bCanRankUp = SkillComponent.CanRankUp(Skill.SkillId);`(2221) 다음에 추가:
```cpp
		Slot.Element = Skill.Element;
```

- [ ] **Step 4: 빌더 — BuildElementLegendViewModel**

`BuildSkillSlotViewModels` 함수 다음에 추가(`StageWeakElementToLabel/IconLabel/Color`는 같은 파일 내 file-local 함수라 직접 호출 가능):
```cpp
FIdleHUDElementLegendViewModel IdleProject::UI::BuildElementLegendViewModel()
{
	FIdleHUDElementLegendViewModel Legend;
	const ESkillElement Order[] = { ESkillElement::Fire, ESkillElement::Ice, ESkillElement::Lightning, ESkillElement::Holy, ESkillElement::Dark };
	for (ESkillElement E : Order)
	{
		FIdleHUDElementLegendEntry Entry;
		Entry.Label = StageWeakElementToLabel(E);
		Entry.IconLabel = StageWeakElementToIconLabel(E);
		Entry.Color = StageWeakElementToColor(E);
		Legend.Elements.Add(Entry);
	}
	Legend.WeakNote = IdleProject::Localization::UI(TEXT("ELEMENT_LEGEND_WEAK"));
	Legend.ResistNote = IdleProject::Localization::UI(TEXT("ELEMENT_LEGEND_RESIST"));
	return Legend;
}
```
> 주: `BuildElementLegendViewModel`가 `StageWeakElement*` 함수보다 파일에서 **뒤에** 정의돼야 한다(1388~1448에 정의됨, 2226 이후 추가이므로 OK). 호출부도 마찬가지.

- [ ] **Step 5: 에디터 빌드**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공.

- [ ] **Step 6: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/UI/IdleHUD.h client/Source/IdleProject/UI/IdleHUD.cpp
git commit -m @'
속성 UX(1) — 스킬 슬롯 Element + 범례 ViewModel/빌더

FIdleHUDSkillSlotViewModel.Element(Skill.Element), 신규 범례 ViewModel 구조체 +
BuildElementLegendViewModel(5속성 색/라벨/아이콘 + 약점/저항 노트). generic 헬퍼 재사용.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 2: 렌더 (스킬 배지 + 범례 패널)

**Files:**
- Modify: `client/Source/IdleProject/UI/IdleHUD.cpp` (`DrawSkillSlot`, `DrawSkillHud`), `IdleHUD.h`(DrawElementLegend 선언)

- [ ] **Step 1: DrawSkillSlot에 속성 배지**

`DrawSkillSlot`(7450)의 랭크업 버튼 블록(7494~7502) 다음, 함수 끝 `}` 앞에 추가:
```cpp
	if (Slot.Element != ESkillElement::None)
	{
		const float BadgeSize = 16.0f * Scale;
		const float BadgeX = X + 28.0f * Scale;
		const float BadgeY = Y + Height - BadgeSize - 7.0f * Scale;
		DrawRect(StageWeakElementToColor(Slot.Element).CopyWithNewOpacity(0.92f), BadgeX, BadgeY, BadgeSize, BadgeSize);
		DrawText(StageWeakElementToIconLabel(Slot.Element).ToString(), Theme::BgPrimary, BadgeX + 4.0f * Scale, BadgeY + 1.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
	}
```
> 좌표: 쿨다운 바는 `Y+Height−12`(높이 5), 배지는 `X+28`(이름 라벨 아래 좌측), `Y+Height−23`로 쿨다운 바와 세로로 분리. 기존 요소(랭크업 버튼 우상단, 상태 라벨 Y+24)와 비충돌. 겹치면 BadgeX/Y 미세조정.

- [ ] **Step 2: DrawElementLegend 선언 + 정의**

`IdleHUD.h`의 `DrawSkillSlot` 선언(1177) 인근(private 렌더 메서드)에 추가:
```cpp
	void DrawElementLegend(const FIdleHUDElementLegendViewModel& Legend, float X, float Y, float Width);
```
`IdleHUD.cpp`의 `DrawUltimateGauge` 정의 앞(또는 DrawSkillSlot 다음)에 정의:
```cpp
void AIdleHUD::DrawElementLegend(const FIdleHUDElementLegendViewModel& Legend, float X, float Y, float Width)
{
	using namespace IdleProject::UI;
	if (!Canvas || Legend.Elements.Num() <= 0)
	{
		return;
	}
	const float Scale = FMath::Clamp(Canvas->SizeY / 1080.0f, 1.0f, 2.0f);
	const float PanelH = 40.0f * Scale;
	DrawRect(Theme::BgPanel.CopyWithNewOpacity(0.86f), X, Y, Width, PanelH);
	DrawText(IdleProject::Localization::UI(TEXT("ELEMENT_LEGEND_TITLE")).ToString(), Theme::TextMuted, X + 8.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.74f * Scale);

	float CursorX = X + 8.0f * Scale;
	const float SwatchY = Y + 20.0f * Scale;
	const float SwatchSize = 12.0f * Scale;
	for (const FIdleHUDElementLegendEntry& Entry : Legend.Elements)
	{
		DrawRect(Entry.Color.CopyWithNewOpacity(0.92f), CursorX, SwatchY, SwatchSize, SwatchSize);
		DrawText(Entry.Label.ToString(), Theme::TextPrimary, CursorX + SwatchSize + 3.0f * Scale, SwatchY - 1.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.68f * Scale);
		CursorX += SwatchSize + 52.0f * Scale;
	}
	const FString Notes = Legend.WeakNote.ToString() + TEXT("  /  ") + Legend.ResistNote.ToString();
	DrawText(Notes, Theme::TextMuted, X + Width - 150.0f * Scale, Y + 4.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.70f * Scale);
}
```
> 좌표/폭은 plan 구현 시 화면폭 대비 조정. 패널은 기존 패널과 비충돌 위치(아래 Step 3에서 배치).

- [ ] **Step 3: DrawSkillHud에서 범례 호출**

`DrawSkillHud`(7407)의 스킬 슬롯 루프(7431~7434) 다음, `DrawUltimateGauge` 호출 전/후 적절히, 범례를 스킬 바 위쪽에 배치:
```cpp
	DrawElementLegend(
		IdleProject::UI::BuildElementLegendViewModel(),
		StartX,
		SlotY - 100.0f * HudScale,
		TotalWidth);
```
> `SlotY − 100`은 궁극기 게이지(SlotY−34)·스킬 포인트 라벨(SlotY−62)보다 위. 겹치면 Y 오프셋 조정.

- [ ] **Step 4: 에디터 빌드 + 비충돌 확인**

Run:
```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" IdleProjectEditor Win64 Development -Project="C:\game\idle game\repo\client\IdleProject.uproject" -WaitMutex -NoHotReload
```
Expected: 빌드 성공(C4458 0). 범례/배지 좌표가 기존 HUD 요소와 코드상 비충돌 확인.

- [ ] **Step 5: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/UI/IdleHUD.h client/Source/IdleProject/UI/IdleHUD.cpp
git commit -m @'
속성 UX(2) — 스킬 속성 배지 렌더 + 상성 범례 패널

DrawSkillSlot에 속성 색+아이콘 배지(Element!=None), 신규 DrawElementLegend(5속성
스와치+라벨 + 약점/저항 노트) DrawSkillHud에서 호출. 기존 HUD 요소와 비충돌 배치.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 3: 로컬라이즈 ELEMENT_LEGEND_*

**Files:**
- Modify: `client/Content/Localization/Game/ko/UI.csv`, `en/UI.csv`
- Modify: `client/Source/IdleProject/Tests/LocalizationTests.cpp`

- [ ] **Step 1: UI.csv ko/en 키 추가**

`ko/UI.csv`의 `ELEMENT_DARK` 행(208) 다음에 추가(4컬럼 `Namespace,Key,"Text","Comment"` 포맷 준수):
```
UI,ELEMENT_LEGEND_TITLE,"속성 상성","범례"
UI,ELEMENT_LEGEND_WEAK,"약점 +50%","범례"
UI,ELEMENT_LEGEND_RESIST,"저항 -50%","범례"
```
`en/UI.csv`의 대응 위치에:
```
UI,ELEMENT_LEGEND_TITLE,"Element Affinity","Legend"
UI,ELEMENT_LEGEND_WEAK,"Weakness +50%","Legend"
UI,ELEMENT_LEGEND_RESIST,"Resist -50%","Legend"
```
(en CSV의 실제 컬럼 구조를 `ELEMENT_DARK` 행 보고 정확히 맞춰라.)

- [ ] **Step 2: LocalizationTests 키 단언**

`LocalizationTests.cpp`에서 `STAGE_RESIST_FORMAT`(또는 ELEMENT_* 키) 단언 패턴을 찾아 `ELEMENT_LEGEND_TITLE/WEAK/RESIST` ko·en 존재 단언 추가(`RequiredCloudSyncKeys` 배열 또는 해당 키 무결성 루프).

- [ ] **Step 3: 로컬라이즈 게이트**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.Localization"
```
Expected: `[ue-automation] GREEN.`(ELEMENT_LEGEND_* ko·en orphan 0).

- [ ] **Step 4: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Content/Localization/Game/ko/UI.csv client/Content/Localization/Game/en/UI.csv client/Source/IdleProject/Tests/LocalizationTests.cpp
git commit -m @'
속성 UX(3) — ELEMENT_LEGEND_* ko·en 로컬라이즈

상성 범례 제목/약점/저항 노트 ko·en + LocalizationTests 무결성.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 4: 회귀 테스트 (스킬 Element + 범례 ViewModel)

**Files:**
- Modify: `client/Source/IdleProject/Tests/CombatTests.cpp` (기존 스킬 슬롯 viewmodel 테스트 위치)

- [ ] **Step 1: 범례 ViewModel 테스트 추가**

`CombatTests.cpp`에 신규 테스트(또는 기존 스킬 슬롯 테스트 인근) 추가:
```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FElementLegendViewModelTest,
	"IdleProject.UI.HUD.ElementLegend",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FElementLegendViewModelTest::RunTest(const FString& Parameters)
{
	const FIdleHUDElementLegendViewModel Legend = IdleProject::UI::BuildElementLegendViewModel();
	TestEqual(TEXT("5 elements"), Legend.Elements.Num(), 5);
	for (const FIdleHUDElementLegendEntry& Entry : Legend.Elements)
	{
		TestFalse(TEXT("element label non-empty"), Entry.Label.IsEmpty());
		TestFalse(TEXT("element icon non-empty"), Entry.IconLabel.IsEmpty());
	}
	TestFalse(TEXT("weak note non-empty"), Legend.WeakNote.IsEmpty());
	TestFalse(TEXT("resist note non-empty"), Legend.ResistNote.IsEmpty());
	return true;
}
```
(include에 `UI/IdleHUD.h` 필요 시 추가 — CombatTests가 이미 BuildSkillSlotViewModels 테스트하므로 보통 존재.)

- [ ] **Step 2: 스킬 슬롯 Element 단언(기존 테스트 확장)**

CombatTests의 기존 `BuildSkillSlotViewModels` 테스트에서, 액티브 스킬에 Element를 설정한 SkillComponent로 빌드 후 `Slot.Element == 설정값` 단언 추가(기존 테스트가 SkillComponent를 구성하는 방식을 따라). 기존 테스트가 Element 미설정이면, 최소한 빌드된 슬롯의 `Element`가 소스 스킬과 일치함을 단언.

- [ ] **Step 3: HUD/Combat 게이트**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.UI.HUD+IdleProject.Combat"
```
Expected: `[ue-automation] GREEN.`(ElementLegend + 스킬 슬롯 회귀 포함).

- [ ] **Step 4: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/Tests/CombatTests.cpp
git commit -m @'
속성 UX(4) — 스킬 Element + 상성 범례 회귀

ElementLegend ViewModel(5속성 + 약점/저항 노트 비어있지 않음) + 스킬 슬롯
Element 일치 단언.

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

`superpowers:finishing-a-development-branch` 옵션 2(push + PR). 본문에 클라 전용·세이브 무변경·generic 헬퍼 재사용 명시.

---

## Self-Review

- **스펙 커버리지:** 스킬 배지(§3)=Task 1 Step1·3 + Task 2 Step1 / 범례(§4)=Task 1 Step2·4 + Task 2 Step2·3 / 로컬라이즈(§5)=Task 3 / 테스트(§6)=Task 4 / 무변경(§7)=Task 5 Step2. 전부 매핑.
- **플레이스홀더:** ViewModel/빌더/렌더 코드 구체. CSV 컬럼·LocalizationTests 단언 위치는 기존 행/패턴 확인 지시(환경 의존).
- **타입 일관성:** `FIdleHUDElementLegendEntry`/`FIdleHUDElementLegendViewModel`/`BuildElementLegendViewModel`/`DrawElementLegend` 명명이 헤더(Task1·2)↔빌더↔렌더↔테스트(Task4) 일치. 스킬 `Element` 필드가 ViewModel(Task1)↔빌더(Task1)↔렌더(Task2)↔테스트(Task4) 일치. `ELEMENT_LEGEND_*` 키가 CSV(Task3)↔빌더(Task1)↔렌더(Task2) 동일.
- **함수 순서:** `BuildElementLegendViewModel`/`DrawElementLegend`가 file-local `StageWeakElement*`(1388~1448) 뒤에 정의되도록(2226 이후 / DrawSkillSlot 이후) 배치 — 컴파일 가시성 확보.
