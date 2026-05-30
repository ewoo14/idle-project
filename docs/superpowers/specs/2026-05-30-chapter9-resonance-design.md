# 챕터 9 (스테이지 81~90) + 속성 공명(이중 약점/저항) 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(콘텐츠 + 전용 기믹, 클릭) → plan 단계화
- 선행: 멀티챕터 일반화(#66/#93/#98/#103). 약점/원소 시스템(combat). 브랜치 `feat/chapter9-resonance`.

## 1. 배경

챕터 8(#103)까지 후존계 아크가 열렸다(균열 봉인 이후 더 오랜 것의 잔존). 챕터 9는 그 아크를 잇는 새 영역이며, **전용 전투 기믹 "속성 공명"**을 도입한다.

현 원소 시스템: 몬스터는 단일 `WeakElement`(스테이지 약점)를 스폰 시 보유(`StageInfo.WeakElement = getStageWeakElement(idx)`). 전투에서 `computeElementMultiplier(skillElement, weakElement)`가 약점 일치 ×1.5, 상극쌍(Fire↔Ice, Lightning↔Holy, Holy↔Dark) ×0.5, 그 외 ×1. 서버 `combat.ts`/`stage.ts` ↔ 클라 `CombatFormulas`/`StageFormula` parity.

**한계**: 스테이지당 원소 상호작용이 약점 하나에만 묶여, 부 저항은 상극쌍에 종속(독립 설계 불가).

## 2. 목표 / 비목표

### 목표
1. 챕터 9(스테이지 81~90): 클라 `TotalChapters` 8→9.
2. 약점 81~90(Dark 가중 4속성, None 없음) — 서버 stage.ts ↔ 클라 StageFormula parity.
3. **속성 공명 기믹**: 스테이지별 **독립 부 저항 속성** + 공명 데미지 배율.
4. 새 아크 스토리(후존계 계승) + 메인 퀘스트 `main_ch9_001~006`(3중 DefinitionParity).
5. **세이브 무변경**(SaveVer 29, 저항=스테이지 파생값). HUD 데이터 구동 자동.

### 비목표
- 신규 재화/드랍 변경(고레벨 → 기존 #65 레어도 자동).
- 약점/저항 외 추가 모디파이어(잔향/페이즈 등은 후속 챕터 기믹).
- balance-sim TOTAL_CHAPTERS 갱신(별도).
- SaveVer 변경.

## 3. 속성 공명 기믹 (전용)

### 3-1. 부 저항 속성
신규 `getStageResistElement(globalStageIndex)`:
- 스테이지 1~80: `None`(하위호환 — 기존 챕터 전투 완전 불변).
- 스테이지 81~90: 정의된 저항 속성(약점과 **독립**, 예시 plan 확정):
  `81 Ice / 82 Holy / 83 Dark / 84 Fire / 85 Holy / 86 Lightning / 87 Fire / 88 Lightning / 89 Holy / 90 Ice`
- 약점(plan 예시): `81 Dark / 82 Fire / 83 Lightning / 84 Holy / 85 Dark / 86 Ice / 87 Dark / 88 Holy / 89 Fire / 90 Dark` — 약점≠저항 보장.

### 3-2. 공명 데미지 배율
신규 `computeResonanceMultiplier(skillElement, weakElement, resistElement)`:
1. `skillElement === weakElement` → **1.5**(약점 우선).
2. else `resistElement !== "None" && skillElement === resistElement` → **0.5**(명시 저항).
3. else → `computeElementMultiplier(skillElement, weakElement)`(기존 상극/중립 로직).

**하위호환**: `resistElement === "None"`(스테이지 1~80) → 항상 3번으로 떨어져 **기존과 byte 동일**. 서버 `combat.ts` ↔ 클라 `CombatFormulas` parity.

### 3-3. 전투 배선
- `StageInfo`에 `ResistElement` 추가(`StageService` 계산 시 `getStageResistElement`).
- `AIdleMonster`에 `ResistElement` + setter/getter, 스폰 시 `Monster->SetResistElement(StageInfo.ResistElement)`(GameMode).
- `USkillComponent` 데미지 계산이 `ComputeElementMultiplier(Skill.Element, WeakElement)` → `ComputeResonanceMultiplier(Skill.Element, WeakElement, ResistElement)`로 교체.

## 4. 데이터 변경

### 약점/저항 (81~90)
서버 `stage.ts`: `getStageWeakElement` case 81~90 + 신규 `getStageResistElement`. 클라 `StageFormula`: `GetStageWeakElement` 81~90 + 신규 `GetStageResistElement`. byte parity.

### 메인 퀘스트 (main_ch9_001~006)
ch8 패턴 미러. `prerequisiteQuestId: main_ch8_006` → main_ch9_001 → … → 006. chapterMapId `9-1`…`9-10`. 목표 타입 ch8 동형(kill→clear→level→tower→kill→boss), 보상 글로벌 9x 스케일 자동. 클라 QuestDB.csv ↔ 서버 quests.ts ↔ QuestService DefinitionParity 1:1(59→65).

### 스토리 (Story.csv / StoryText.csv)
`STORY_C09_*` 진입/완료/보스 ko·en. 후존계 계승 — ch8 화자/톤 유지, 공명(원소 잔향) 서사 모티프 연결("후존계의 잔향이 적의 본질을 두 겹으로 비튼다" 류).

## 5. 클라 변경
- `StageService.h: TotalChapters 8 → 9`.
- `StageFormula` 약점 81~90 + `GetStageResistElement`.
- `StageInfo.ResistElement` + `StageService` 설정.
- `IdleMonster` ResistElement + 스폰 배선.
- `CombatFormulas::ComputeResonanceMultiplier` + `SkillComponent` 사용.
- HUD: 챕터 9 진입/배지/약점 색 데이터 구동 자동. **저항 표기 UI는 비목표**(데이터·전투만, 표시는 후속 — 약점 색 경로는 기존).

## 6. 테스트 / 게이트
- 서버 `stage.test.ts`: 약점 81~90 + 저항 81~90(약점≠저항, 1~80 저항 None). `combat.test.ts`: `computeResonanceMultiplier` 케이스(약점 1.5/저항 0.5/None 폴백 동일/우선순위). `quests.test.ts`/parity: ch9 6퀘스트 + 체인 + DefinitionParity.
- 클라 `CombatTests`: ComputeResonanceMultiplier parity(약점/저항/None 하위호환). `StageFormula` 약점·저항 parity. `LocalizationTests`: Story/StoryText/Quest ch9 ko·en 무결성.
- 표준 jumbo + 전체 Automation(Stage/Combat/Quest/Localization/UI). 서버 vitest + biome clean. **세이브 무변경**(SaveVer 29).

## 7. 안전 가드
- 저항=None(1~80) → 기존 전투 byte 동일(회귀 0). CombatTests 기존 단언 불변.
- 약점 None 미발생(#70). 약점≠저항(부 저항이 약점을 상쇄하지 않음).
- 세이브 무변경(글로벌 idx, 파생값) → 기존 진행 보존.
- 퀘스트 체인 prerequisite 정합(ch8 finale→ch9). DefinitionParity 게이트 적발.

## 8. 구현 단계화 (plan)
1. 서버: stage.ts 약점 81~90 + getStageResistElement + combat.ts computeResonanceMultiplier + 테스트.
2. 클라: StageService TotalChapters 9 + StageFormula 약점/저항 + StageInfo/Monster ResistElement + CombatFormulas ComputeResonanceMultiplier + SkillComponent 사용 + CombatTests.
3. 데이터: quests.ts main_ch9 6 + Quest.csv/QuestDB.csv + Story/StoryText ko·en ch9 + LocalizationTests.
4. 게이트: 서버 vitest+biome, 표준 jumbo+Automation, 세이브 무변경.

## 9. 후속
- 챕터 10+, 저항 표기 HUD, 잔향/페이즈 등 챕터 기믹, balance-sim TOTAL_CHAPTERS 갱신(stale).
