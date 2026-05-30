# 챕터 9 + 속성 공명 구현 플랜

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 챕터 9(스테이지 81~90) 콘텐츠 + 전용 기믹 "속성 공명"(독립 부 저항 + 공명 데미지 배율)을 서버/클라 parity로 추가한다.

**Architecture:** 서버 `stage.ts`/`combat.ts`에 약점 81~90·`getStageResistElement`·`computeResonanceMultiplier` 추가, 클라 `StageFormula`/`CombatFormulas`/`StageService`/`IdleMonster`/`SkillComponent`에 byte parity 미러. 저항=None(1~80)이면 기존과 동일(회귀 0). 콘텐츠(스토리/퀘스트)는 ch8 패턴 미러. 세이브 무변경.

**Tech Stack:** Node/TS/vitest/biome(server), UE5.7 C++/Automation(client), CSV 로컬라이즈.

---

## 파일 구조

- `server/src/core/formulas/stage.ts` — 약점 81~90 + `getStageResistElement`.
- `server/src/core/formulas/combat.ts` — `computeResonanceMultiplier`.
- `server/src/core/formulas/stage.test.ts`, `combat.test.ts` — 회귀.
- `server/src/core/data/quests.ts` + `quests.test.ts` — main_ch9.
- `client/.../GameCore/StageFormula.h/.cpp` — 약점 81~90 + `GetStageResistElement`.
- `client/.../GameCore/StageService.h/.cpp` — TotalChapters 9 + StageInfo.ResistElement.
- `client/.../CharacterSystem/IdleMonster.h` — ResistElement.
- `client/.../CombatSystem/CombatFormulas.h/.cpp` — `ComputeResonanceMultiplier`.
- `client/.../CombatSystem/SkillComponent.cpp` — 공명 배율 사용.
- `client/.../IdleProjectGameModeBase.cpp` — 몬스터 ResistElement 배선.
- `client/.../Tests/CombatTests.cpp`, `StageFormulaTests`(있으면) — 회귀.
- `client/Content/.../Quest*.csv`, `Story*.csv`, `StoryText*.csv` + 클라 QuestService — 콘텐츠.

---

### Task 1: 서버 — 약점/저항 81~90 + 공명 배율 + 테스트

**Files:**
- Modify: `server/src/core/formulas/stage.ts`
- Modify: `server/src/core/formulas/combat.ts`
- Modify: `server/src/core/formulas/stage.test.ts`, `combat.test.ts`

- [ ] **Step 1: stage.ts 약점 81~90 추가**

`getStageWeakElement`의 `case 80: return "Dark";` 다음, `default` 앞에 추가:
```ts
    case 81: return "Dark";
    case 82: return "Fire";
    case 83: return "Lightning";
    case 84: return "Holy";
    case 85: return "Dark";
    case 86: return "Ice";
    case 87: return "Dark";
    case 88: return "Holy";
    case 89: return "Fire";
    case 90: return "Dark";
```

- [ ] **Step 2: stage.ts getStageResistElement 추가**

`getStageWeakElement` 함수 다음에 추가:
```ts
// 챕터 9 전용 부 저항 속성(스테이지 81~90). 1~80은 None(하위호환 — 기존 전투 불변).
export function getStageResistElement(globalStageIndex: number): StageElement {
  switch (Math.max(0, globalStageIndex)) {
    case 81: return "Ice";
    case 82: return "Holy";
    case 83: return "Dark";
    case 84: return "Fire";
    case 85: return "Holy";
    case 86: return "Lightning";
    case 87: return "Fire";
    case 88: return "Lightning";
    case 89: return "Holy";
    case 90: return "Ice";
    default: return "None";
  }
}
```

- [ ] **Step 3: combat.ts computeResonanceMultiplier 추가**

`computeElementMultiplier` 함수 다음에 추가:
```ts
/**
 * 속성 공명(챕터 9): 약점 우선(1.5) → 명시 저항(0.5) → 기존 상극/중립 폴백.
 * resistElement === "None"(스테이지 1~80)이면 computeElementMultiplier와 동일(하위호환).
 */
export function computeResonanceMultiplier(
  skillElement: SkillElement,
  weakElement: SkillElement,
  resistElement: SkillElement,
): number {
  if (skillElement !== "None" && skillElement === weakElement) {
    return 1.5;
  }
  if (
    resistElement !== "None" &&
    skillElement !== "None" &&
    skillElement === resistElement
  ) {
    return 0.5;
  }
  return computeElementMultiplier(skillElement, weakElement);
}
```

- [ ] **Step 4: 서버 테스트 추가**

`stage.test.ts`에 약점/저항 단언 추가(기존 약점 테스트 패턴 옆):
```ts
  it("defines chapter 9 weak and resist elements (81~90, weak != resist)", () => {
    const weak: Record<number, string> = {81:"Dark",82:"Fire",83:"Lightning",84:"Holy",85:"Dark",86:"Ice",87:"Dark",88:"Holy",89:"Fire",90:"Dark"};
    const resist: Record<number, string> = {81:"Ice",82:"Holy",83:"Dark",84:"Fire",85:"Holy",86:"Lightning",87:"Fire",88:"Lightning",89:"Holy",90:"Ice"};
    for (let i = 81; i <= 90; i++) {
      expect(getStageWeakElement(i)).toBe(weak[i]);
      expect(getStageResistElement(i)).toBe(resist[i]);
      expect(getStageWeakElement(i)).not.toBe(getStageResistElement(i));
    }
    expect(getStageResistElement(80)).toBe("None");
    expect(getStageResistElement(1)).toBe("None");
  });
```
(import에 `getStageResistElement` 추가.)

`combat.test.ts`에 공명 단언 추가(import에 `computeResonanceMultiplier`):
```ts
  it("computes resonance multipliers (weak/resist/backward-compat)", () => {
    // 약점 우선
    expect(computeResonanceMultiplier("Dark", "Dark", "Ice")).toBe(1.5);
    // 명시 저항
    expect(computeResonanceMultiplier("Ice", "Dark", "Ice")).toBe(0.5);
    // 저항 None → computeElementMultiplier와 동일(하위호환)
    expect(computeResonanceMultiplier("Fire", "Fire", "None")).toBe(1.5);
    expect(computeResonanceMultiplier("Ice", "Fire", "None")).toBe(0.5);
    expect(computeResonanceMultiplier("Lightning", "Fire", "None")).toBe(1);
    // 약점·저항 모두 불일치 → 폴백
    expect(computeResonanceMultiplier("Holy", "Dark", "Ice")).toBe(1.5); // Holy↔Dark 상극보정
  });
```

- [ ] **Step 5: 서버 게이트**

Run:
```powershell
cd "C:\game\idle game\repo\server"; npm test; if ($?) { npx biome check . ../tools/balance-sim }
```
Expected: vitest 전체 통과 + biome clean.

- [ ] **Step 6: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add server/src/core/formulas/stage.ts server/src/core/formulas/combat.ts server/src/core/formulas/stage.test.ts server/src/core/formulas/combat.test.ts
git commit -m @'
챕터 9 + 공명(1) 서버 — 약점/저항 81~90 + computeResonanceMultiplier

stage.ts 약점 81~90 + getStageResistElement(부 저항, 1~80 None), combat.ts
computeResonanceMultiplier(약점 1.5/저항 0.5/None 폴백 하위호환). vitest+biome.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 2: 클라 — 약점/저항 parity + 공명 배율 + 전투 배선 + 회귀

**Files:**
- Modify: `client/.../GameCore/StageFormula.h/.cpp`, `StageService.h/.cpp`
- Modify: `client/.../CharacterSystem/IdleMonster.h`
- Modify: `client/.../CombatSystem/CombatFormulas.h/.cpp`, `SkillComponent.cpp`
- Modify: `client/.../IdleProjectGameModeBase.cpp`
- Modify: `client/.../Tests/CombatTests.cpp`

- [ ] **Step 1: StageFormula 약점 81~90 + GetStageResistElement**

`StageFormula.cpp`의 `GetStageWeakElement` `case 80:` 다음, `default:` 앞에 추가:
```cpp
	case 81: return ESkillElement::Dark;
	case 82: return ESkillElement::Fire;
	case 83: return ESkillElement::Lightning;
	case 84: return ESkillElement::Holy;
	case 85: return ESkillElement::Dark;
	case 86: return ESkillElement::Ice;
	case 87: return ESkillElement::Dark;
	case 88: return ESkillElement::Holy;
	case 89: return ESkillElement::Fire;
	case 90: return ESkillElement::Dark;
```
`GetStageWeakElement` 함수 다음에 추가:
```cpp
ESkillElement FStageFormula::GetStageResistElement(int32 GlobalStageIndex)
{
	switch (FMath::Max(0, GlobalStageIndex))
	{
	case 81: return ESkillElement::Ice;
	case 82: return ESkillElement::Holy;
	case 83: return ESkillElement::Dark;
	case 84: return ESkillElement::Fire;
	case 85: return ESkillElement::Holy;
	case 86: return ESkillElement::Lightning;
	case 87: return ESkillElement::Fire;
	case 88: return ESkillElement::Lightning;
	case 89: return ESkillElement::Holy;
	case 90: return ESkillElement::Ice;
	default: return ESkillElement::None;
	}
}
```
`StageFormula.h`의 `GetStageWeakElement` 선언 다음에 `static ESkillElement GetStageResistElement(int32 GlobalStageIndex);` 추가.

- [ ] **Step 2: StageService TotalChapters 9 + StageInfo.ResistElement**

`StageService.h`: `TotalChapters = 8` → `9`. `StageInfo` 구조체의 `ESkillElement WeakElement = ESkillElement::None;` 다음에 `ESkillElement ResistElement = ESkillElement::None;` 추가.
`StageService.cpp`: `Info.WeakElement = FStageFormula::GetStageWeakElement(Info.GlobalStageIndex);`(145행) 다음에 `Info.ResistElement = FStageFormula::GetStageResistElement(Info.GlobalStageIndex);` 추가.

- [ ] **Step 3: IdleMonster ResistElement**

`IdleMonster.h`: `SetWeakElement`/`GetWeakElement` 인근에 추가:
```cpp
	UFUNCTION(BlueprintCallable, Category = "Idle|Monster")
	void SetResistElement(ESkillElement InResistElement) { ResistElement = InResistElement; }

	UFUNCTION(BlueprintPure, Category = "Idle|Monster")
	ESkillElement GetResistElement() const { return ResistElement; }
```
protected 멤버 `ESkillElement WeakElement = ESkillElement::Fire;` 다음에 `ESkillElement ResistElement = ESkillElement::None;` (UPROPERTY 동일 패턴).

- [ ] **Step 4: CombatFormulas ComputeResonanceMultiplier**

`CombatFormulas.h`의 `ComputeElementMultiplier` 선언 다음에:
```cpp
	static float ComputeResonanceMultiplier(ESkillElement SkillElement, ESkillElement WeakElement, ESkillElement ResistElement);
```
`CombatFormulas.cpp`의 `ComputeElementMultiplier` 함수 다음에:
```cpp
float FCombatFormulas::ComputeResonanceMultiplier(ESkillElement SkillElement, ESkillElement WeakElement, ESkillElement ResistElement)
{
	if (SkillElement != ESkillElement::None && SkillElement == WeakElement)
	{
		return 1.5f;
	}
	if (ResistElement != ESkillElement::None && SkillElement != ESkillElement::None && SkillElement == ResistElement)
	{
		return 0.5f;
	}
	return ComputeElementMultiplier(SkillElement, WeakElement);
}
```

- [ ] **Step 5: SkillComponent 공명 배율 사용**

`SkillComponent.cpp:722-725` 블록을 교체:
```cpp
	const ESkillElement TargetWeakElement = Target && Target->IsA<AIdleMonster>()
		? CastChecked<AIdleMonster>(Target)->GetWeakElement()
		: ESkillElement::None;
	const ESkillElement TargetResistElement = Target && Target->IsA<AIdleMonster>()
		? CastChecked<AIdleMonster>(Target)->GetResistElement()
		: ESkillElement::None;
	const float ElementDamage = BaseDamage * FCombatFormulas::ComputeResonanceMultiplier(Skill.Element, TargetWeakElement, TargetResistElement);
```
(기존 두 줄 선언 형태에 맞춰 변수명/캐스팅 유지. 라인 번호는 근사 — `GetWeakElement()` 사용처를 찾아 교체.)

- [ ] **Step 6: GameMode 몬스터 ResistElement 배선**

`IdleProjectGameModeBase.cpp`의 `Monster->SetWeakElement(StageInfo.WeakElement);`(416행 근방) 다음에:
```cpp
		Monster->SetResistElement(StageInfo.ResistElement);
```

- [ ] **Step 7: CombatTests 공명 회귀**

`CombatTests.cpp`의 기존 ComputeElementMultiplier 단언(157~159행) 인근에 추가:
```cpp
	// 공명: 약점 우선 / 명시 저항 / None 하위호환.
	TestEqual(TEXT("Resonance weak priority"), FCombatFormulas::ComputeResonanceMultiplier(ESkillElement::Dark, ESkillElement::Dark, ESkillElement::Ice), 1.5f);
	TestEqual(TEXT("Resonance explicit resist"), FCombatFormulas::ComputeResonanceMultiplier(ESkillElement::Ice, ESkillElement::Dark, ESkillElement::Ice), 0.5f);
	TestEqual(TEXT("Resonance none == element mult"), FCombatFormulas::ComputeResonanceMultiplier(ESkillElement::Ice, ESkillElement::Fire, ESkillElement::None), 0.5f);
```

- [ ] **Step 8: 클라 게이트(약점/저항/공명/HUD)**

Run:
```powershell
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1 -Filter "IdleProject.Combat+IdleProject.Stage+IdleProject.GameCore+IdleProject.UI.HUD"
```
Expected: `[ue-automation] GREEN.`

- [ ] **Step 9: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add client/Source/IdleProject/GameCore/StageFormula.h client/Source/IdleProject/GameCore/StageFormula.cpp client/Source/IdleProject/GameCore/StageService.h client/Source/IdleProject/GameCore/StageService.cpp client/Source/IdleProject/CharacterSystem/IdleMonster.h client/Source/IdleProject/CombatSystem/CombatFormulas.h client/Source/IdleProject/CombatSystem/CombatFormulas.cpp client/Source/IdleProject/CombatSystem/SkillComponent.cpp client/Source/IdleProject/IdleProjectGameModeBase.cpp client/Source/IdleProject/Tests/CombatTests.cpp
git commit -m @'
챕터 9 + 공명(2) 클라 — TotalChapters 9 + 약점/저항 parity + 공명 전투 배선

StageFormula 약점 81~90/GetStageResistElement, StageInfo·IdleMonster ResistElement,
CombatFormulas ComputeResonanceMultiplier, SkillComponent 공명 배율 사용, GameMode
스폰 배선. 저항 None(1~80) 기존 전투 불변. CombatTests 회귀.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 3: 콘텐츠 — ch9 스토리 + 메인 퀘스트(3중 parity)

**Files:**
- Modify: `server/src/core/data/quests.ts` + `quests.test.ts`
- Modify: 클라 `QuestDB.csv`, `Quest.csv`, `QuestService`(ch8 패턴 위치), `Story.csv`, `StoryText.csv`
- Modify: 클라 `Tests/LocalizationTests`(ch8 게이트 패턴)

- [ ] **Step 1: 기존 ch8 콘텐츠 위치 파악**

`server/src/core/data/quests.ts`에서 `main_ch8_001`~`006` 블록, 클라 `QuestDB.csv`/`Quest.csv`의 ch8 행, `Story.csv`/`StoryText.csv`의 `STORY_C08_*` 키, `LocalizationTests`의 ch8 단언을 Read로 확인(미러 대상).

- [ ] **Step 2: quests.ts main_ch9 6개 추가**

ch8 6 퀘스트를 미러해 `main_ch9_001`~`006` 추가. `main_ch9_001.prerequisiteQuestId = "main_ch8_006"`, 이후 순차 체인. `chapterMapId` `9-1`…`9-10`(ch8과 동일 분포: 001=9-1 kill, 002=9-3 clear, 003=level, 004=tower, 005=9-7 kill, 006=9-10 boss — ch8 실제 값 확인 후 +10 스테이지). 보상은 ch8 대비 글로벌 스케일 비례 증가(ch8 값 패턴 따름).

- [ ] **Step 3: 클라 QuestDB.csv/Quest.csv ch9 6행 + QuestService**

ch8 행을 미러해 ch9 6행 추가(ID/prerequisite/chapterMapId/목표/보상 quests.ts와 1:1). `QuestService`(클라)에 ch8 정의가 코드에 있으면 동일 미러(DefinitionParity 1:1, 59→65).

- [ ] **Step 4: Story.csv/StoryText.csv ch9 ko·en**

`STORY_C09_*` 키(진입/완료/미니보스 9-5/보스 9-10) ko·en 작성. **후존계 계승 서사** — ch8 finale와 자연스럽게 이어지고, **공명(원소 잔향) 모티프**를 도입("후존계의 잔향이 적의 본질을 두 겹으로 비튼다 — 약점과 저항이 함께 깨어난다" 류). ch8 화자/톤 유지. orphan 키 0(ko·en 1:1).

- [ ] **Step 5: LocalizationTests ch9 단언**

ch8 게이트 패턴 미러 — ch9 Story/StoryText/Quest 키 무결성·ko/en 1:1·orphan 0 단언 추가.

- [ ] **Step 6: 서버 quests 게이트**

Run:
```powershell
cd "C:\game\idle game\repo\server"; npm test; if ($?) { npx biome check . ../tools/balance-sim }
```
Expected: vitest(quests/parity 포함) + biome clean. DefinitionParity 1:1.

- [ ] **Step 7: 커밋**

```powershell
cd "C:\game\idle game\repo"
git add server/src/core/data/quests.ts server/src/core/data/quests.test.ts client/Content
git add client/Source/IdleProject  # QuestService/LocalizationTests 변경분
git commit -m @'
챕터 9 + 공명(3) 콘텐츠 — 후존계 계승 스토리 + 메인 퀘스트 main_ch9

main_ch9_001~006(ch8 finale→순차 체인, 9-1…9-10) 서버 quests.ts↔클라
QuestDB/QuestService 3중 DefinitionParity. STORY_C09_* ko·en(공명 모티프).
LocalizationTests ch9 무결성.

Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>
'@
```

---

### Task 4: 전체 게이트 + 무변경 확인 (PM)

**Files:** (검증)

- [ ] **Step 1: 서버 전체 + 클라 전체**

Run:
```powershell
cd "C:\game\idle game\repo\server"; npm test; if ($?) { npx biome check . ../tools/balance-sim }
cd "C:\game\idle game\repo"; .\tools\ci\ue-automation.ps1
```
Expected: 서버 vitest+biome clean, `[ue-automation] GREEN.`(전체 IdleProject).

- [ ] **Step 2: 세이브 무변경 확인**

Run:
```powershell
cd "C:\game\idle game\repo"; git diff origin/main --stat -- client/Source/IdleProject/GameCore/IdleSaveGame.h
```
Expected: 출력 없음(SaveVer 29).

- [ ] **Step 3: PR 발행**

`superpowers:finishing-a-development-branch` 옵션 2(push + PR). 본문에 콘텐츠+기믹·하위호환(저항 None)·세이브 무변경·3중 parity 명시.

---

## Self-Review

- **스펙 커버리지:** 콘텐츠(§4)=Task 3 / 약점·저항(§3-1,§4)=Task 1·2 / 공명 배율(§3-2)=Task 1·2 / 전투 배선(§3-3)=Task 2 Step 3·5·6 / TotalChapters(§5)=Task 2 Step 2 / 세이브 무변경(§2,§7)=Task 4 Step 2 / 하위호환(§3-2,§7)=Task1 Step3·Task2 Step4의 None 폴백. 전부 매핑.
- **플레이스홀더:** 기믹 코드·약점/저항 표 전부 구체. 스토리/퀘스트 prose는 ch8 미러 지시(콘텐츠 슬라이스 표준, #103 동일) — 기계적 ID/prerequisite/mapId는 구체.
- **타입 일관성:** `getStageResistElement`/`GetStageResistElement`, `computeResonanceMultiplier`/`ComputeResonanceMultiplier`, `ResistElement`(StageInfo/IdleMonster) 명명이 서버↔클라↔테스트 일관. 시그니처 `(skill, weak, resist)` 순서 동일.
- **하위호환 가드:** 저항=None(1~80)이면 `computeResonanceMultiplier`가 `computeElementMultiplier`로 폴백 → 기존 CombatTests 단언 불변(회귀 0). Task1 Step4·Task2 Step7이 이를 명시 단언.
