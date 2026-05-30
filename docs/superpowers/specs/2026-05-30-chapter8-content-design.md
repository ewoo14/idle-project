# 챕터 8 (스테이지 71~80) 콘텐츠 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(순수 콘텐츠, #98 패턴) → plan 단계화
- 선행 패턴: #66(챕터3 일반화)/#93(챕터6)/#98(챕터7). 브랜치 `feat/chapter8-content`.

## 1. 배경

멀티챕터 구조는 #66에서 일반화되어 데이터 구동으로 챕터를 누적해 왔다(현재 챕터 7, 글로벌 스테이지 70). 챕터 7에서 **"균열" 서사가 종결**됐다("균열은 마침내 닫혀 끝났습니다"). 챕터 8은 그 **이후의 새 서사 아크**를 연다.

현 구조:
- 클라 `UStageService::TotalChapters = 7`(StageService.h:48), `StagesPerChapter = 10`. 글로벌 idx `(Chapter-1)*10+Stage`.
- 스테이지 스케일 `computeMonsterStatMultiplier(idx) = 1 + (idx-1)*0.15`(연속, 자동 외삽). 미니보스=stage5(`IsElite`)/챕터보스=stage10(`IsBoss`).
- 약점 `getStageWeakElement(idx)`(서버 stage.ts ↔ 클라 StageFormula parity), 현재 70까지 정의. **None 없음(#70 교훈)**.
- 메인 퀘스트 `main_ch7_001~006`(quests.ts + Quest.csv, chapterMapId 7-1…7-10 체인, 클라↔서버 DefinitionParity 1:1).
- 스토리 `Story.csv`/`StoryText.csv`(ko/en), 챕터별 진입/완료/보스 텍스트.
- **서버에 챕터 카운트 상수 없음** — 데이터(퀘스트/약점 케이스)로만 챕터가 존재. 클라 `TotalChapters`만 정수 상수.

## 2. 목표 / 비목표

### 목표
1. 챕터 8(스테이지 71~80) 추가: 클라 `TotalChapters` 7→8.
2. 약점 71~80(Dark 가중 4속성, None 없음) — 서버 stage.ts ↔ 클라 StageFormula parity.
3. 새 아크 스토리(균열 종결 이후) — Story/StoryText ko·en.
4. 메인 퀘스트 `main_ch8_001~006` 체인(ch7 finale→ch8 순차) — quests.ts + Quest.csv + QuestDB.csv, DefinitionParity 1:1.
5. **세이브 무변경**(글로벌 idx, SaveVer 29 유지). HUD 데이터 구동 자동.

### 비목표
- 신규 기믹/시스템(순수 콘텐츠 슬라이스 — 사용자 선택).
- 신규 재화/드랍 테이블 변경(고레벨 → 기존 #65 레어도 드롭 자동 적용).
- balance-sim TOTAL_CHAPTERS 갱신(별도, 이번 제외).
- SaveVer 변경(불필요).

## 3. 서사 (새 아크 방향)

챕터 7에서 균열은 닫혔다. 챕터 8은 **균열이 닫힌 자리에 남은 여파/새 위협**을 다룬다. 구체 서사는 구현 시 story 작가 서브에이전트가 ch7 finale와 자연스럽게 이어지도록 작성하되, 방향은:
- 균열이 닫힌 뒤 그 경계에 새로 드러난 영역/세력(예: 균열이 봉인하던 더 깊은 무언가, 혹은 닫힌 균열의 잔재가 만든 새 질서).
- 미니보스(8-5)/챕터보스(8-10)는 그 새 위협의 정점.
- ch7 등장 화자(사르겔/이리스 등) 톤 유지, ch8 도입에서 "끝난 줄 알았으나" 류의 전환.

## 4. 데이터 변경

### 약점 (71~80, Dark 가중 4속성)
서버 `stage.ts getStageWeakElement` + 클라 `StageFormula`에 case 71~80 추가. Dark 가중(고챕터 패턴, #66~#98 일관), Fire/Ice/Lightning/Holy/Dark 중 None 없음. 예시 배치(구현 시 확정):
`71 Dark / 72 Lightning / 73 Holy / 74 Fire / 75 Dark / 76 Ice / 77 Dark / 78 Holy / 79 Fire / 80 Dark`

### 메인 퀘스트 (main_ch8_001~006)
ch7 패턴 미러. `prerequisiteQuestId: main_ch7_006` → main_ch8_001 → … → 006. chapterMapId `8-1`…`8-10`. 목표 타입은 ch7과 동형(kill→clear→level→tower→kill→boss 류), 보상은 글로벌 스테이지 8x 스케일 자동. 클라 QuestDB.csv ↔ 서버 quests.ts DefinitionParity 1:1(개수/ID/전제/맵 일치).

### 스토리 (Story.csv / StoryText.csv)
ch7 키 네이밍(`STORY_C07_*`) 따라 `STORY_C08_*` 진입/완료/보스 텍스트 ko·en. 미니보스(8-5)/보스(8-10) 시작·완료 포함.

## 5. 클라 변경
- `StageService.h: TotalChapters 7 → 8`.
- `StageFormula`(클라) getStageWeakElement 케이스 71~80(서버 parity).
- HUD: 챕터 8 진입/배지/약점 색은 기존 데이터 구동 경로가 자동 처리(#80/#98 입증, 신규 코드 0).

## 6. 테스트 / 게이트
- 서버 `stage.test.ts`: 약점 71~80 단언(None 없음). `quests.test.ts`/parity: ch8 6퀘스트 + 체인 + DefinitionParity.
- 클라 `LocalizationTests`: Story/StoryText/Quest ch8 ko·en 키 무결성(#98 게이트 패턴). StageFormula 약점 parity 회귀.
- 표준 jumbo + 전체 Automation(Stage/Quest/Localization/UI). 서버 vitest + biome clean. **세이브 무변경 확인**(SaveVer 29).

## 7. 안전 가드
- 세이브 무변경(글로벌 idx) → 기존 진행 보존, 마이그레이션 불필요.
- 약점 None 미발생(#70). 약점 케이스 71~80 서버·클라 동일.
- 퀘스트 체인 prerequisite 정합(ch7 finale→ch8). DefinitionParity 깨지면 게이트 적발.

## 8. 구현 단계화 (plan)
- 서버: stage.ts 약점 71~80 + quests.ts main_ch8 6 + 테스트.
- 클라: StageService TotalChapters 8 + StageFormula 약점 71~80 parity + 회귀.
- 데이터: Quest.csv/QuestDB.csv ch8 6 + Story.csv/StoryText.csv ko·en ch8 + LocalizationTests 게이트.
- 게이트: 서버 vitest+biome, 표준 jumbo+Automation, 세이브 무변경 확인.

## 9. 후속
- 챕터 9+, 챕터 전용 기믹, balance-sim TOTAL_CHAPTERS 갱신(현재 4, 누적 stale).
