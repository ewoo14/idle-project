# 챕터 9 마감 — 저항 표기 HUD + 속성 UX 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(클릭) → plan 단계화
- 분류: 프론트엔드(UE5 HUD). **클라 전용 — 서버/세이브/parity 무관.**
- 선행: 챕터 9 + 속성 공명(#107, 부 저항 데이터·전투 도입). 브랜치 `feat/ch9-resist-hud`.

## 1. 배경

#107이 ch9 스테이지(81~90)에 **부 저항 속성**(`StageInfo.ResistElement`, `AIdleMonster::ResistElement`)과 공명 데미지를 도입했으나, **HUD에 저항이 표시되지 않는다**. 플레이어는 약점만 보고(우상단 약점 배지) 저항을 알 수 없어, 공명 기믹의 전술적 선택(저항 속성 회피)이 가려져 있다.

현 HUD(`IdleHUD.cpp`):
- 헬퍼 `StageWeakElementToLabel/IconLabel/Color(ESkillElement)` — **임의 Element를 받는 generic**(저항 재사용 가능).
- ViewModel(`IdleHUD.h:230~233`) `WeaknessLabel`/`WeaknessIconLabel`/`WeaknessColor` ← `StageInfo.WeakElement`.
- 렌더(`IdleHUD.cpp:5559~5564`): 우상단 약점 색 사각 + 아이콘 글자 + 라벨.
- 색 팔레트: `StageWeakElementToColor`가 속성→테마색 단일 매핑(Fire=AccentRed/Ice=AccentBlue/Lightning=Warn/Holy=AccentGold/Dark=ElementDark/None=TextMuted).

## 2. 목표 / 비목표

### 목표
1. HUD에 **저항 배지** 표시(약점 배지와 시각 구분, `ResistElement != None`일 때만 — ch9+).
2. ViewModel에 저항 필드(`ResistLabel`/`ResistIconLabel`/`ResistColor`/`bHasResist`) ← `StageInfo.ResistElement`.
3. `STAGE_RESIST_FORMAT` 로컬라이즈 ko·en.
4. 속성 색 체계 단일 출처(`StageWeakElementToColor`) 약점·저항 공용 확정.
5. HUD ViewModel 회귀(저항 필드 채워짐 / 이전 챕터 None → `bHasResist=false`).

### 비목표
- 서버/세이브/parity/전투 로직 변경(#107에서 완료, 표시만).
- 속성 범례/툴팁/스킬 속성 표시(후속).
- 저항 외 새 HUD 요소.
- 신규 속성 색(기존 팔레트 재사용).

## 3. ViewModel 변경 (`IdleHUD.h`)
`WeaknessColor` 다음에 추가:
```cpp
FText ResistLabel;
FText ResistIconLabel;
FLinearColor ResistColor = FLinearColor::White;
bool bHasResist = false;
```

빌드 함수(`IdleHUD.cpp:2257~2272` 영역, WeaknessColor 설정 다음):
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
(generic 헬퍼 재사용 — 약점/저항 동일 색 팔레트.)

## 4. 렌더 (`IdleHUD.cpp` 약점 배지 블록 다음)
약점 배지 행 아래에 저항 배지(약점과 같은 X 정렬, Y만 한 행 아래). **시각 구분**: 약점=채워진 사각(기존), 저항=동일 속성색 **외곽선 사각(내부 디밍)** + 아이콘 글자, 그리고 라벨. "저항"은 약화 의미이므로 약점보다 낮은 불투명도/외곽선 스타일.
```cpp
if (ViewModel.bHasResist)
{
    const float ResistIconY = WeaknessIconY + 24.0f * Scale;
    // 외곽선(저항=방어적 시각) + 내부 디밍
    DrawRect(ViewModel.ResistColor.CopyWithNewOpacity(0.30f), WeaknessIconX, ResistIconY, WeaknessIconSize, WeaknessIconSize);
    // 외곽 4변
    DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY, WeaknessIconSize, 1.5f * Scale);
    DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY + WeaknessIconSize - 1.5f * Scale, WeaknessIconSize, 1.5f * Scale);
    DrawRect(ViewModel.ResistColor, WeaknessIconX, ResistIconY, 1.5f * Scale, WeaknessIconSize);
    DrawRect(ViewModel.ResistColor, WeaknessIconX + WeaknessIconSize - 1.5f * Scale, ResistIconY, 1.5f * Scale, WeaknessIconSize);
    DrawText(ViewModel.ResistIconLabel.ToString(), ViewModel.ResistColor, WeaknessIconX + 7.0f * Scale, ResistIconY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.72f * Scale);
    DrawText(ViewModel.ResistLabel.ToString(), ViewModel.ResistColor, X + PanelWidth - 128.0f * Scale, ResistIconY + 3.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.78f * Scale);
}
```
(픽셀 좌표는 plan에서 패널 레이아웃 보아 미세조정 — 보스/엘리트 배지와 겹치지 않게.)

## 5. 로컬라이즈
`UI` 로컬라이즈 테이블(ko·en)에 `STAGE_RESIST_FORMAT` 추가:
- ko: `저항: {Element}`
- en: `Resist: {Element}`
(기존 `STAGE_WEAKNESS_FORMAT`와 동일 형식. `ELEMENT_*` 라벨은 기존 재사용.)

## 6. 테스트 / 게이트
- HUD ViewModel 테스트(기존 stage badge viewmodel 빌드 테스트에 추가): ch9 스테이지(ResistElement 설정) → `bHasResist==true` + `ResistLabel/IconLabel` 비어있지 않음 + `ResistColor == StageWeakElementToColor(resist)`. 이전 챕터(ResistElement None) → `bHasResist==false`.
- 약점 표시 회귀 불변(기존 Weakness 단언).
- 표준 jumbo + 전체 Automation(`IdleProject.UI.HUD` 포함) + `LocalizationTests`(STAGE_RESIST_FORMAT ko·en orphan 0). **세이브·서버 무변경**(SaveVer 29).

## 7. 안전 가드
- `bHasResist` 가드로 저항 배지는 ch9+에서만 — 이전 챕터 HUD 시각 불변(회귀 0).
- generic 헬퍼 재사용 → 속성 색 단일 출처(약점/저항 일관).
- 클라 전용 — 세이브/서버/전투 무변경(#107 데이터 그대로 소비).
- 렌더 좌표는 기존 배지(보스/엘리트)와 비충돌 확인.

## 8. 구현 단계화 (plan)
1. `IdleHUD.h` ViewModel 저항 필드 + `IdleHUD.cpp` 빌드 함수 저항 설정.
2. `IdleHUD.cpp` 렌더 저항 배지(bHasResist 가드).
3. 로컬라이즈 `STAGE_RESIST_FORMAT` ko·en + LocalizationTests.
4. HUD ViewModel 회귀 테스트.
5. 게이트: 표준 jumbo + Automation(UI.HUD/Localization), 세이브·서버 무변경.

## 9. 후속
- 속성 상성 범례/툴팁, 스킬 속성 표시, 저항 배지 애니메이션(공명 강조).
