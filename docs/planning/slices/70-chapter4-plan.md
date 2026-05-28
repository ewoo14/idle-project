# 챕터4 콘텐츠 구현 계획 — PR #70

> **실행 방식:** 워크플로우 v3 [2] Codex 명세. character가 클라+서버 미러+테스트, [3] Claude TM 검증, [5] **UE Automation 직접**(#68 교훈). PM 자율. #66(챕터3/멀티챕터 일반화) 패턴 재사용.

**목표:** TotalChapters 3→4, ch4(idx 31~40) 약점·미니보스·챕터보스·스토리·퀘스트.

**아키텍처:** `StageService::TotalChapters` 4. `GetStageWeakElement` idx 31~40 추가. ch4 전환(3-10→4-1, 4-10 동결)은 기존 `OnChapterBossDefeated` 경로 자동. 저장은 TotalChapters만 증가(마이그레이션 불필요).

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 Stage/Quest 패턴.

---

## 인터페이스 계약

### 1. StageService.h / 서버 stage.ts
```cpp
static constexpr int32 TotalChapters = 4; // 3 → 4 (StagesPerChapter 10 유지)
```
서버 `stages.ts` 40 스테이지 생성(ch1~ch4). 글로벌 idx = `(Chapter-1)*10 + Stage`(ch4 31~40).

### 2. StageFormula GetStageWeakElement (idx 31~40)
```cpp
// 현재 default None인 31~40에 약점 5속성 조합 추가. 예:
// 31 Lightning / 32 Holy / 33 Ice / 34 Fire / 35 Dark(미니보스)
// 36 Lightning / 37 Holy / 38 Ice / 39 Fire / 40 Dark(챕터보스)
// 클라/서버 동일.
```

### 3. ch4 퀘스트 (quest)
- 클라 `QuestService` BuildDefaultDefinitions ch4 메인 5~7(스토리 §5.3 이후 훅, EQuestObjective 기존 타입) + 서버 `quests.ts` 미러 + DefinitionParity. 진행 훅(ch4 보스/스테이지) 기존 RecordQuestProgress.

### 4. 저장
- StageChapter/StageStage 기존(#52). TotalChapters만 4로 → ch4 진행 수용. **마이그레이션 불필요**(기존 진행 보존, StageStage 1~10/Chapter 1~4 유효). SaveVersion bump 불필요.

---

## 테스트 케이스

### 클라 Automation
- TotalChapters 4 반영, 글로벌 idx 경계(4-1 idx31, 4-10 idx40)
- IsBossStage(4,10,10)=true, IsEliteStage(5)=true
- 약점 idx 31~40 매핑 클라/서버 일치(default None 아님)
- 3-10 보스 → 4-1 진입, 4-10 보스 → 최종 동결
- ch4 메인 퀘스트 진행/완료
- 기존 ch1~3 진행 회귀안전(저장)

### 서버 vitest
- stage/stages 40 스테이지 parity, ch4 약점
- quests ch4 DefinitionParity

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | TotalChapters 4 + 약점 31~40 + 미니보스/보스 + 서버 stage/stages 미러 + Automation |
| story | 06-story-bible ch4 본문 |
| quest | 메인 퀘스트 ch4 + 서버 quests 미러 |
| designer | 챕터4 HUD/전환 + 로컬라이즈 ko/en |
| balance | ch4 스케일/보상 + 페이싱 시뮬 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] Codex 5파트 → [3] Claude TM → [4] fix(필요시) → [5] 검증(UE Automation 직접) → [N] CI 그린 + 머지

## Self-Review
- DoD 1~7 매핑 ✅
- placeholder 없음(약점 매핑 예시 명시, 정밀 수치 balance 위임)
- 타입 일관성: `TotalChapters`(4)/약점 31~40/`GetStageWeakElement` 일치
- 주의: 약점 idx 31~40 추가(default None 탈출) / 챕터 전환 #66 경로 재사용 / 저장 마이그레이션 불필요 / 클라↔서버 parity / [5] UE Automation 직접
