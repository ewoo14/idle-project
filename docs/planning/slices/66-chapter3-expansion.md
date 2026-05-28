# PR #66 기획서 — 챕터3 + 스테이지 대확장 (콘텐츠)

> **사용자 선택/지시**: 챕터3 콘텐츠 + **챕터당 10스테이지(전역 5→10)** + **미니보스/엘리트** + **신규 속성(Dark/심연)**. ch2(#37) 이후 진행축이자 멀티챕터 일반화(#37) 위 대확장. 콘텐츠 볼륨([[project-content-richness]]). client + server 멀티시스템(7파트). [[project-session-progress]].

## 1. 목표 / DoD
챕터가 3개(각 10스테이지)로 확장되고, 각 챕터에 미니보스(X-5)·챕터보스(X-10)가 있으며, 신규 속성 Dark가 상성에 편입되고, ch3 스토리/퀘스트가 연결된다. 기존 스테이지(#31/#37)·전투/상성(#30/#35)·저장(#52)·레어도(#65)와 정합.

### DoD 검증
1. **스테이지 구조(전역 10)**: `StagesPerChapter` 5→10, `TotalChapters` 2→3. 총 30스테이지(ch1 1-1~1-10/ch2 2-1~2-10/ch3 3-1~3-10), 글로벌 idx 1~30. 클라 `StageService`/`StageFormula` + 서버 `stage.ts`/`stages.ts` 미러.
2. **미니보스/엘리트**: `FStageFormula::IsEliteStage(Stage)`(Stage==5) — 강화 몬스터(HP/공격 증폭 + 전용 보상 가중). `IsBossStage`(Stage==10, 기존 5→10 이동). 클라/서버 미러. 엘리트 보상 `FRewardFormula` 가중.
3. **약점 30스테이지**: `GetStageWeakElement(GlobalStageIndex)` idx 1~30 재정의 — 5속성(Fire/Ice/Lightning/Holy/Dark) 순환/조합. 클라/서버 parity.
4. **신규 속성 Dark**: `ESkillElement` `Dark=5` 추가. 상성(`CombatFormulas`/`CombatComponent` #30) 확장 — 예 Holy↔Dark 상극(×1.5/×0.5), Dark 무저항(×1.0). 스킬/몬스터 속성 부여 가능. 클라/서버 `combat`/`skills` 미러. grep `ESkillElement`/element 전수 5속성 처리.
5. **ch3 스토리/퀘스트**: `06-story-bible.md` ch3 "차원 군주의 그림자" 본문(심연계 경계). 메인 퀘스트 ch3 신규(#56 ch1 7+ch2 5 → ch3 추가) + 진행 훅. 로컬라이즈 ko/en.
6. **저장 마이그레이션(SaveVersion 7→8)**: 기존 5스테이지 세이브(StageStage 1~5, ch1~2) → 10스테이지 구조. `bChapter1BossDefeated`(기존 1-5 보스 클리어 = 새 1-5 미니보스 → 환생 게이트 정책 유지) 매핑. 회귀안전 + 라운드트립 Automation.
7. **레어도 연계(자동)**: ch3 고레벨 → #65 유니크/초월 드롭 자동(drop 곡선 레벨 비례 무변경).
8. **테스트**: 클라 Automation(30스테이지 진행·엘리트/보스·약점30·Dark 상성·ch3 퀘스트·저장 v7→v8 마이그레이션) + 서버 vitest(stage/stages/combat/skills/quest 미러 parity). UE Build/Automation + 서버 build/test/lint **GREEN**, server-ci CI 그린([[feedback-ci-before-merge]]).

## 2. 범위 (In Scope)
### 2.1 스테이지 구조 (character + backend)
StagesPerChapter 10 + TotalChapters 3 + 엘리트/보스 + 약점30 + 글로벌 idx 재계산. 서버 stage/stages 미러.
### 2.2 신규 속성 Dark (character + backend)
ESkillElement Dark + 상성 확장. combat/skills 미러.
### 2.3 ch3 스토리 (story) / 퀘스트 (quest)
스토리바이블 ch3 + 메인 퀘스트 ch3 + 서버 quests 미러.
### 2.4 저장 마이그레이션 v8 (character)
5→10 스테이지 + bChapter1BossDefeated 매핑.
### 2.5 UI (designer)
챕터3/엘리트 표시 + Dark 속성 색/아이콘 + 로컬라이즈 ko/en.
### 2.6 밸런스 (balance)
30스테이지 스케일/보상/엘리트/Dark 상성 + 기존 ch1/ch2 페이싱 영향 시뮬.
### 2.7 테스트 — DoD 8.

## 3. 범위 외 (후속)
- 챕터4+ · ch3 전용 신규 시스템 · 챕터 선택/리플레이 UI · 엘리트 전용 룬/세트 · Dark 전용 스킬 트리 확장.

## 4. 작업 분배 — Codex 호출 (7파트)
| 파트 | 작업 |
| --- | --- |
| character (메인) | StagesPerChapter 10/TotalChapters 3/IsEliteStage/약점30/ESkillElement Dark+상성/저장 v8 마이그레이션/서버 미러 전부/Automation |
| story | 스토리바이블 ch3 "차원 군주의 그림자" 본문 + 용어/톤 |
| quest | 메인 퀘스트 ch3 + 진행 훅 + 서버 quests 미러 |
| designer | 챕터3/엘리트 HUD + Dark 속성 색/아이콘 + 로컬라이즈 ko/en |
| balance | 30스테이지/엘리트/Dark 상성 곡선 + 기존 페이싱 영향 시뮬 |
| backend | 서버 stage/stages/combat/skills/quest 미러 parity 검증 |
| qa | 30스테이지/엘리트/약점/Dark/마이그레이션 v7→v8 시나리오 |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+story/quest/designer/balance/backend/qa) → [3] Claude TM → [4] Codex fix → [5] 검증 → [N] **CI 그린 확정** + PM 종합 소견 + 머지.

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 전역 5→10이 기존 ch1/ch2 약점·밸런스·진행 저장 영향 | 약점30 재정의 + balance 페이싱 재검증 + 저장 v7→v8 마이그레이션 + 라운드트립 |
| 글로벌 idx 재계산(ch2 6→11) 오류 | StageService 글로벌 idx 공식 + Automation 경계(1-10/2-1/3-10) |
| 신규 속성 Dark 횡단(상성/스킬/몬스터) | grep ESkillElement/element 전수 5속성 + combat/skills parity(Math.fround) |
| bChapter1BossDefeated 의미 변동(1-5→1-10) | 환생 게이트 정책 명시 + 마이그레이션 매핑 + Automation |
| 엘리트 보상 밸런스 | 엘리트 가중 보수적 + balance 시뮬 |
| 클라/서버 30스테이지·상성 불일치 | stage/stages/combat parity + DefinitionParity |
| 클라우드(#54)/레어도(#65) 정합 | 저장 v8 round-trip + ch3 유니크/초월 드롭 곡선 확인 |

## 7. 후속
- 챕터4, 엘리트 전용 보상/룬, Dark 스킬 트리, 챕터 리플레이.
