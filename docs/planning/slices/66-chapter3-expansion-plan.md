# 챕터3 + 스테이지 대확장 구현 계획 — PR #66

> **실행 방식:** 워크플로우 v3 [2] Codex 7파트 명세. character 대규모 횡단(스테이지/속성/저장/서버 미러), [3] Claude TM이 마이그레이션·parity·글로벌 idx 집중 검증. `grep ESkillElement`/`StagesPerChapter`/`GetStageWeakElement` 전수.

**목표:** 챕터 3개×10스테이지(미니보스/챕터보스) + 신규 속성 Dark + ch3 스토리/퀘스트.

**아키텍처:** `StagesPerChapter` 5→10, `TotalChapters` 2→3(전역). 글로벌 idx 1~30. 엘리트(X-5)/보스(X-10). `ESkillElement` +Dark 상성 확장. 저장 `SaveVersion` 7→8 마이그레이션.

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 Stage/Combat/Quest 패턴.

---

## 인터페이스 계약

### 1. StageService.h / stage.ts
```cpp
static constexpr int32 StagesPerChapter = 10; // 5 → 10
static constexpr int32 TotalChapters = 3;     // 2 → 3
```
서버: `DEFAULT_STAGES_PER_CHAPTER = 10`(stage.ts), `stages.ts` 30 스테이지 생성. 글로벌 idx = `(Chapter-1)*StagesPerChapter + Stage` (ch1 1~10, ch2 11~20, ch3 21~30).

### 2. StageFormula
```cpp
static bool IsBossStage(int32 Chapter, int32 Stage, int32 StagesPerChapter); // Stage == StagesPerChapter(10)
static bool IsEliteStage(int32 Stage);  // 신규: Stage == 5
static ESkillElement GetStageWeakElement(int32 GlobalStageIndex); // 1~30 약점 (5속성 순환/조합)
```
- 약점 30 매핑: 5속성(Fire/Ice/Lightning/Holy/Dark) 순환. ch3(21~30)는 Dark 비중↑(심연계). 클라/서버 동일.
- 엘리트 보상: `FRewardFormula`에 엘리트 가중(예 보스 8× 대비 엘리트 3×) 또는 `IsEliteStage` 분기.

### 3. ESkillElement (StatusElementTypes.h) + 상성
```cpp
enum class ESkillElement : uint8
{
    None = 0, Fire = 1, Ice = 2, Lightning = 3, Holy = 4, Dark = 5  // Dark 신규
};
```
- 상성(`CombatFormulas` 약점×1.5/저항×0.5/무1.0): 기존 Fire↔Ice, Lightning↔Holy. **Dark↔Holy 상극**(Holy가 Dark 약점, Dark가 Holy 약점 또는 비대칭 — 설계: Holy 공격 vs Dark 몬스터 ×1.5, Dark 공격 vs Holy 몬스터 ×1.5, 상호 약점). 클라/서버 `combat`/`skills` 미러. grep `ESkillElement` 전수 5속성 switch 처리.

### 4. IdleSaveGame.h + 마이그레이션
```cpp
UPROPERTY() int32 SaveVersion = 8; // 7 → 8
```
- `ApplyFromSave`/StageService RestoreState: 기존 5스테이지 진행(StageStage 1~5) → 10스테이지 구조 수용(StageStage 1~5 유효, 그대로). `bChapter1BossDefeated`(기존 1-5 클리어=챕터보스였음) → 새 구조에서 환생 게이트 의미 유지(기존 클리어자는 ch1 완료로 간주, StageHighestClearedChapter 보존). SaveVersion<8 게이트(이중 적용 방지).
- ※ 글로벌 idx 변경으로 약점이 바뀌지만 진행 위치(Chapter/Stage)는 보존.

### 5. 서버 미러
- `stage.ts`(DEFAULT_STAGES_PER_CHAPTER 10, weakness 30, isEliteStage), `stages.ts`(30 정의), `combat.ts`(Dark 상성), `skills.ts`(Dark 부여 가능), `quests.ts`(ch3 메인). parity + `Math.fround`.

### 6. ch3 퀘스트 (quest)
- 클라 `QuestService` BuildDefaultDefinitions ch3 메인 퀘스트(스토리 §5.3 훅) + 서버 `quests.ts` 미러. DefinitionParity. 진행 훅(ch3 보스/스테이지) 기존 RecordQuestProgress 경유.

---

## 테스트 케이스

### 클라 Automation
- StagesPerChapter 10/TotalChapters 3 반영, 글로벌 idx 경계(1-10 idx10, 2-1 idx11, 3-10 idx30)
- IsEliteStage(5)=true, IsBossStage(.,10,10)=true, IsBossStage(.,5,10)=false
- 약점 30 idx 매핑(ch3 Dark 포함) 클라/서버 일치
- Dark 상성: Holy 공격 vs Dark 몬스터 ×1.5, Dark vs Holy ×1.5, Dark vs None ×1.0
- 엘리트 보상 가중(일반<엘리트<보스)
- 저장 v7→v8: 기존 5스테이지 세이브 → 10스테이지 진행 보존, bChapter1BossDefeated 매핑, 이중적용 방지
- ch3 메인 퀘스트 진행/완료
- 2-10 보스 → 3-1 진입, 3-10 보스 → 최종 동결

### 서버 vitest
- stage/stages 30 스테이지 parity, isEliteStage
- combat Dark 상성 parity(Math.fround)
- quests ch3 DefinitionParity
- 약점 30 클라 앵커 일치

---

## Codex 작업 분배 (7파트)
| 파트 | 작업 |
| --- | --- |
| character (메인) | StagesPerChapter 10/TotalChapters 3/IsEliteStage/약점30/ESkillElement Dark+상성/저장 v8/서버 stage·stages·combat·skills 미러/Automation |
| story | 06-story-bible ch3 본문 |
| quest | 메인 퀘스트 ch3 + 서버 quests 미러 |
| designer | 챕터3/엘리트 HUD + Dark 색/아이콘 + 로컬라이즈 ko/en |
| balance | 30스테이지/엘리트/Dark 상성 + 기존 페이싱 시뮬 |
| backend | 서버 미러 parity 검증 보강 |
| qa | 30스테이지/엘리트/약점/Dark/마이그레이션 v7→v8 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] Codex 7파트 → [3] Claude TM → [4] fix → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(약점 매핑 원칙·상성·엘리트 가중 명시, 정밀 수치 balance 위임)
- 타입 일관성: `StagesPerChapter`(10)/`TotalChapters`(3)/`IsEliteStage`/`ESkillElement::Dark`/`SaveVersion`(8) 전 섹션 일치
- 주의: 전역 5→10 마이그레이션(SaveVersion<8 게이트 1회) / Dark grep 전수 5속성 / 글로벌 idx 재계산 / 클라↔서버 parity 동시
