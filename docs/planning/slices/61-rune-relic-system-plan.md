# 룬/유물 장착 시스템 구현 계획 — PR #61

> **실행 방식:** 본 계획은 멀티 에이전트 워크플로우 v3 (Codex 디스패치 + Claude 리뷰 ping-pong)의 **[2] Codex 작업 명세**로 사용한다. superpowers subagent 흐름이 아니라 프로젝트 워크플로우 v3 ([1]~[N])를 따른다. 아래 인터페이스 계약·테스트 앵커는 Codex(character 메인 + backend/designer/balance/qa 보조)가 일괄 구현하고, parity·정합은 [3] Claude TM 리뷰에서 검증한다.

**목표:** 장비와 독립된 룬 슬롯 6칸 + 룬 무한 강화로 무한 성장(코어 % 곱)과 빌드 다양성(코어/유틸 혼합)을 추가한다.

**아키텍처:** 코어 룬은 스탯별 % 무한 곱 → `RefreshDerivedStats`의 기존 `StatMultiplier` 곱 뒤에 스탯별 추가 곱으로 합류. 유틸 룬은 flat 가산 + 타입별 캡 → CritDmg는 파생스탯 가산, GoldFind/ExpBoost/OfflineEff는 보상/오프라인 공식에서 조회. 드롭+분해 → 룬 에센스(전용 재화), 강화는 에센스+골드(기하급수) 소비.

**기술 스택:** UE5 C++ (client) + TypeScript/Fastify/vitest (server). 클라↔서버 공식 `Math.fround` parity. 기존 `EItemRarity`/U…Service/USaveGame 패턴 재사용.

---

## 파일 구조

### 생성 (client)
- `client/Source/IdleProject/RuneSystem/RuneTypes.h` — `ERuneType`, `FRuneInstance`, `FRuneSaveEntry`, `FRuneCoreMultipliers`, `FRuneUtilValues`
- `client/Source/IdleProject/RuneSystem/RuneFormula.h` / `.cpp` — `FRuneFormula` (공식)
- `client/Source/IdleProject/RuneSystem/RuneService.h` / `.cpp` — `URuneService`
- `client/Source/IdleProject/Tests/RuneFormulaTests.cpp` — 공식 Automation
- `client/Source/IdleProject/Tests/RuneServiceTests.cpp` — 서비스/장착/강화/분해/저장 Automation
- `client/Content/Localization/Game/ko/Rune.csv` / `en/Rune.csv`

### 수정 (client)
- `GameCore/IdleSaveGame.h` — SaveVersion 2→3 + 룬 필드 3개
- `GameCore/IdleGameInstance.h` / `.cpp` — `RuneService` 멤버/getter/`EnsureRuneService`/Shutdown + Capture/Apply + reward/offline/CritDmg 훅 + 골드 게이트 API
- `CharacterSystem/IdleCharacter.cpp` — `RefreshDerivedStats` 룬 합성 (라인 181-185 직후)
- `GameCore/RewardFormula.cpp` 또는 적용부 — GoldFind/ExpBoost 조회 곱 (GameInstance 적용 시점)
- `GameCore/OfflineRewardFormula.*` 적용부 — OfflineEff 조회 곱
- `UI/IdleHUD.h` / `.cpp` — `FIdleHUDRuneViewModel` + 룬 패널
- `client/Source/IdleProject/IdleProject.Build.cs` — (필요시 모듈 경로, 보통 자동)

### 생성 (server)
- `server/src/core/formulas/rune.ts` — 공식 미러
- `server/src/core/formulas/rune.test.ts` — parity/앵커/캡

### 수정 (server)
- `server/src/core/formulas/index.ts` — `export * from "./rune.js"` 추가

---

## 인터페이스 계약 (정확한 시그니처)

### 1. RuneTypes.h

```cpp
UENUM(BlueprintType)
enum class ERuneType : uint8
{
    None = 0 UMETA(Hidden),
    // 코어 (% 무한 곱 — RefreshDerivedStats 합류)
    PhysAtk  = 1 UMETA(DisplayName = "PhysAtk"),
    MagicAtk = 2 UMETA(DisplayName = "MagicAtk"),
    PhysDef  = 3 UMETA(DisplayName = "PhysDef"),
    MagicDef = 4 UMETA(DisplayName = "MagicDef"),
    Hp       = 5 UMETA(DisplayName = "Hp"),
    // 유틸 (flat 가산 + 캡)
    CritDamage = 6 UMETA(DisplayName = "CritDamage"),
    GoldFind   = 7 UMETA(DisplayName = "GoldFind"),
    ExpBoost   = 8 UMETA(DisplayName = "ExpBoost"),
    OfflineEff = 9 UMETA(DisplayName = "OfflineEff")
};

// 코어 룬 합산 배수 (각 = 1 + Σ 코어 룬 배수; 미장착 시 1.0)
USTRUCT(BlueprintType)
struct FRuneCoreMultipliers
{
    GENERATED_BODY()
    UPROPERTY() float PhysAtk = 1.0f;
    UPROPERTY() float MagicAtk = 1.0f;
    UPROPERTY() float PhysDef = 1.0f;
    UPROPERTY() float MagicDef = 1.0f;
    UPROPERTY() float Hp = 1.0f;
};

// 유틸 룬 합산 값 (캡 적용 후; 미장착 시 0)
USTRUCT(BlueprintType)
struct FRuneUtilValues
{
    GENERATED_BODY()
    UPROPERTY() float CritDamage = 0.0f; // 파생 CritDmg 가산
    UPROPERTY() float GoldFind = 0.0f;   // 골드 획득 배수 가산 (reward ×(1+GoldFind))
    UPROPERTY() float ExpBoost = 0.0f;   // 경험치 배수 가산
    UPROPERTY() float OfflineEff = 0.0f; // 오프라인 효율 가산
};

USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneInstance
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RuneId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) ERuneType RuneType = ERuneType::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EItemRarity Rarity = EItemRarity::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0")) int32 EnhanceLevel = 0;
};

// 저장 엔트리 (인벤토리 룬 1개)
USTRUCT(BlueprintType)
struct IDLEPROJECT_API FRuneSaveEntry
{
    GENERATED_BODY()
    UPROPERTY() ERuneType RuneType = ERuneType::None;
    UPROPERTY() EItemRarity Rarity = EItemRarity::None;
    UPROPERTY() int32 EnhanceLevel = 0;
    UPROPERTY() FName RuneId = NAME_None;
};
```

`#include "ItemSystem/ItemTypes.h"` (EItemRarity 재사용).

### 2. RuneFormula.h — `FRuneFormula` (static, 클라↔서버 미러)

```cpp
struct IDLEPROJECT_API FRuneFormula
{
    static constexpr int32 RuneSlotCount = 6;

    static bool IsCoreType(ERuneType Type);   // PhysAtk..Hp = true
    static bool IsUtilType(ERuneType Type);   // CritDamage..OfflineEff = true

    // 코어 룬 1개 % 배수: baseByRarity + lvl * stepByRarity (무한)
    static float GetCoreRuneMultiplier(EItemRarity Rarity, int32 EnhanceLevel);
    // 유틸 룬 1개 값: min(baseByRarity + lvl * stepByType, GetUtilCap(Type))
    static float GetUtilRuneValue(ERuneType Type, EItemRarity Rarity, int32 EnhanceLevel);
    static float GetUtilCap(ERuneType Type);

    // 강화 비용 (1회 = 현재 lvl → lvl+1)
    static int64 GetEnhanceEssenceCost(int32 CurrentLevel); // 10 * (lvl+1)^2
    static int64 GetEnhanceGoldCost(int32 CurrentLevel);    // 1000 * (lvl+1)^2

    // 분해 환급 에센스
    static int64 GetDisenchantEssence(EItemRarity Rarity, int32 EnhanceLevel);

    // 드롭/상점
    static bool RollRuneDrop(int32 MonsterLevel, bool bIsBoss, FRandomStream& Rng, FRuneInstance& OutRune);
    static int64 GetShopRuneRollCost(int32 ProgressIndex);
    static FRuneInstance RollShopRune(int32 ProgressIndex, FRandomStream& Rng);
};
```

**초기 밸런스 상수(balance가 [2]에서 시뮬 튜닝):**

| 항목 | Common | Uncommon | Rare | Epic | Legendary | Mythic |
| --- | --- | --- | --- | --- | --- | --- |
| 코어 base(%) | 2 | 3.5 | 5 | 8 | 12 | 18 |
| 코어 step(%/lvl) | 0.4 | 0.6 | 0.9 | 1.4 | 2.0 | 3.0 |
| 유틸 base(%) | 2 | 3 | 4 | 6 | 9 | 12 |
| 분해 에센스 base | 1 | 2 | 5 | 12 | 30 | 80 |

- 유틸 step: CritDamage/GoldFind/ExpBoost 0.5%/lvl, OfflineEff 0.3%/lvl
- 유틸 캡: CritDamage 1.0(=+100%), GoldFind 2.0, ExpBoost 2.0, OfflineEff 0.5
- 강화 비용: 에센스 `10*(lvl+1)^2`, 골드 `1000*(lvl+1)^2`
- 분해 환급: `base + EnhanceLevel * 2`
- 드롭 확률: 일반 몬스터 2%, 보스 25%. 레어도 곡선은 장비 `RollRarityForLevel` 유사(레벨 비례 상위 확률↑), 룬 전용 상수.
- 상점 비용: `5000 * (1 + ProgressIndex * 0.1)` (int64 포화 클램프)

### 3. RuneService.h — `URuneService : public UObject`

```cpp
UCLASS()
class IDLEPROJECT_API URuneService : public UObject
{
    GENERATED_BODY()
public:
    void AddRune(const FRuneInstance& Rune); // OwnedRunes 추가
    // 장착: 슬롯(0..5)에 인벤 인덱스 룬 장착. 이미 장착된 동일 인덱스 거부, 슬롯 점유 시 교체.
    bool TryEquipRune(int32 SlotIndex, int32 OwnedIndex);
    bool UnequipRune(int32 SlotIndex);
    // 강화: 비용 충분 시 EnhanceLevel++. 비용 차감은 GameInstance 게이트가 수행 → 여기서는 레벨만 증가.
    bool EnhanceRune(int32 OwnedIndex);
    // 분해: 장착중이면 거부. 제거 후 장착 인덱스 재매핑. 환급 에센스 반환(out).
    bool TryDisenchantRune(int32 OwnedIndex, int64& OutEssenceRefund);

    FRuneCoreMultipliers GetEquippedCoreMultipliers() const; // 장착 코어 룬 합산
    FRuneUtilValues GetEquippedUtilValues() const;           // 장착 유틸 룬 합산(캡)

    const TArray<FRuneInstance>& GetOwnedRunes() const { return OwnedRunes; }
    int32 GetEquippedOwnedIndex(int32 SlotIndex) const; // 없으면 INDEX_NONE

    void CaptureState(TArray<FRuneSaveEntry>& OutRunes, TArray<int32>& OutEquippedSlots) const;
    void RestoreState(const TArray<FRuneSaveEntry>& InRunes, const TArray<int32>& InEquippedSlots);

private:
    UPROPERTY() TArray<FRuneInstance> OwnedRunes;
    // 인덱스 = 슬롯(0..5), 값 = OwnedRunes 인덱스 또는 INDEX_NONE. 길이 항상 RuneSlotCount.
    UPROPERTY() TArray<int32> EquippedSlots;
};
```

저장 직렬화는 `EquippedSlots`(길이 6, 값 OwnedRunes 인덱스/-1) 그대로 + `OwnedRunes`→`FRuneSaveEntry` 변환. RestoreState는 무효 RuneType/Rarity/음수레벨 필터 후, 필터로 바뀐 인덱스를 `EquippedSlots`에 재매핑(#53 교훈). 장착 인덱스가 범위 밖이면 -1.

### 4. IdleSaveGame.h 수정

```cpp
UPROPERTY() int32 SaveVersion = 3;          // 2 → 3
UPROPERTY() TArray<FRuneSaveEntry> Runes;
UPROPERTY() TArray<int32> EquippedRuneSlots; // 길이 6
UPROPERTY() int64 RuneEssence = 0;
```
`#include "RuneSystem/RuneTypes.h"`.

### 5. IdleGameInstance 통합

헤더 추가:
```cpp
UPROPERTY(Transient) TObjectPtr<URuneService> RuneService;
void EnsureRuneService();
UFUNCTION(BlueprintPure, Category="Idle|Rune") URuneService* GetRuneService() const { return RuneService; }
UFUNCTION(BlueprintPure, Category="Idle|Rune") int64 GetRuneEssence() const { return RuneEssence; }

// 골드+에센스 게이트 (3중 가드 1회 차감, 기존 강화/상점 패턴 동일)
UFUNCTION(BlueprintCallable, Category="Idle|Rune") bool TryEnhanceRune(int32 OwnedIndex);
UFUNCTION(BlueprintCallable, Category="Idle|Rune") bool TryDisenchantRune(int32 OwnedIndex);
UFUNCTION(BlueprintCallable, Category="Idle|Rune") bool TryBuyRuneRoll();
UFUNCTION(BlueprintCallable, Category="Idle|Rune") bool TryEquipRune(int32 SlotIndex, int32 OwnedIndex);
UFUNCTION(BlueprintCallable, Category="Idle|Rune") bool UnequipRune(int32 SlotIndex);

// 유틸 룬 조회 (reward/offline 훅용)
float GetRuneGoldFindBonus() const;  // RuneService->GetEquippedUtilValues().GoldFind
float GetRuneExpBoostBonus() const;
float GetRuneOfflineEffBonus() const;
float GetRuneCritDamageBonus() const;
```
- 멤버: `int64 RuneEssence = 0;`
- `Init()`에 `EnsureRuneService()` 추가, `Shutdown()`에 null.
- `TryEnhanceRune`: 룬 존재 → `GetEnhanceEssenceCost`/`GetEnhanceGoldCost` 계산 → 에센스·골드 동시 충분 검사 → 둘 다 차감 → `RuneService->EnhanceRune` → `RefreshDerivedStats` 트리거(캐릭터). 부족 시 false(차감 없음).
- `TryDisenchantRune`: `RuneService->TryDisenchantRune`(장착중 거부) → 환급 에센스 += → autosave.
- `TryBuyRuneRoll`: 골드 `GetShopRuneRollCost` 게이트 1회 차감 → `RollShopRune` → `AddRune`.
- **드롭 훅**: 몬스터 처치 시점(기존 골드/아이템 드롭 경로, `IdleMonster`/GameMode 처치 처리부)에 `RollRuneDrop` 추가 → 성공 시 `RuneService->AddRune`.

### 6. RefreshDerivedStats 통합 (IdleCharacter.cpp:185 직후)

```cpp
// 기존 181-185: Derived.* *= StatMultiplier;
if (IdleGameInstance)
{
    if (const URuneService* Rune = IdleGameInstance->GetRuneService())
    {
        const FRuneCoreMultipliers M = Rune->GetEquippedCoreMultipliers();
        Derived.PhysAtk  *= M.PhysAtk;
        Derived.MagicAtk *= M.MagicAtk;
        Derived.PhysDef  *= M.PhysDef;
        Derived.MagicDef *= M.MagicDef;
        Derived.Hp       *= M.Hp;
        Derived.CritDmg  += Rune->GetEquippedUtilValues().CritDamage; // 유틸 CritDmg 가산
    }
}
```
(CritDmg 필드명은 `FDerivedStats`의 기존 크리뎀 필드에 맞춤 — `BonusCritDmg` 전파 경로 확인.)

### 7. reward / offline 훅
- **GoldFind**: 골드 지급 적용 시점(GameInstance가 `AddGold` 호출하는 처치/스테이지 보상 경로)에서 `round(gold * (1 + GetRuneGoldFindBonus()))`. `RewardFormula`는 순수 유지, 곱은 적용부에서.
- **ExpBoost**: 경험치 지급 시점 동일 패턴 `(1 + GetRuneExpBoostBonus())`.
- **OfflineEff**: 오프라인 보상 계산 시 `OfflineRewardFormula` 결과에 `(1 + GetRuneOfflineEffBonus())` 곱(적용부). int64 포화 클램프.

### 8. CaptureToSave / ApplyFromSave (IdleGameInstance.cpp)
- Capture: `RuneService->CaptureState(Save->Runes, Save->EquippedRuneSlots); Save->RuneEssence = RuneEssence;`
- Apply: `RuneEssence = Save->RuneEssence;` + `if (SaveVersion >= 3) RuneService->RestoreState(...)` else 빈 룬/0 에센스(회귀안전).

### 9. server/src/core/formulas/rune.ts (미러)

```ts
export type RuneRarity = 1|2|3|4|5|6; // Common..Mythic
export const RUNE_SLOT_COUNT = 6;
export function isCoreType(t: number): boolean;   // 1..5
export function isUtilType(t: number): boolean;   // 6..9
export function getCoreRuneMultiplier(rarity: number, lvl: number): number; // Math.fround
export function getUtilRuneValue(type: number, rarity: number, lvl: number): number; // min(.., cap), fround
export function getUtilCap(type: number): number;
export function getEnhanceEssenceCost(lvl: number): number; // 10*(lvl+1)^2
export function getEnhanceGoldCost(lvl: number): number;    // 1000*(lvl+1)^2
export function getDisenchantEssence(rarity: number, lvl: number): number;
export function getShopRuneRollCost(progressIndex: number): number;
```
상수값은 클라 `FRuneFormula`와 **정확히 동일**. 모든 float 반환은 `Math.fround`로 32-bit 정밀도 미러.

---

## 테스트 케이스 (TDD — 먼저 작성, 실패 확인 후 구현)

### 클라 Automation — RuneFormulaTests.cpp
- `GetCoreRuneMultiplier(Common, 0) == 0.02f`
- `GetCoreRuneMultiplier(Mythic, 50) ≈ 0.18 + 50*0.03 = 1.68f` (무한 — 캡 없음)
- 코어 배수 강화 레벨 단조 증가
- `GetUtilRuneValue(GoldFind, Mythic, 0) == 0.12f`
- 유틸 캡 경계: `GetUtilRuneValue(GoldFind, Mythic, 10000) == GetUtilCap(GoldFind) == 2.0f`
- `GetUtilRuneValue(OfflineEff, ...) ≤ 0.5f` 항상
- `GetEnhanceEssenceCost(0)==10, (1)==40, (4)==250`; 골드 `(0)==1000`
- `IsCoreType/IsUtilType` 분류 정확
- 드롭 결정성: 동일 시드 → 동일 룬

### 클라 Automation — RuneServiceTests.cpp
- AddRune → OwnedRunes 길이 증가
- TryEquipRune(slot,idx) → GetEquippedOwnedIndex == idx; 코어 장착 시 `GetEquippedCoreMultipliers().<type>` = 1+배수
- 빈 룬 → 코어 배수 모두 1.0, 유틸 모두 0.0 (회귀안전)
- EnhanceRune → EnhanceLevel++ → 배수 증가
- 장착중 TryDisenchantRune == false (거부)
- 미장착 TryDisenchantRune == true + 환급 에센스 = GetDisenchantEssence
- 분해 후 장착 인덱스 재매핑: 슬롯에 idx2 장착, idx0 분해 → 슬롯이 여전히 동일 룬 가리킴
- Capture→Restore 라운드트립: OwnedRunes/EquippedSlots/배수 동일
- 무효 데이터 Restore: RuneType=None/음수레벨/범위밖 슬롯 → 필터, 크래시 없음
- v2 세이브(룬 없음) Apply → 빈 룬/0 에센스, 기존 시스템 무영향

### 클라 Automation — 스탯 합성/통합
- 룬 미장착 RefreshDerivedStats → 기존 값 동일 (회귀안전)
- PhysAtk 코어 룬 장착 → Derived.PhysAtk만 증가, 타 스탯 불변
- TryEnhanceRune 골드/에센스 부족 → false + 차감 없음 + 레벨 불변

### 서버 vitest — rune.test.ts
- 앵커값 클라와 동일 (`getCoreRuneMultiplier(6,50)` toBeCloseTo `Math.fround(1.68)`)
- 캡: `getUtilRuneValue(7,6,10000) === 2.0`
- 강화 비용 정수 일치
- parity: 코어/유틸/비용/분해 클라 앵커 전수 미러

---

## Codex 작업 분배 ([2])

| 파트 | 작업 |
| --- | --- |
| **character (메인)** | RuneTypes/RuneFormula/RuneService + GameInstance 통합(getter/Ensure/Capture/Apply/게이트 API) + RefreshDerivedStats 합성 + reward/offline/CritDmg 훅 + 저장(v3) + RuneFormulaTests/RuneServiceTests |
| **backend** | rune.ts 미러 + rune.test.ts(앵커/parity/캡) + index.ts export + 클라우드(#54) clientSave 룬 직렬화 round-trip 확인 |
| **designer** | FIdleHUDRuneViewModel + 룬 패널(장착/해제/강화/분해/상점) + Rune.csv ko/en(룬 타입명 9 + UI 라벨) |
| **balance** | 코어/유틸 base·step·캡, 강화 비용(에센스·골드), 분해 환급, 드롭률, 상점 비용 시뮬 튜닝. 골드·에센스 싱크 + 페이싱 median 영향 + 파워크리프 가드. balance-sim 확장 |
| **qa** | 장착·강화·분해·스탯합성·저장·v2→v3 마이그레이션·유틸 캡·장착중 분해 거부·드롭 결정성 시나리오 |

---

## 워크플로우 v3 단계
- [1] ✅ 기획서(`61-rune-relic-system.md`) + 본 계획 + PR 생성
- [2] Codex character 메인(+backend/designer/balance/qa) → PM이 산출 PR 코멘트 게시
- [3] Claude TM 종합 리뷰 + 직접 fix (parity/회귀안전/장착 인덱스/캡/저장 마이그레이션 중점)
- [4] Codex TM 리뷰 + fix → PM이 fix 산출 게시
- [5] Claude 검증 (UE Build + Automation / 서버 build·test·lint)
- [N] **CI 그린 확정(server-ci 포함)** + PM 종합 소견 + `gh pr merge --squash --delete-branch`

## Self-Review 결과
- **스펙 커버리지**: 기획서 DoD 1~8 전부 본 계획에 매핑(데이터모델→§1, 공식→§2, 서비스→§3, 스탯합성→§6, 저장→§4/§8, 서버미러→§9, HUD→designer, 테스트→테스트 케이스). 갭 없음.
- **placeholder 스캔**: 초기 밸런스 수치 구체 명시(balance 튜닝은 의도적 위임이며 시작값 제공). TBD 없음.
- **타입 일관성**: `ERuneType`/`FRuneInstance`/`FRuneCoreMultipliers`/`FRuneUtilValues`/`FRuneSaveEntry`/`URuneService` API/`FRuneFormula` static 명칭이 전 섹션 일치. `EquippedSlots`(클라 내부) ↔ `EquippedRuneSlots`(저장 필드) 변환 명시.
