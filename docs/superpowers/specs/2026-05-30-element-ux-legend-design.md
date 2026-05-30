# 속성 UX 후속 — 스킬 속성 배지 + 상성 범례 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(사용자 "1번" 선택) → plan 단계화
- 분류: 프론트엔드(UE5 HUD). **클라 전용 — 서버/세이브/parity 무관.**
- 선행: 챕터 9 공명(#107) + 저항 표기 HUD(#108). 브랜치 `feat/element-ux-legend`.

## 1. 배경

#107(공명: 약점 +50% / 저항 −50%)·#108(저항 배지)로 **몬스터의** 약점/저항은 HUD에 표시됐다. 그러나 ① **플레이어 스킬의 속성**이 HUD에 안 보여, 어떤 스킬이 어떤 속성인지 알 수 없어 공명 전술(약점 속성 스킬 선택, 저항 속성 회피)을 활용하기 어렵다. ② **속성 상성 규칙**(왜 1.5/0.5인지) 안내가 없다.

현 HUD:
- 스킬 슬롯 ViewModel `FIdleHUDSkillSlotViewModel`(IdleHUD.h:38) ← `BuildSkillSlotViewModels`(IdleHUD.cpp:2202) ← `Skill.Element`(SkillComponent.h:77, `ESkillElement`). 렌더 `DrawSkillSlot`(IdleHUD.cpp:7450). **현재 Element 미사용.**
- generic 속성 헬퍼 `StageWeakElementToLabel/IconLabel/Color`(IdleHUD.cpp:1388~1448) 재사용 가능.
- 로컬라이즈 `ELEMENT_NONE/FIRE/ICE/LIGHTNING/HOLY/DARK` ko·en 존재(UI.csv).

## 2. 목표 / 비목표

### 목표
1. **스킬 속성 배지**: 각 액티브 스킬 슬롯에 속성 색+아이콘 배지(`Element != None`일 때).
2. **상성 범례**: 5속성 색/라벨 + 약점(+50%)/저항(−50%) 의미 컴팩트 범례 패널(상시).
3. 로컬라이즈 `ELEMENT_LEGEND_*` ko·en.
4. 회귀: 스킬 슬롯 Element 채워짐 + 범례 ViewModel 데이터.

### 비목표
- 서버/세이브/전투 로직 변경(표시만).
- 범례 토글/호버 툴팁(상시 표시로 MVP — 토글은 후속).
- 속성별 상세 상극 매트릭스 그래프(컴팩트 요약만).
- 신규 속성/색(기존 팔레트·라벨 재사용).

## 3. 스킬 속성 배지

### 3-1. ViewModel (`IdleHUD.h`)
`FIdleHUDSkillSlotViewModel`에 추가:
```cpp
ESkillElement Element = ESkillElement::None;
```
(`#include "CombatSystem/StatusElementTypes.h"` 필요 시 추가 — IdleHUD.h가 이미 ESkillElement 사용하면 불필요.)

### 3-2. 빌더 (`IdleHUD.cpp:2212~`)
`Slot.bCanRankUp = ...;` 인근에 추가:
```cpp
Slot.Element = Skill.Element;
```

### 3-3. 렌더 (`DrawSkillSlot`)
슬롯 우하단(랭크업 버튼·상태 라벨과 비충돌 위치)에 속성 배지. `Element != None`일 때만:
```cpp
if (Slot.Element != ESkillElement::None)
{
    const float BadgeSize = 16.0f * Scale;
    const float BadgeX = X + Width - BadgeSize - 8.0f * Scale;
    const float BadgeY = Y + Height - BadgeSize - 8.0f * Scale;
    DrawRect(StageWeakElementToColor(Slot.Element).CopyWithNewOpacity(0.92f), BadgeX, BadgeY, BadgeSize, BadgeSize);
    DrawText(StageWeakElementToIconLabel(Slot.Element).ToString(), Theme::BgPrimary, BadgeX + 4.0f * Scale, BadgeY + 1.0f * Scale, GEngine ? GEngine->GetSmallFont() : nullptr, 0.66f * Scale);
}
```
(좌표는 plan에서 기존 요소와 비충돌 확인 — 랭크업 버튼은 우상단 Y+6, 상태 라벨은 Y+24, 쿨다운 바는 Y+Height−12.)

## 4. 상성 범례

### 4-1. ViewModel (`IdleHUD.h`)
```cpp
struct IDLEPROJECT_API FIdleHUDElementLegendEntry
{
    FText Label;
    FText IconLabel;
    FLinearColor Color = FLinearColor::White;
};

struct IDLEPROJECT_API FIdleHUDElementLegendViewModel
{
    TArray<FIdleHUDElementLegendEntry> Elements; // Fire/Ice/Lightning/Holy/Dark
    FText WeakNote;   // 약점 +50%
    FText ResistNote; // 저항 -50%
};
```

### 4-2. 빌더 (`IdleHUD.cpp`)
`BuildElementLegendViewModel()`:
- `Elements` = Fire/Ice/Lightning/Holy/Dark 각각 `{StageWeakElementToLabel(e), StageWeakElementToIconLabel(e), StageWeakElementToColor(e)}`.
- `WeakNote` = `Localization::UI("ELEMENT_LEGEND_WEAK")`, `ResistNote` = `ELEMENT_LEGEND_RESIST`.

### 4-3. 렌더 (`DrawElementLegend`)
컴팩트 패널 — 제목(`ELEMENT_LEGEND_TITLE`) + 5속성 색 스와치+라벨 가로 나열 + 약점/저항 노트 1행. 위치는 스킬 바 위(또는 화면 우하단 모서리)로 기존 패널과 비충돌. DrawSkillHud 호출부 인근에서 1회 호출.

## 5. 로컬라이즈
UI.csv ko·en에 추가:
- `ELEMENT_LEGEND_TITLE` — ko "속성 상성" / en "Element Affinity"
- `ELEMENT_LEGEND_WEAK` — ko "약점 +50%" / en "Weakness +50%"
- `ELEMENT_LEGEND_RESIST` — ko "저항 −50%" / en "Resist −50%"
(속성 라벨은 기존 `ELEMENT_*` 재사용.)

## 6. 테스트 / 게이트
- `CombatTests`(또는 HUD 테스트): `BuildSkillSlotViewModels` 결과 슬롯 `Element == Skill.Element`(액티브 스킬에 속성 설정 시).
- `BuildElementLegendViewModel`: `Elements.Num() == 5` + 각 Label/IconLabel 비어있지 않음 + WeakNote/ResistNote 비어있지 않음.
- `LocalizationTests`: `ELEMENT_LEGEND_TITLE/WEAK/RESIST` ko·en orphan 0.
- 표준 jumbo + 전체 Automation(`IdleProject.UI.HUD`/`Localization`/`Combat`). **세이브·서버 무변경**(SaveVer 29).

## 7. 안전 가드
- `Element != None` 가드 — 무속성 스킬 슬롯 시각 불변.
- generic 헬퍼 재사용 → 속성 색/라벨 단일 출처(약점/저항/스킬/범례 일관).
- 범례 렌더 좌표 기존 HUD 패널과 비충돌 확인.
- 클라 전용 — 세이브/서버/전투 무변경.

## 8. 구현 단계화 (plan)
1. ViewModel 확장(스킬 슬롯 Element + 범례 구조체) + 빌더(스킬 Element 설정 + BuildElementLegendViewModel).
2. 렌더(DrawSkillSlot 배지 + DrawElementLegend 패널 + 호출).
3. 로컬라이즈 ELEMENT_LEGEND_* + LocalizationTests.
4. 회귀 테스트(스킬 Element + 범례 ViewModel).
5. 게이트: 표준 jumbo + Automation, 세이브·서버 무변경.

## 9. 후속
- 범례 토글/호버 툴팁, 상극쌍 화살표 그래프, 적 저항 대비 추천 스킬 하이라이트.
