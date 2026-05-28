# 직업 전용 룬 구현 계획 — PR #63

> **실행 방식:** 워크플로우 v3 [2] Codex 작업 명세. character가 클라+서버 미러+테스트 통합, [3] Claude TM 검증.

**목표:** 8직업 전용 마스터리 룬 + 전용 슬롯(인덱스 6)으로 직업 주력 스탯 무한 강화.

**아키텍처:** `FRuneInstance.ClassRestriction`(직업) + `ERuneType::ClassMastery`. 전용 슬롯 6은 직업 일치 ClassMastery 룬만, 범용 0~5는 코어/유틸만. 마스터리 배수는 `FClassRuneFormula`가 직업별 코어 스탯 묶음으로 산출 → `GetEquippedCoreMultipliers` 합산 합류. 저장 `SaveVersion` 4→5(슬롯 6→7).

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 RuneService/RuneFormula 패턴 재사용.

---

## 파일 구조

### 생성 (client)
- `RuneSystem/ClassRuneFormula.h` / `.cpp` — `FClassRuneFormula`
- `Tests/ClassRuneFormulaTests.cpp` / `Tests/ClassRuneServiceTests.cpp`

### 수정 (client)
- `RuneSystem/RuneTypes.h` — `ERuneType::ClassMastery = 10` + `FRuneInstance.ClassRestriction`(EClassId) + `FRuneSaveEntry.ClassRestriction`
- `RuneSystem/RuneFormula.h/.cpp` — `RuneSlotCount` 6→7, `IsCoreType`/`IsUtilType`에 ClassMastery 비포함(별 분류), 드롭에 ClassMastery 제외(전용 경로)
- `RuneSystem/RuneService.h/.cpp` — `TryEquipRune` 슬롯6 직업 제약 + 0~5 ClassMastery 거부, `GetEquippedCoreMultipliers`에 마스터리 합산, `AddClassRune`/제작, `RestoreState` 7슬롯
- `GameCore/IdleSaveGame.h` — `SaveVersion` 4→5
- `GameCore/IdleGameInstance.h/.cpp` — `TryCraftClassRune`(현재 직업, 에센스 게이트) + 드롭 훅(자기 직업 ClassMastery) + Apply v5 마이그레이션
- `CharacterSystem/IdleCharacter.cpp` — (변경 최소: GetEquippedCoreMultipliers가 이미 마스터리 포함하므로 RefreshDerivedStats 무수정 가능. 캐릭터 직업을 RuneService에 전달하는 경로 필요 시 추가)
- `UI/IdleHUD.h/.cpp` — 전용 슬롯 표시 + 제작 액션
- `Content/Localization/Game/{ko,en}/Rune.csv` — 직업 룬 키

### 생성/수정 (server)
- `server/src/core/formulas/classRune.ts` + `classRune.test.ts` + `index.ts` export

---

## 인터페이스 계약

### 1. RuneTypes.h 수정
```cpp
// ERuneType에 추가:
ClassMastery = 10 UMETA(DisplayName = "ClassMastery")

// FRuneInstance에 추가:
UPROPERTY(EditAnywhere, BlueprintReadWrite) EClassId ClassRestriction = EClassId::None;
// FRuneSaveEntry에 추가:
UPROPERTY() EClassId ClassRestriction = EClassId::None;
```
`#include "CharacterSystem/StatFormulas.h"`(EClassId).

### 2. ClassRuneFormula
```cpp
struct IDLEPROJECT_API FClassRuneFormula
{
    static constexpr int32 ClassRuneSlotIndex = 6; // 전용 슬롯

    // 직업 전용 룬의 코어 스탯 묶음 기여 (장착 1개). 직업별 1~2 스탯에 % 부여.
    // 반환 FRuneCoreMultipliers: 해당 스탯에 (배수), 무관 스탯 0.
    static FRuneCoreMultipliers GetClassMasteryMultipliers(EClassId ClassId, EItemRarity Rarity, int32 EnhanceLevel);

    // 직업별 마스터리 단위 배수 (코어 step 재사용 또는 전용 상수)
    static float GetClassMasteryUnit(EItemRarity Rarity, int32 EnhanceLevel); // = FRuneFormula::GetCoreRuneMultiplier 재사용
};
```
- `GetClassMasteryMultipliers`: 직업별 스탯 매핑(Warrior PhysAtk+PhysDef / Mage MagicAtk / Archer PhysAtk / Thief PhysAtk / Cleric MagicAtk+Hp / Paladin PhysDef+Hp / Berserker PhysAtk / Summoner MagicAtk). 단일 스탯 직업은 `unit`, 2스탯 직업은 각 `unit`(또는 0.7×unit balance 튜닝). `FRuneCoreMultipliers`의 해당 필드에 `unit`(가산 기여값, 1.0 base 아님 — GetEquippedCoreMultipliers에서 합산되므로 **순수 보너스 값** 반환).

⚠️ **주의**: `GetClassMasteryMultipliers`는 `FRuneCoreMultipliers`를 **보너스 합산용**으로 반환(해당 스탯 = bonus, 무관 스탯 = 0). `GetEquippedCoreMultipliers`가 `Result.<stat> += bonus`로 흡수.

### 3. RuneService 수정
```cpp
// RuneSlotCount: FRuneFormula::RuneSlotCount = 7 (6→7)

// TryEquipRune 규칙:
//  - SlotIndex == ClassRuneSlotIndex(6): OwnedRunes[OwnedIndex].RuneType == ClassMastery
//    && ClassRestriction == EquippedClassId(캐릭터 직업) 여야 함. 아니면 false.
//  - SlotIndex 0~5: RuneType != ClassMastery 여야 함(범용만). ClassMastery면 false.
bool TryEquipRune(int32 SlotIndex, int32 OwnedIndex); // 직업 제약 추가

// 캐릭터 직업 주입 (장착 검사용). GameInstance가 설정하거나 인자로.
void SetOwnerClassId(EClassId ClassId);
EClassId OwnerClassId = EClassId::None;

// GetEquippedCoreMultipliers: 기존 코어 룬 합산 + 전용 슬롯 ClassMastery 마스터리 합산
//  for 전용 슬롯 룬: Bonus = FClassRuneFormula::GetClassMasteryMultipliers(rune.ClassRestriction, rarity, lvl)
//  Result.PhysAtk += Bonus.PhysAtk; ... (Bonus는 순수 보너스값)
```

### 4. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 5; // 4 → 5
// FRuneSaveEntry.ClassRestriction 추가로 Runes 직렬화 자동 포함
// EquippedRuneSlots 길이 7 (RestoreState가 EnsureSlotCount로 7 보장)
```

### 5. IdleGameInstance
```cpp
UFUNCTION(BlueprintCallable, Category="Idle|Rune")
bool TryCraftClassRune(); // 현재 직업 ClassMastery 룬 제작. 에센스 게이트(GetClassRuneCraftCost) 1회 차감 → AddRune(ClassRestriction=직업)
// 드롭 훅: RollRuneDrop 외 별도 RollClassRuneDrop(현재 직업, 낮은 확률) → AddRune
// EnsureRuneService 후 RuneService->SetOwnerClassId(캐릭터 직업) 동기화
// ApplyFromSave: SaveVersion>=5 면 그대로, <5 면 7슬롯 확장(전용 슬롯 INDEX_NONE)
```
`FClassRuneFormula` 또는 `FRuneFormula`에 `GetClassRuneCraftCost(EItemRarity)` 추가(에센스 비용).

### 6. server classRune.ts
```ts
export const CLASS_RUNE_SLOT_INDEX = 6;
// 직업별 마스터리 스탯 매핑 (클라 동일)
export interface ClassMasteryMultipliers { physAtk: number; magicAtk: number; physDef: number; magicDef: number; hp: number; }
export function getClassMasteryMultipliers(classId: number, rarity: number, lvl: number): ClassMasteryMultipliers; // Math.fround
```
상수/매핑 클라 정확히 동일.

---

## 테스트 케이스

### 클라 Automation
- 전용 슬롯6: 직업 일치 ClassMastery 룬 장착 성공
- 전용 슬롯6: 직업 불일치 ClassMastery 룬 거부
- 전용 슬롯6: 범용(코어/유틸) 룬 거부
- 범용 슬롯0~5: ClassMastery 룬 거부
- 마스터리 합산: Warrior 룬 장착 → GetEquippedCoreMultipliers PhysAtk·PhysDef 증가, 타 스탯 불변(직업 매핑)
- 8직업 각 마스터리 매핑 정확
- TryCraftClassRune: 에센스 충분 → 현재 직업 룬 생성, 부족 → 거부
- 드롭: RollClassRuneDrop 현재 직업 것만
- 저장 v4→v5 마이그레이션: 6슬롯 세이브 → 7슬롯, 전용 슬롯 INDEX_NONE, 기존 룬 ClassRestriction 0
- Capture-Restore 라운드트립(ClassRestriction 포함)
- 미장착/직업 불일치 → 마스터리 0 기여(회귀안전)

### 서버 vitest
- `getClassMasteryMultipliers(1, rarity, lvl)` Warrior physAtk·physDef 값 클라 앵커 일치
- 8직업 전부 매핑 parity + Math.fround
- 무효 classId(0/9+) → 전부 0

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | ClassRestriction/ClassMastery/FClassRuneFormula + RuneSlotCount 7 + TryEquipRune 직업 제약 + GetEquippedCoreMultipliers 마스터리 합산 + TryCraftClassRune/드롭 + 저장 v5 + 서버 classRune.ts + Tests |
| designer | 전용 슬롯 HUD + 제작 액션 + Rune.csv ko/en 직업 룬 키 |
| balance | 8직업 마스터리 배율 시뮬 + #60 직업 밸런스 밴드 정합 + 파워크리프 가드 |
| (backend/qa) | character 흡수 가능, [3] Claude TM이 parity·커버리지 점검 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~9 매핑 ✅
- placeholder 없음(직업 매핑 표 명시, 배율은 balance 위임 + unit 재사용 명시)
- 타입 일관성: `ClassRestriction`/`ERuneType::ClassMastery`/`FClassRuneFormula::GetClassMasteryMultipliers`/`ClassRuneSlotIndex=6`/`classRune.ts` 전 섹션 일치
- 주의: `GetClassMasteryMultipliers` 반환을 **순수 보너스값**(0 base, 해당 스탯만)으로 통일 — `GetEquippedCoreMultipliers`의 `+=` 합산과 정합(곱 아님, PR #61 교훈)
