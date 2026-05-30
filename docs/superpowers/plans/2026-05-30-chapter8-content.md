# 챕터 8 (스테이지 71~80) 콘텐츠 구현 Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans. Steps use checkbox (`- [ ]`).

**Goal:** 챕터 8(스테이지 71~80)을 #98(챕터7) 패턴으로 추가한다 — TotalChapters 8, 약점 71~80, 새 아크 스토리, 메인퀘스트 main_ch8 6개 체인. 세이브 무변경.

**Architecture:** 데이터 구동 멀티챕터(#66 일반화). 약점은 서버 stage.ts ↔ 클라 StageFormula parity. 퀘스트는 클라 QuestDB.csv ↔ 서버 quests.ts DefinitionParity 1:1. 스토리/퀘스트 텍스트는 ko·en CSV. HUD 자동(신규 코드 0).

**Tech Stack:** 서버 TS+vitest+biome. 클라 UE5 C++ + CSV. 게이트 `tools/ci/ue-automation.ps1`.

**스펙:** `docs/superpowers/specs/2026-05-30-chapter8-content-design.md`. 브랜치 `feat/chapter8-content`.
**선행 참고:** ch7(#98) 동형 — 모든 변경은 ch7 엔트리/케이스를 미러.

---

## Task 1: 서버 — 약점 71~80 + 메인퀘스트 main_ch8

**Files:** Modify `server/src/core/formulas/stage.ts`, `server/src/core/data/quests.ts`; tests `stage.test.ts`, `quests.test.ts`(존재 시).

- [ ] **Step 1: 약점 71~80 실패 테스트**

`server/src/core/formulas/stage.test.ts`에 추가(기존 약점 테스트 양식 따라):

```ts
it("챕터8 약점 71~80 (None 없음, Dark 가중)", () => {
  const ch8 = [71, 72, 73, 74, 75, 76, 77, 78, 79, 80].map(getStageWeakElement);
  expect(ch8).toEqual(["Dark", "Lightning", "Holy", "Fire", "Dark", "Ice", "Dark", "Holy", "Fire", "Dark"]);
  expect(ch8).not.toContain("None");
});
```

- [ ] **Step 2: 실패 확인**

Run: `cd server; npx vitest run src/core/formulas/stage.test.ts`
Expected: FAIL(71~80 미정의 → 기본값 반환)

- [ ] **Step 3: stage.ts 약점 케이스 추가**

`stage.ts getStageWeakElement`의 `case 70:` 다음, default 앞에 추가:

```ts
    case 71:
      return "Dark";
    case 72:
      return "Lightning";
    case 73:
      return "Holy";
    case 74:
      return "Fire";
    case 75:
      return "Dark";
    case 76:
      return "Ice";
    case 77:
      return "Dark";
    case 78:
      return "Holy";
    case 79:
      return "Fire";
    case 80:
      return "Dark";
```

- [ ] **Step 4: 약점 통과 확인**

Run: `cd server; npx vitest run src/core/formulas/stage.test.ts` → PASS

- [ ] **Step 5: 메인퀘스트 main_ch8 추가**

`quests.ts`의 `main_ch7_006` 엔트리 다음(daily 퀘스트 앞)에 6개 추가(ch7 미러, 보상 ~+13% 스케일, reach_level 95/climb_tower 55):

```ts
  {
    questId: "main_ch8_001",
    type: "main",
    title: "Echoes Beyond the Sealed Rift",
    objective: "kill_monster",
    targetCount: 130,
    rewardGold: 484_000,
    rewardExp: 363_000,
    prerequisiteQuestId: "main_ch7_006",
    chapterMapId: "8-1",
  },
  {
    questId: "main_ch8_002",
    type: "main",
    title: "Tracks in the Stillness' Wake",
    objective: "clear_map",
    targetCount: 1,
    rewardGold: 547_000,
    rewardExp: 410_000,
    prerequisiteQuestId: "main_ch8_001",
    chapterMapId: "8-2",
  },
  {
    questId: "main_ch8_003",
    type: "main",
    title: "Strength for the New Threshold",
    objective: "reach_level",
    targetCount: 95,
    rewardGold: 618_000,
    rewardExp: 463_000,
    prerequisiteQuestId: "main_ch8_002",
    chapterMapId: "8-4",
  },
  {
    questId: "main_ch8_004",
    type: "main",
    title: "Quiet the Risen Remnant",
    objective: "climb_tower",
    targetCount: 55,
    rewardGold: 698_000,
    rewardExp: 524_000,
    prerequisiteQuestId: "main_ch8_003",
    chapterMapId: "8-5",
  },
  {
    questId: "main_ch8_005",
    type: "main",
    title: "Through the Afterrift Expanse",
    objective: "kill_monster",
    targetCount: 165,
    rewardGold: 789_000,
    rewardExp: 592_000,
    prerequisiteQuestId: "main_ch8_004",
    chapterMapId: "8-8",
  },
  {
    questId: "main_ch8_006",
    type: "main",
    title: "Confront What the Rift Sealed",
    objective: "defeat_boss",
    targetCount: 1,
    rewardGold: 928_000,
    rewardExp: 696_000,
    prerequisiteQuestId: "main_ch8_005",
    chapterMapId: "8-10",
  },
```

> 주: `objective`/필드명은 ch7 엔트리와 **정확히 동일 키**를 써야 한다(quests.ts 실제 ch7 엔트리 확인). `title`은 영문(클라 로컬라이즈가 ko 제공). reward 스케일은 ch7 마지막(428k)에서 연속 증가.

- [ ] **Step 6: 퀘스트 테스트(존재 시) + 전체 + biome**

Run: `cd server; npm test`
Expected: GREEN(stage 약점 + 퀘스트 카운트/체인 테스트 통과). 퀘스트 개수 단언이 있으면 +6 반영.

Run: `cd server; npx biome check . ../tools/balance-sim`
Expected: clean(위반 시 `--write` 후 재확인)

- [ ] **Step 7: 커밋**

```bash
git add server/src/core/formulas/stage.ts server/src/core/formulas/stage.test.ts server/src/core/data/quests.ts
git commit -m "feat(server): 챕터8 약점 71~80 + 메인퀘스트 main_ch8 6 체인"
```

---

## Task 2: 클라 — TotalChapters 8 + StageFormula 약점 parity

**Files:** Modify `client/Source/IdleProject/GameCore/StageService.h`, 클라 약점 공식 파일(`StageFormula.h/.cpp` 또는 StageService 내), 회귀 테스트.

- [ ] **Step 1: TotalChapters 8**

`StageService.h`의 `static constexpr int32 TotalChapters = 7;` → `8`.

- [ ] **Step 2: 클라 약점 71~80 parity**

클라의 `getStageWeakElement` 대응 구현(서버 stage.ts와 미러하는 곳 — `grep -rn "WeakElement" client/Source/IdleProject`로 위치 확인)에 case 71~80을 서버와 **동일 배치**로 추가:
`71 Dark / 72 Lightning / 73 Holy / 74 Fire / 75 Dark / 76 Ice / 77 Dark / 78 Holy / 79 Fire / 80 Dark`.

- [ ] **Step 3: 약점 parity 회귀 테스트**

기존 약점 parity 테스트(클라 StageTests/StageFormulaTests)에 71~80 단언 추가(서버와 동일 배열). 위치는 `grep -rn "WeakElement" client/Source/IdleProject/Tests`로 확인.

- [ ] **Step 4: 빌드 + Stage Automation**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.Stage+IdleProject.GameCore"`
Expected: 빌드 성공 + 약점 71~80 parity GREEN, 기존 Stage 회귀 GREEN

- [ ] **Step 5: 커밋**

```bash
git add client/Source/IdleProject/GameCore/StageService.h client/Source/IdleProject/GameCore/StageFormula.h client/Source/IdleProject/GameCore/StageFormula.cpp client/Source/IdleProject/Tests/
git commit -m "feat(client): TotalChapters 8 + StageFormula 약점 71~80 parity"
```

> 주: StageFormula 파일 경로가 다르면(예: StageService.cpp 내 정의) 실제 위치로 add. 약점 정의가 클라에 없고 서버 권위만이면 Step 2~3은 클라 미러 위치 확인 후 진행(#98이 어떻게 했는지 `git show` 참고).

---

## Task 3: 데이터 — 스토리 + 퀘스트 텍스트 (ko·en) + QuestDB

**Files:** Modify `client/Content/Data/QuestDB.csv`, `client/Content/Localization/Game/{ko,en}/Quest.csv`, `Story.csv`, `StoryText.csv`.

- [ ] **Step 1: QuestDB.csv ch8 6행**

`QuestDB.csv`에 main_ch8_001~006 6행 추가(서버 quests.ts와 **questId/objective/targetCount/prerequisite/chapterMapId 1:1**). ch7 행 양식 그대로.

- [ ] **Step 2: Quest.csv ko·en (제목/설명)**

`ko/Quest.csv`, `en/Quest.csv`에 main_ch8_001~006 제목·설명 키 추가. en은 quests.ts title 영문, ko는 한글 번역(서사 톤 유지).

- [ ] **Step 3: Story/StoryText ch8 (새 아크)**

`Story.csv`/`StoryText.csv` ko·en에 `STORY_C08_*` 키 추가(ch7 `STORY_C07_*` 키 네이밍·행수 미러): 진입(8-1)/주요 구간/미니보스(8-5)/전실(8-9)/보스(8-10 시작·완료). **새 아크 서사**(균열 종결 이후 — "끝난 줄 알았으나 닫힌 자리에서 드러난 것"), ch7 화자 톤 유지.

> 주: ch7 키 목록을 `grep "STORY_C07" StoryText.csv`로 뽑아 동일 구조(_M01_INTRO/_CLEAR, _M03, _M05, _M09, _M10_BOSS, _M10_CLEAR 등)를 C08로 미러. ko·en 모두.

- [ ] **Step 4: LocalizationTests 게이트**

Run: `./tools/ci/ue-automation.ps1 -Filter "IdleProject.Localization+IdleProject.Quest"`
Expected: 빌드 성공 + Localization ch8 키 무결성(Story/StoryText/Quest ko·en) + Quest DefinitionParity(ch8 6) GREEN

- [ ] **Step 5: 커밋**

```bash
git add client/Content/Data/QuestDB.csv "client/Content/Localization/Game/ko/Quest.csv" "client/Content/Localization/Game/en/Quest.csv" "client/Content/Localization/Game/ko/Story.csv" "client/Content/Localization/Game/en/Story.csv" "client/Content/Localization/Game/ko/StoryText.csv" "client/Content/Localization/Game/en/StoryText.csv"
git commit -m "feat(content): 챕터8 스토리(새 아크) + 퀘스트 텍스트 ko·en + QuestDB"
```

---

## Task 4: 최종 게이트

- [ ] **Step 1: 서버 전체 + biome**

Run: `cd server; npm test` → GREEN
Run: `cd server; npx biome check . ../tools/balance-sim` → clean

- [ ] **Step 2: UE 표준 jumbo + 전체 Automation**

Run: `./tools/ci/ue-automation.ps1`
Expected: Fail 0, EXIT 0. Stage/Quest/Localization/UI/GameCore/Combat GREEN. **SaveVer 29 무변경 확인**(세이브 단언 그대로).

- [ ] **Step 3: 세이브 무변경 확인**

Run: `grep -rn "SaveVersion = 2" client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp`
Expected: 둘 다 29(변경 없음 — 챕터는 세이브 무관).

---

## Self-Review (작성자 점검)

**스펙 커버리지:**
- §2.1 TotalChapters 8 → Task 2 ✅
- §2.2 약점 71~80 parity(None 없음) → Task 1,2 ✅
- §2.3 새 아크 스토리 → Task 3 ✅
- §2.4 메인퀘스트 main_ch8 6 체인 + DefinitionParity → Task 1,3 ✅
- §2.5 세이브 무변경 → Task 4 Step 3 ✅
- §6 게이트(stage/quest 테스트·LocalizationTests·jumbo·biome) → Task 1~4 ✅

**비목표:** 신규 기믹/재화/balance-sim/SaveVer → 미포함(의도적) ✅

**플레이스홀더:** "주:" 는 실제 파일 위치/키 목록 확인 안내(약점 배치·퀘스트 키·스토리 키 구조 제공). TODO/TBD 없음.

**타입 일관성:** 약점 배열 71~80은 Task 1(서버)·Task 2(클라)·테스트 **동일 배치**(Dark/Lightning/Holy/Fire/Dark/Ice/Dark/Holy/Fire/Dark). 퀘스트 questId/chapterMapId/objective는 서버 quests.ts ↔ 클라 QuestDB.csv ↔ Quest.csv 정합.

**세이브:** 무변경(SaveVer 29) — 챕터는 글로벌 idx, 마이그레이션 불필요(#98 입증).
