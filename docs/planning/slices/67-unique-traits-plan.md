# 유니크/초월 전용 효과 구현 계획 — PR #67

> **실행 방식:** 워크플로우 v3 [2] Codex 명세. character가 클라+서버 미러+테스트 통합, [3] Claude TM 검증. PM 자율 진행.

**목표:** 유니크/초월 등급 장비 전용 고유 스탯 효과(Unique Trait), 유니크 1 / 초월 2.

**아키텍처:** `FItemInstance.UniqueTrait1/2`(EUniqueTrait). 드롭/생성 시 등급 게이트(유니크/초월만) RNG 부여. `FUniqueTraitFormula`가 trait 값(등급 배수) 산출 → `ComputeEquipmentBonus` 합산. 저장 `SaveVersion` 8→9(enum 필드 None 기본값).

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 Item/Drop/Inventory 패턴.

---

## 인터페이스 계약

### 1. ItemTypes.h
```cpp
UENUM(BlueprintType)
enum class EUniqueTrait : uint8
{
    None = 0 UMETA(Hidden),
    AllStatSurge = 1 UMETA(DisplayName = "AllStatSurge"),     // 물공·마공·물방·마방 +%
    CritDamageSurge = 2 UMETA(DisplayName = "CritDamageSurge"),// 치명피해 +
    CritRateSurge = 3 UMETA(DisplayName = "CritRateSurge"),    // 치명률 +
    LifeSurge = 4 UMETA(DisplayName = "LifeSurge"),            // 체력 +%
    SwiftSurge = 5 UMETA(DisplayName = "SwiftSurge"),          // 공속 +
    PhysMastery = 6 UMETA(DisplayName = "PhysMastery"),        // 물공 +%
    MagicMastery = 7 UMETA(DisplayName = "MagicMastery"),      // 마공 +%
    GuardMastery = 8 UMETA(DisplayName = "GuardMastery")       // 물방·마방 +%
};
// FItemInstance에 추가:
UPROPERTY(EditAnywhere, BlueprintReadWrite) EUniqueTrait UniqueTrait1 = EUniqueTrait::None;
UPROPERTY(EditAnywhere, BlueprintReadWrite) EUniqueTrait UniqueTrait2 = EUniqueTrait::None;
```

### 2. UniqueTraitFormula (신규 ItemSystem/UniqueTraitFormula.h/.cpp)
```cpp
struct IDLEPROJECT_API FUniqueTraitFormula
{
    // trait 값 (Unique base, Transcendent ×1.5). %는 0.xx, flat은 절대값.
    static float GetTraitValue(EUniqueTrait Trait, EItemRarity Rarity);
    // trait가 유효 등급(Unique=4/Transcendent=6)에서만 부여되는지
    static bool RarityGrantsUnique(EItemRarity Rarity);   // Unique 이상
    static bool RarityGrantsTwoTraits(EItemRarity Rarity);// Transcendent
    // 부여: 유니크 1 / 초월 2(서로 다른). 그 외 None.
    static void RollUniqueTraits(EItemRarity Rarity, FRandomStream& Rng, EUniqueTrait& OutTrait1, EUniqueTrait& OutTrait2);
    // 장비 1개 trait → DerivedStats 기여(ComputeEquipmentBonus가 호출, 순수 보너스)
    static void AccumulateTraitBonus(const FItemInstance& Item, FDerivedStats& OutBonus);
};
```
- 값 가이드(balance 조정): AllStatSurge Unique 0.08(8%)/Transcendent 0.12, CritDamageSurge 0.15/0.225, CritRateSurge 0.05/0.075, LifeSurge 0.10/0.15, SwiftSurge 0.08/0.12, PhysMastery·MagicMastery 0.12/0.18, GuardMastery 0.10/0.15.
- %류는 해당 스탯에 곱(`Stat *= (1+val)`) 아니라 **가산 누적 후 적용** 일관성 위해 ComputeEquipmentBonus 패턴 따름(기존 affix가 flat 가산이면 trait %는 별도 처리 — 구현 시 기존 ComputeEquipmentBonus 구조에 맞춰 character 결정, 단 곱 중첩 폭발 금지/합산 기조 PR #61 교훈).

### 3. ComputeEquipmentBonus (InventoryComponent.cpp) 통합
```cpp
// 각 장착 장비 루프에서 기존 Bonus 합산 직후:
FUniqueTraitFormula::AccumulateTraitBonus(EquippedItem, OutBonus);
// (UniqueTrait1/2 각각 GetTraitValue → 해당 파생스탯 가산. None은 0.)
```

### 4. 부여 (DropFormula/ItemFactory)
```cpp
// ComputeItemBonus 또는 RollAffixes 후, 등급이 Unique/Transcendent면:
FUniqueTraitFormula::RollUniqueTraits(Rarity, Rng, OutItem.UniqueTrait1, OutItem.UniqueTrait2);
// 시드 순서 보존(기존 RollAffixes 후 추가 → 결정성 테스트 갱신)
```

### 5. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 9; // 8 → 9
// FItemInstance.UniqueTrait1/2 직렬화 자동(기존 세이브 None 기본값 — 마이그레이션 불필요, 회귀안전)
```

### 6. server uniqueTrait.ts
```ts
export type UniqueTrait = 0|1|2|3|4|5|6|7|8;
export function getTraitValue(trait: number, rarity: number): number; // Math.fround, Unique base/Transcendent 1.5×
export function rarityGrantsUnique(rarity: number): boolean;   // >=4 (Unique) … 단 7단계: Unique=4/Transcendent=6
export function rarityGrantsTwoTraits(rarity: number): boolean; // ===6 (Transcendent)
```
값/배수 클라 동일. 클라우드 검증은 clientSave 자동(서버 스키마 무변경).

---

## 테스트 케이스

### 클라 Automation
- 등급 게이트: Common~Legendary(1,2,3,5) → trait None. Unique(4) → Trait1만(Trait2 None). Transcendent(6) → Trait1+Trait2(서로 다름).
- Mythic(7)은? — 설계: trait 미부여(유니크/초월 전용). 또는 Mythic도? **명시: trait는 Unique(4)/Transcendent(6) 전용**, Mythic(7)은 None(기존 최강 스탯 배수 유지). RollUniqueTraits Mythic → None.
- GetTraitValue Unique vs Transcendent 배수(×1.5)
- AccumulateTraitBonus: AllStatSurge → 4스탯 가산, None → 0
- ComputeEquipmentBonus trait 합산 반영
- 저장 라운드트립(UniqueTrait1/2) + v8 세이브(trait 없음) → None 회귀안전
- 결정적 RNG(동일 시드 동일 trait)
- 초월 Trait1≠Trait2

### 서버 vitest
- getTraitValue 8 trait × Unique/Transcendent 앵커, Math.fround
- rarityGrantsUnique(4)=true,(6)=true,(7)=false,(1~3,5)=false
- rarityGrantsTwoTraits(6)=true, 그 외 false

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | EUniqueTrait/FItemInstance.UniqueTrait1·2/FUniqueTraitFormula + 부여(DropFormula/ItemFactory) + ComputeEquipmentBonus + 저장 v9 + 서버 uniqueTrait.ts + Tests |
| designer | 유니크/초월 고유 효과 HUD + Item 로컬라이즈 ko/en(trait명 8) |
| balance | trait 값/등급 배수 시뮬 + 파워크리프 가드 |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(trait 값 가이드 명시, 정밀 수치 balance 위임)
- 타입 일관성: `EUniqueTrait`(8)/`FItemInstance.UniqueTrait1·2`/`FUniqueTraitFormula`/`SaveVersion`(9)/`uniqueTrait.ts` 전 섹션 일치
- 주의: trait는 **Unique(4)/Transcendent(6) 전용**(Mythic 7 제외) / 합산 기조(곱 폭발 금지, PR #61 교훈) / 초월 Trait1≠Trait2 / 클라↔서버 parity
