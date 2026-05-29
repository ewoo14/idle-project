# 통합 마스터리 (Unified Mastery) 구현 계획

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.
>
> **프로젝트 주의:** 본 저장소는 PM→Codex 5-team→Claude 리뷰→TM 종합→머지 v3 워크플로우를 따른다. 본 계획은 PR #72 슬라이스의 단일 출처이며, 각 Task 는 Codex 파트(character/backend/designer/balance/qa)로 매핑된다. 모든 산출 문서/주석/커밋 본문은 **한글**, 식별자/`feat:` prefix 는 영문.

**Goal:** 각 시스템을 플레이하면 자동 적립되는 6개 무한 "숙련 트랙"을 추가하고, 그 합인 "월드 파워"가 환생·초월로 초기화되지 않는 영구 전역 보너스를 능력치·드롭·골드·EXP에 부여한다.

**Architecture:** 순수 공식(`FMasteryFormula` C++ / `mastery.ts` 서버 미러)을 단일 출처로 두고, 클라이언트는 `AchievementService` 선례를 따른 `UMasteryService`(GameInstance 소유)에 트랙별 누적 XP를 보관한다. 기존 record 훅(`RecordMonsterKilled` 등)에서 트랙 XP를 적립하고, 전역 코어 스탯 배수는 `AIdleCharacter::RefreshDerivedStats`의 기존 multiplier 곱(현재 transcend×tower×achievement)에 한 항으로 합류시킨다. 마스터리 상태는 세이브 v13으로 영속하되 환생/초월 리셋 대상에서 제외한다.

**Tech Stack:** UE5 5.7.4 C++ (IdleProject 모듈) + UE Automation Test / Node 22 + TypeScript + Vitest / Drizzle 세이브 payload.

**설계 출처:** [`docs/superpowers/specs/2026-05-29-unified-mastery-design.md`](../specs/2026-05-29-unified-mastery-design.md)

---

## 파일 구조 (생성/수정 맵)

**공식 (순수, 단일 출처):**
- Create: `client/Source/IdleProject/GameCore/MasteryFormula.h` / `.cpp`
- Create: `server/src/core/formulas/mastery.ts` (클라 미러)
- Create: `server/src/core/formulas/mastery.test.ts`

**클라이언트 서비스 + 통합:**
- Create: `client/Source/IdleProject/GameCore/MasteryService.h` / `.cpp` (`UMasteryService`)
- Create: `client/Source/IdleProject/GameCore/MasteryTypes.h` (`EMasteryTrack`, `FMasterySaveEntry`, `FMasteryGlobalBonus`)
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h` / `.cpp` (서비스 보유 + getter + XP 적립 + 리셋 제외)
- Modify: `client/Source/IdleProject/CharacterSystem/IdleCharacter.cpp:180-200` (전역 보너스 적용)
- Modify: `client/Source/IdleProject/GameCore/IdleSaveGame.h` (v13 + 마스터리 필드)
- Modify: 세이브 capture/apply 경로 (`IdleGameInstance.cpp` `CaptureToSave`/`ApplyFromSave`)

**테스트:**
- Create: `client/Source/IdleProject/Tests/MasteryTests.cpp` (UE Automation)
- Modify: `client/Source/IdleProject/Tests/SaveSystemTests.cpp` (v12→v13 마이그레이션)

**서버 세이브:**
- Modify: `server/src/modules/save/save.schema.ts` (payload 선택 필드)
- Modify: `server/src/modules/save/save.test.ts`

**UI / 로컬라이즈 / 밸런스:**
- Create: 숙련 패널 위젯 (디자이너, `client/Content/UI/...` + ViewModel getter)
- Modify: ko/en CSV 로컬라이즈 테이블
- Create: `docs/planning/mastery-v1-balance-note.md`

---

## 핵심 상수 / 공식 (단일 출처 — 양쪽 동일값)

```
MasteryTrackCount = 6
EMasteryTrack: Combat=0, Equipment=1, Abyss=2, Rune=3, Beast=4, Explore=5

XP 곡선 (무한, 기하):
  MasteryXpBase   = 100
  MasteryXpGrowth = 1.15
  XpToNext(level) = floor(MasteryXpBase * MasteryXpGrowth^level)   // level>=0
  LevelFromTotalXp(totalXp): level 0부터 XpToNext 누적 차감 → {level, xpIntoLevel, xpToNext}

전역 보너스 (체감 곡선, 무한, 캡 없음):
  CoreStatMultiplier(combatLv, equipLv, exploreLv):
    sum = combatLv + equipLv + exploreLv
    return 1 + 0.02 * ln(1 + sum)            // 코어 스탯(HP/공/방) 배수
  CritRateAdd(runeLv)   = 0.01 * ln(1 + runeLv)   // 크리티컬률(0~1 스케일) 가산
  DropRateAdd(abyssLv)  = 0.01 * ln(1 + abyssLv)  // 드롭 확률 가산
  GoldFindPct(beastLv)  = 0.02 * ln(1 + beastLv)  // 골드 획득 % (0.02=2%)
  ExpBoostPct(beastLv)  = 0.02 * ln(1 + beastLv)  // EXP 획득 %
  WorldPower = combatLv+equipLv+abyssLv+runeLv+beastLv+exploreLv   // 표시용 합

트랙별 XP 적립량(활동 1회당):
  Combat   : 처치 1회 +1,  스테이지 보스 처치 +20
  Equipment: 강화 시도 +5,  잠재 재감정 +3,  세트 완성 +50
  Abyss    : 던전 클리어 +30,  탑 1층 등반 +10
  Rune     : 룬 합성/구매 +5,  룬 장착 +2
  Beast    : 펫 먹이 +5,  펫 진화 +40
  Explore  : 퀘스트 완료 +20,  업적 달성 +30
```

서버 미러는 C++ float 경계를 맞추기 위해 `Math.fround`를 transcend.ts 선례대로 사용한다(곱/가산 경계). `ln`은 `Math.log` 사용.

---

## Task 1: 서버 공식 미러 + 테스트 (backend, 먼저 — 순수 검증 용이)

**Files:**
- Create: `server/src/core/formulas/mastery.ts`
- Create: `server/src/core/formulas/mastery.test.ts`
- Modify: `server/src/core/formulas/index.ts` (export 추가)

- [ ] **Step 1: 실패 테스트 작성**

`server/src/core/formulas/mastery.test.ts`:

```ts
import { describe, it, expect } from "vitest";
import {
  MASTERY_TRACK_COUNT,
  MASTERY_XP_BASE,
  MASTERY_XP_GROWTH,
  xpToNext,
  levelFromTotalXp,
  coreStatMultiplier,
  critRateAdd,
  dropRateAdd,
  goldFindPct,
  expBoostPct,
  worldPower,
} from "./mastery.js";

describe("mastery formula", () => {
  it("트랙 수는 6", () => {
    expect(MASTERY_TRACK_COUNT).toBe(6);
  });

  it("XpToNext는 기하 증가", () => {
    expect(xpToNext(0)).toBe(MASTERY_XP_BASE);
    expect(xpToNext(1)).toBe(Math.floor(MASTERY_XP_BASE * MASTERY_XP_GROWTH));
    expect(xpToNext(10)).toBeGreaterThan(xpToNext(9));
  });

  it("totalXp 0이면 레벨 0", () => {
    const r = levelFromTotalXp(0);
    expect(r.level).toBe(0);
    expect(r.xpIntoLevel).toBe(0);
    expect(r.xpToNext).toBe(MASTERY_XP_BASE);
  });

  it("정확히 한 레벨치 XP면 레벨 1", () => {
    const r = levelFromTotalXp(MASTERY_XP_BASE);
    expect(r.level).toBe(1);
    expect(r.xpIntoLevel).toBe(0);
  });

  it("레벨은 totalXp에 단조 증가", () => {
    expect(levelFromTotalXp(100000).level).toBeGreaterThan(
      levelFromTotalXp(1000).level,
    );
  });

  it("CoreStatMultiplier는 1 이상이며 레벨 합에 단조 증가", () => {
    expect(coreStatMultiplier(0, 0, 0)).toBe(1);
    expect(coreStatMultiplier(10, 10, 10)).toBeGreaterThan(
      coreStatMultiplier(1, 1, 1),
    );
  });

  it("CritRate/Drop/Gold/Exp 보너스는 0레벨에서 0, 양수 레벨에서 증가", () => {
    expect(critRateAdd(0)).toBe(0);
    expect(dropRateAdd(0)).toBe(0);
    expect(goldFindPct(0)).toBe(0);
    expect(expBoostPct(0)).toBe(0);
    expect(critRateAdd(50)).toBeGreaterThan(0);
    expect(goldFindPct(50)).toBeGreaterThan(goldFindPct(5));
  });

  it("WorldPower는 전 트랙 레벨 합", () => {
    expect(worldPower([1, 2, 3, 4, 5, 6])).toBe(21);
  });
});
```

- [ ] **Step 2: 실패 확인**

Run: `cd server; npm run test -- mastery`
Expected: FAIL (`Cannot find module './mastery.js'`)

- [ ] **Step 3: 구현**

`server/src/core/formulas/mastery.ts`:

```ts
export const MASTERY_TRACK_COUNT = 6;
export const MASTERY_XP_BASE = 100;
export const MASTERY_XP_GROWTH = 1.15;

export function xpToNext(level: number): number {
  const safe = Math.max(0, Math.floor(level));
  return Math.floor(MASTERY_XP_BASE * Math.pow(MASTERY_XP_GROWTH, safe));
}

export interface MasteryLevelInfo {
  level: number;
  xpIntoLevel: number;
  xpToNext: number;
}

export function levelFromTotalXp(totalXp: number): MasteryLevelInfo {
  let remaining = Math.max(0, Math.floor(totalXp));
  let level = 0;
  let need = xpToNext(0);
  while (remaining >= need) {
    remaining -= need;
    level += 1;
    need = xpToNext(level);
  }
  return { level, xpIntoLevel: remaining, xpToNext: need };
}

export function coreStatMultiplier(
  combatLv: number,
  equipLv: number,
  exploreLv: number,
): number {
  const sum = Math.max(0, combatLv) + Math.max(0, equipLv) + Math.max(0, exploreLv);
  return Math.fround(1 + 0.02 * Math.log(1 + sum));
}

export function critRateAdd(runeLv: number): number {
  return Math.fround(0.01 * Math.log(1 + Math.max(0, runeLv)));
}

export function dropRateAdd(abyssLv: number): number {
  return Math.fround(0.01 * Math.log(1 + Math.max(0, abyssLv)));
}

export function goldFindPct(beastLv: number): number {
  return Math.fround(0.02 * Math.log(1 + Math.max(0, beastLv)));
}

export function expBoostPct(beastLv: number): number {
  return Math.fround(0.02 * Math.log(1 + Math.max(0, beastLv)));
}

export function worldPower(trackLevels: number[]): number {
  return trackLevels.reduce((acc, lv) => acc + Math.max(0, Math.floor(lv)), 0);
}
```

`server/src/core/formulas/index.ts` 에 한 줄 추가:

```ts
export * from "./mastery.js";
```

- [ ] **Step 4: 통과 확인**

Run: `cd server; npm run test -- mastery`
Expected: PASS (8 tests)

- [ ] **Step 5: 커밋**

```bash
git add server/src/core/formulas/mastery.ts server/src/core/formulas/mastery.test.ts server/src/core/formulas/index.ts
git commit -F - <<'EOF'
feat: 마스터리 공식 서버 미러 (PR #72)

무한 XP 곡선/레벨 환산 + 트랙별 전역 보너스(코어배수/크리/드롭/골드/EXP) + 월드파워 합산. Vitest 8케이스.
EOF
```

---

## Task 2: 클라이언트 공식 `FMasteryFormula` + Automation (character 메인)

**Files:**
- Create: `client/Source/IdleProject/GameCore/MasteryTypes.h`
- Create: `client/Source/IdleProject/GameCore/MasteryFormula.h` / `.cpp`
- Create: `client/Source/IdleProject/Tests/MasteryTests.cpp`

- [ ] **Step 1: 타입 정의 작성**

`MasteryTypes.h`:

```cpp
#pragma once
#include "CoreMinimal.h"
#include "MasteryTypes.generated.h"

UENUM(BlueprintType)
enum class EMasteryTrack : uint8
{
    Combat = 0,
    Equipment = 1,
    Abyss = 2,
    Rune = 3,
    Beast = 4,
    Explore = 5
};

/** 세이브에 직렬화되는 트랙별 누적 XP. */
USTRUCT()
struct IDLEPROJECT_API FMasterySaveEntry
{
    GENERATED_BODY()

    UPROPERTY()
    uint8 Track = 0;

    UPROPERTY()
    int64 TotalXp = 0;
};

/** RefreshDerivedStats 및 보상 경로에 적용할 전역 보너스 묶음. */
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FMasteryGlobalBonus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    float CoreStatMultiplier = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    float CritRateAdd = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    float DropRateAdd = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    float GoldFindPct = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    float ExpBoostPct = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Idle|Mastery")
    int64 WorldPower = 0;
};
```

- [ ] **Step 2: 실패 Automation 작성**

`MasteryTests.cpp` (UE Automation, `transcend`/`RebirthTests.cpp` 스타일):

```cpp
#include "Misc/AutomationTest.h"
#include "GameCore/MasteryFormula.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMasteryFormulaTest,
    "IdleProject.Mastery.Formula",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryFormulaTest::RunTest(const FString&)
{
    TestEqual(TEXT("XpToNext(0)"), FMasteryFormula::XpToNext(0), (int64)100);
    TestTrue(TEXT("XpToNext 증가"), FMasteryFormula::XpToNext(10) > FMasteryFormula::XpToNext(9));

    int32 Level = -1; int64 Into = -1; int64 Need = -1;
    FMasteryFormula::LevelFromTotalXp(0, Level, Into, Need);
    TestEqual(TEXT("0XP→레벨0"), Level, 0);

    FMasteryFormula::LevelFromTotalXp(100, Level, Into, Need);
    TestEqual(TEXT("100XP→레벨1"), Level, 1);

    TestEqual(TEXT("코어배수 0레벨=1"), FMasteryFormula::CoreStatMultiplier(0,0,0), 1.0f);
    TestTrue(TEXT("코어배수 단조"), FMasteryFormula::CoreStatMultiplier(10,10,10) > FMasteryFormula::CoreStatMultiplier(1,1,1));
    TestEqual(TEXT("크리 0레벨=0"), FMasteryFormula::CritRateAdd(0), 0.0f);
    TestTrue(TEXT("골드 단조"), FMasteryFormula::GoldFindPct(50) > FMasteryFormula::GoldFindPct(5));
    return true;
}
```

- [ ] **Step 3: 실패 확인**

Run (헤드리스 빌드/테스트는 [[reference-ue-headless-verify]] 절차):
`Engine\Build\BatchFiles\RunUAT.bat ... -RunAutomationTest="IdleProject.Mastery"`
Expected: FAIL (`MasteryFormula.h` 없음 → 빌드 실패)

- [ ] **Step 4: 구현**

`MasteryFormula.h`:

```cpp
#pragma once
#include "CoreMinimal.h"

struct IDLEPROJECT_API FMasteryFormula
{
    static constexpr int32 TrackCount = 6;
    static constexpr int64 XpBase = 100;
    static constexpr float XpGrowth = 1.15f;

    static int64 XpToNext(int32 Level);
    static void LevelFromTotalXp(int64 TotalXp, int32& OutLevel, int64& OutXpIntoLevel, int64& OutXpToNext);
    static float CoreStatMultiplier(int32 CombatLv, int32 EquipLv, int32 ExploreLv);
    static float CritRateAdd(int32 RuneLv);
    static float DropRateAdd(int32 AbyssLv);
    static float GoldFindPct(int32 BeastLv);
    static float ExpBoostPct(int32 BeastLv);
};
```

`MasteryFormula.cpp` (서버 `mastery.ts`와 동일 수식; `FMath::Pow`, `FMath::Loge`, `FMath::FloorToInt`):

```cpp
#include "GameCore/MasteryFormula.h"

int64 FMasteryFormula::XpToNext(int32 Level)
{
    const int32 Safe = FMath::Max(0, Level);
    return (int64)FMath::FloorToDouble((double)XpBase * FMath::Pow((double)XpGrowth, (double)Safe));
}

void FMasteryFormula::LevelFromTotalXp(int64 TotalXp, int32& OutLevel, int64& OutXpIntoLevel, int64& OutXpToNext)
{
    int64 Remaining = FMath::Max((int64)0, TotalXp);
    int32 Level = 0;
    int64 Need = XpToNext(0);
    while (Remaining >= Need)
    {
        Remaining -= Need;
        ++Level;
        Need = XpToNext(Level);
    }
    OutLevel = Level;
    OutXpIntoLevel = Remaining;
    OutXpToNext = Need;
}

float FMasteryFormula::CoreStatMultiplier(int32 CombatLv, int32 EquipLv, int32 ExploreLv)
{
    const int32 Sum = FMath::Max(0, CombatLv) + FMath::Max(0, EquipLv) + FMath::Max(0, ExploreLv);
    return 1.0f + 0.02f * FMath::Loge(1.0f + (float)Sum);
}

float FMasteryFormula::CritRateAdd(int32 RuneLv) { return 0.01f * FMath::Loge(1.0f + (float)FMath::Max(0, RuneLv)); }
float FMasteryFormula::DropRateAdd(int32 AbyssLv) { return 0.01f * FMath::Loge(1.0f + (float)FMath::Max(0, AbyssLv)); }
float FMasteryFormula::GoldFindPct(int32 BeastLv) { return 0.02f * FMath::Loge(1.0f + (float)FMath::Max(0, BeastLv)); }
float FMasteryFormula::ExpBoostPct(int32 BeastLv) { return 0.02f * FMath::Loge(1.0f + (float)FMath::Max(0, BeastLv)); }
```

- [ ] **Step 5: 통과 확인 + 커밋**

Run: `IdleProject.Mastery.Formula` Automation GREEN.

```bash
git add client/Source/IdleProject/GameCore/MasteryTypes.h client/Source/IdleProject/GameCore/MasteryFormula.h client/Source/IdleProject/GameCore/MasteryFormula.cpp client/Source/IdleProject/Tests/MasteryTests.cpp
git commit -F - <<'EOF'
feat: FMasteryFormula 클라 공식 + Automation (PR #72)

서버 mastery.ts 미러. 무한 XP/레벨 환산 + 전역 보너스. IdleProject.Mastery.Formula GREEN.
EOF
```

---

## Task 3: `UMasteryService` — 트랙 누적 XP 보관 + 전역 보너스 산출 (character 메인)

`AchievementService`(점수→`GetAchievementStatMultiplier`) 패턴을 그대로 따른다.

**Files:**
- Create: `client/Source/IdleProject/GameCore/MasteryService.h` / `.cpp`
- Modify: `client/Source/IdleProject/Tests/MasteryTests.cpp` (서비스 테스트 추가)

- [ ] **Step 1: 실패 테스트 추가**

`MasteryTests.cpp` 에 추가:

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMasteryServiceTest,
    "IdleProject.Mastery.Service",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMasteryServiceTest::RunTest(const FString&)
{
    UMasteryService* S = NewObject<UMasteryService>();
    S->Initialize();

    TestEqual(TEXT("초기 레벨0"), S->GetTrackLevel(EMasteryTrack::Combat), 0);
    S->AddXp(EMasteryTrack::Combat, 250);                  // 100+115=215 < 250 → 레벨2
    TestEqual(TEXT("250XP→전투 레벨2"), S->GetTrackLevel(EMasteryTrack::Combat), 2);

    S->AddXp(EMasteryTrack::Beast, 1000);
    const FMasteryGlobalBonus B = S->GetGlobalBonus();
    TestTrue(TEXT("골드보너스>0"), B.GoldFindPct > 0.0f);
    TestTrue(TEXT("월드파워=레벨합"), B.WorldPower == S->GetWorldPower());
    return true;
}
```

- [ ] **Step 2: 실패 확인** — 빌드 실패(`UMasteryService` 없음).

- [ ] **Step 3: 구현**

`MasteryService.h`:

```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameCore/MasteryTypes.h"
#include "MasteryService.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMasteryTrackLevelUp, EMasteryTrack, Track, int32, NewLevel);

UCLASS()
class IDLEPROJECT_API UMasteryService : public UObject
{
    GENERATED_BODY()
public:
    void Initialize();

    UFUNCTION(BlueprintCallable, Category = "Idle|Mastery")
    void AddXp(EMasteryTrack Track, int64 Amount);

    UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
    int32 GetTrackLevel(EMasteryTrack Track) const;

    UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
    int64 GetTrackTotalXp(EMasteryTrack Track) const;

    UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
    int64 GetWorldPower() const;

    UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
    FMasteryGlobalBonus GetGlobalBonus() const;

    // 세이브 직렬화
    TArray<FMasterySaveEntry> ExportSave() const;
    void ImportSave(const TArray<FMasterySaveEntry>& Entries);

    UPROPERTY(BlueprintAssignable, Category = "Idle|Mastery")
    FOnMasteryTrackLevelUp OnTrackLevelUp;

private:
    int64 TrackXp[6] = {0,0,0,0,0,0};
    int32 LevelOf(EMasteryTrack Track) const;
};
```

`MasteryService.cpp` 핵심: `AddXp`는 적립 전후 레벨을 비교해 상승 시 `OnTrackLevelUp` 브로드캐스트, `GetGlobalBonus`는 트랙 레벨을 모아 `FMasteryFormula`로 보너스 묶음 산출, `GetWorldPower`는 6트랙 레벨 합. (`Initialize`는 `TrackXp`를 0으로.)

- [ ] **Step 4: 통과 확인 + 커밋**

```bash
git add client/Source/IdleProject/GameCore/MasteryService.h client/Source/IdleProject/GameCore/MasteryService.cpp client/Source/IdleProject/Tests/MasteryTests.cpp
git commit -F - <<'EOF'
feat: UMasteryService 트랙 XP 보관/전역보너스 (PR #72)

AchievementService 선례 패턴. AddXp/레벨환산/월드파워/GetGlobalBonus + 레벨업 델리게이트. IdleProject.Mastery.Service GREEN.
EOF
```

---

## Task 4: GameInstance 연결 — 서비스 보유 + XP 적립 훅 (character 메인)

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.h` (멤버/getter/Ensure 선언)
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` (생성 + record 훅에 적립)

- [ ] **Step 1: 선언 추가** — `IdleGameInstance.h`

`GetAchievementService()` 인근에 getter, private 멤버, `EnsureMasteryService()` 선언 추가:

```cpp
// public:
UFUNCTION(BlueprintPure, Category = "Idle|Services")
UMasteryService* GetMasteryService() const { return MasteryService; }

UFUNCTION(BlueprintPure, Category = "Idle|Mastery")
float GetMasteryCoreStatMultiplier() const;   // RefreshDerivedStats용

// private:
UPROPERTY(Transient)
TObjectPtr<UMasteryService> MasteryService;
void EnsureMasteryService();
```

`#include "GameCore/MasteryService.h"` 추가, 전방선언 `class UMasteryService;`.

- [ ] **Step 2: XP 적립 훅 삽입** — `IdleGameInstance.cpp`

기존 record 메서드 본문 끝에 적립 한 줄씩 추가 (서비스 null 가드):

| 기존 메서드 | 추가 |
| --- | --- |
| `RecordMonsterKilled()` | `if (MasteryService) MasteryService->AddXp(EMasteryTrack::Combat, 1);` |
| `RecordGearEnhanced()` | `... AddXp(EMasteryTrack::Equipment, 5);` |
| `TryRunDungeon()` 성공 분기 | `... AddXp(EMasteryTrack::Abyss, 30);` |
| `ClimbTower()` 성공 분기 | `... AddXp(EMasteryTrack::Abyss, 10);` |
| `TryFeedPet()` 성공 분기 | `... AddXp(EMasteryTrack::Beast, 5);` |
| `TryEnhanceRune`/`AddRune` | `... AddXp(EMasteryTrack::Rune, 5);` |
| `ClaimQuest()` 성공 분기 | `... AddXp(EMasteryTrack::Explore, 20);` |

`Init()`에서 `EnsureMasteryService()` 호출. `GetMasteryCoreStatMultiplier()`는 `MasteryService ? MasteryService->GetGlobalBonus().CoreStatMultiplier : 1.0f`. XP 적립 후 `RefreshPlayerCharacterStats()` 호출로 즉시 반영.

- [ ] **Step 3: 적립 검증 테스트** — `MasteryTests.cpp` 에 GameInstance 통합 테스트(처치 N회 → Combat XP 증가) 추가. `RecordMonsterKilled` 호출 → `GetMasteryService()->GetTrackTotalXp(Combat)` 증가 확인.

- [ ] **Step 4: 빌드/Automation GREEN + 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleGameInstance.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/Tests/MasteryTests.cpp
git commit -F - <<'EOF'
feat: GameInstance 마스터리 서비스 연결 + XP 적립 훅 (PR #72)

처치/강화/던전/탑/펫/룬/퀘스트 record 훅에서 트랙 XP 자동 적립. EnsureMasteryService.
EOF
```

---

## Task 5: 전역 보너스 능력치 적용 — RefreshDerivedStats (character 메인)

**Files:**
- Modify: `client/Source/IdleProject/CharacterSystem/IdleCharacter.cpp:180-201`

- [ ] **Step 1: 실패 테스트** — `MasteryTests.cpp`

Combat/Equipment 레벨 상승 후 캐릭터 `GetCurrentDerivedStats()`의 PhysAtk/Hp가 마스터리 0일 때보다 큼(단조 증가), 마스터리 0이면 회귀 없음을 검증하는 Automation 추가. CP(`GetCombatPower()`)도 증가.

- [ ] **Step 2: 적용 코드 삽입** — `IdleCharacter.cpp:182~188` 수정

기존:
```cpp
const float AchievementMultiplier = IdleGameInstance ? IdleGameInstance->GetAchievementStatMultiplier() : 1.0f;
const float StatMultiplier = TranscendMultiplier * TowerMultiplier * AchievementMultiplier;
```
변경:
```cpp
const float AchievementMultiplier = IdleGameInstance ? IdleGameInstance->GetAchievementStatMultiplier() : 1.0f;
const float MasteryCoreMultiplier = IdleGameInstance ? IdleGameInstance->GetMasteryCoreStatMultiplier() : 1.0f;
const float StatMultiplier = TranscendMultiplier * TowerMultiplier * AchievementMultiplier * MasteryCoreMultiplier;
```

룬 util(crit) 적용부(`:200`) 인근에 마스터리 크리 가산 추가:
```cpp
if (const UMasteryService* MasterySvc = IdleGameInstance->GetMasteryService())
{
    Derived.CritRate += MasterySvc->GetGlobalBonus().CritRateAdd;
}
```

> **이중 계산 방지(설계 §4):** 마스터리는 별도 *곱* 항(코어) + 별도 *가산* 항(크리)로만 들어가며 transcend/achievement 항을 건드리지 않는다.

- [ ] **Step 3: GREEN + 커밋**

```bash
git add client/Source/IdleProject/CharacterSystem/IdleCharacter.cpp client/Source/IdleProject/Tests/MasteryTests.cpp
git commit -F - <<'EOF'
feat: 마스터리 전역 보너스 능력치 적용 (PR #72)

RefreshDerivedStats 단일 지점에 코어 배수(곱)+크리(가산) 합류. transcend/tower/achievement와 직교. CP 자동 반영.
EOF
```

---

## Task 6: 경제 보너스 적용 — 드롭/골드/EXP (character 메인)

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` (골드/EXP/드롭 경로)

- [ ] **Step 1: 실패 테스트** — Beast 레벨↑ 후 `AddGold`/`AddExp` 실효 획득량 증가, Abyss 레벨↑ 후 드롭 확률 증가 검증(Automation). 펫 보너스 적용부와 동일 경로에 마스터리 항을 추가.

- [ ] **Step 2: 구현** — 펫 골드/드롭 적용 선례(`ApplyEquippedPetGoldBonus`, `ApplyEquippedPetDropBonusChance`)와 동일 위치에서 `GetGlobalBonus().GoldFindPct`/`ExpBoostPct`/`DropRateAdd`를 곱/가산. `AddExp` 진입 시 `1 + ExpBoostPct` 배율.

- [ ] **Step 3: GREEN + 커밋** (`feat: 마스터리 드롭/골드/EXP 경제 보너스 (PR #72)`)

---

## Task 7: 세이브 v13 영속 + 리셋 제외 (character 메인 + backend)

**Files:**
- Modify: `client/Source/IdleProject/GameCore/IdleSaveGame.h` (v13 + 필드)
- Modify: `client/Source/IdleProject/GameCore/IdleGameInstance.cpp` (`CaptureToSave`/`ApplyFromSave`, 환생/초월 리셋 제외)
- Modify: `client/Source/IdleProject/Tests/SaveSystemTests.cpp`

- [ ] **Step 1: 실패 테스트** — `SaveSystemTests.cpp` 에:
  - v13 저장→로드 라운드트립으로 트랙 XP 보존.
  - **v12 세이브(마스터리 필드 없음) 로드 시 전 트랙 0으로 안전 마이그레이션**.
  - `Rebirth()` / `Transcend()` 호출 후 마스터리 XP **불변**(리셋 생존).

- [ ] **Step 2: 구현** — `IdleSaveGame.h`:
```cpp
UPROPERTY()
int32 SaveVersion = 13;          // 12 → 13

UPROPERTY()
TArray<FMasterySaveEntry> Mastery;
```
`CaptureToSave`: `SaveGame->Mastery = MasteryService->ExportSave();`
`ApplyFromSave`: `MasteryService->ImportSave(SaveGame->Mastery);` (빈 배열이면 0 유지 → v12 마이그레이션 자동 충족).
`Rebirth()`/`Transcend()` 리셋 블록: 마스터리 관련 초기화 **추가하지 않음**(명시적으로 제외 — 주석으로 표기).

- [ ] **Step 3: GREEN + 커밋**

```bash
git add client/Source/IdleProject/GameCore/IdleSaveGame.h client/Source/IdleProject/GameCore/IdleGameInstance.cpp client/Source/IdleProject/Tests/SaveSystemTests.cpp
git commit -F - <<'EOF'
feat: 마스터리 세이브 v13 영속 + 리셋 제외 (PR #72)

트랙 XP 직렬화, v12→v13 안전 마이그레이션(누락=0), 환생/초월 리셋 대상 제외(영구). 라운드트립/마이그레이션/리셋생존 Automation.
EOF
```

---

## Task 8: 서버 세이브 payload 수용 + 테스트 (backend)

**Files:**
- Modify: `server/src/modules/save/save.schema.ts`
- Modify: `server/src/modules/save/save.test.ts`

- [ ] **Step 1: 실패 테스트** — `save.test.ts` 에 `worldPower`/`masteryLevels` 포함 payload PUT→GET 라운드트립 통과 케이스 추가. (`payload`는 `additionalProperties:true`라 통과해야 하나, 검증 필드 추가로 명시화.)

- [ ] **Step 2: 구현** — `putSaveSchema.body.payload.properties` 에 선택 필드 추가:
```ts
worldPower: { type: "integer", minimum: 0 },
masteryLevels: {
  type: "array",
  items: { type: "integer", minimum: 0 },
  minItems: 0,
  maxItems: 6,
},
```
(required에는 넣지 않음 — v12 호환.)

- [ ] **Step 3: GREEN + 커밋** (`cd server; npm run test -- save`)

```bash
git add server/src/modules/save/save.schema.ts server/src/modules/save/save.test.ts
git commit -F - <<'EOF'
feat: 세이브 payload 마스터리 필드 수용 (PR #72)

worldPower/masteryLevels 선택 필드 명시(미필수=v12 호환). 라운드트립 Vitest.
EOF
```

---

## Task 9: 숙련 UI 패널 + 로컬라이즈 (designer)

**Files:**
- Create: 숙련 패널 위젯 (`client/Content/UI/...`) — 트랙 6행(아이콘/명칭/레벨/XP바/현재 로컬·전역 보너스), 헤더에 월드 파워.
- Modify: HUD 헤더에 월드 파워 표시(전투력 CP 옆).
- Modify: ko/en 로컬라이즈 CSV (트랙명·툴팁 키).

- [ ] **Step 1:** ViewModel getter 노출 — `GetMasteryService()->GetTrackLevel/GetTrackTotalXp/GetWorldPower/GetGlobalBonus` 바인딩. (Task 3에서 BlueprintPure로 이미 노출됨.)
- [ ] **Step 2:** 트랙별 XP바 = `xpIntoLevel / xpToNext`. 큰 수 천단위 콤마.
- [ ] **Step 3:** ko/en CSV 키 추가 후 `CsvIntegrity` Automation으로 키 정합 검증(기존 절차).
- [ ] **Step 4:** 커밋 (`feat: 숙련 패널 + 월드파워 HUD + ko/en (PR #72)`)

---

## Task 10: 밸런스 문서 + 곡선 검증 (balance)

**Files:**
- Create: `docs/planning/mastery-v1-balance-note.md`

- [ ] **Step 1:** XP 곡선/보너스 계수 근거 + 초기(트랙 Lv5)/중반(Lv30)/엔드(Lv100) 단계별 누적 XP·코어 배수·골드/드롭/크리 보너스 예시 표 작성.
- [ ] **Step 2:** transcend/achievement와의 합성 시 인플레 점검(엔드 기준 마스터리 코어 배수 ≈ `1+0.02·ln(1+300)` ≈ 1.11 수준 — 폭주 없음 명시).
- [ ] **Step 3:** 커밋 (`docs: 마스터리 V1 밸런스 노트 (PR #72)`)

---

## Task 11: E2E / 회귀 QA 시나리오 (qa)

**Files:**
- Modify/Create: QA 시나리오 + Automation 통합 케이스

- [ ] **Step 1:** 성장 루프 E2E — 각 시스템 활동 → 해당 트랙 XP↑ → 레벨업 델리게이트 → 월드 파워↑ → 능력치/CP↑.
- [ ] **Step 2:** 리셋 생존 — 환생·초월 후 마스터리 XP 불변 + 전역 보너스 유지.
- [ ] **Step 3:** 회귀 — 마스터리 0(신규/v12 로드) 시 기존 전투/스탯/CP 회귀 없음.
- [ ] **Step 4:** 서버↔클라 parity — 동일 트랙 레벨 입력 시 `mastery.ts`와 `FMasteryFormula` 보너스 일치(경계값 포함).
- [ ] **Step 5:** 커밋 (`test: 마스터리 E2E/회귀/parity 시나리오 (PR #72)`)

---

## Self-Review (작성자 체크)

**1. 스펙 커버리지:**
- §3 6트랙 → Task 3(트랙)·Task 4(XP 소스 매핑) ✅
- §3 전역 매핑(전투/장비/탐험→코어, 룬→크리, 심연→드롭, 야성→골드·EXP) → Task 5(코어·크리)·Task 6(드롭·골드·EXP) ✅
- §4 단일 지점 적용 + 초월 이중계산 방지 → Task 5 (별도 곱/가산 항) ✅
- §5 무한 곡선 → Task 1/2 공식 + Task 10 문서 ✅
- §6 세이브 v13 + 리셋 제외 → Task 7 ✅, 서버 미러/parity → Task 1·Task 11 ✅
- §6 UI → Task 9 ✅
- §7 V1 스코프(트랙당 로컬 보너스 1종): 본 계획 V1은 **전역 보너스 우선** 구현. 트랙 *로컬* 보너스(강화 성공률 보정 등)는 스펙상 "트랙당 1종"이나 시스템별 삽입점이 상이해 범위가 큼 → **Task 12로 분리(아래), V1 필수 아님으로 표기**.

**2. Placeholder 스캔:** TBD/TODO 없음. 공식 코드는 실제 구현 포함. C++ 통합부는 확인된 실제 심볼(`RefreshDerivedStats:180`, `GetAchievementStatMultiplier`, record 훅)을 참조.

**3. 타입 일관성:** `EMasteryTrack`, `FMasteryGlobalBonus`, `FMasterySaveEntry`, `FMasteryFormula::*`, `UMasteryService::*` 명칭이 Task 2~7 전반에서 동일. 서버 함수명(`coreStatMultiplier` 등)과 C++(`CoreStatMultiplier`)은 언어 컨벤션 차이로 의도적 일치.

---

## Task 12 (선택, V1.5): 트랙별 로컬 보너스 1종

> 스펙 §7의 "트랙당 로컬 보너스 1종"을 별도 태스크로 분리. V1 전역 보너스 안정화 후 진행. 예: 장비 숙련→강화 성공률 +보정(상한 클램프), 심연 숙련→던전 입장 +1, 야성 숙련→펫 효과 배수. 각 시스템 공식에 마스터리 레벨 인자를 추가하는 형태(개별 PR로 분할 권장).

---

## 워크플로우 v3 매핑

| Task | Codex 파트 |
| --- | --- |
| 1, 8 | backend |
| 2~7 | character (메인) |
| 9 | designer |
| 10 | balance |
| 11 | qa |

[1] PM 기획+PR → [2] Codex 5-team → [3] Claude TM 종합 → [4] Codex TM+fix → [5] Claude 검증 → [N] CI 그린 + PM 종합 소견 + 머지([[feedback-ci-before-merge]]).
