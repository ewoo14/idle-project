# 룬 세트 효과 구현 계획 — PR #64

> **실행 방식:** 워크플로우 v3 [2] Codex 작업 명세. character가 클라+서버 미러+테스트 통합, [3] Claude TM 검증. 장비 세트 `SetBonusFormula`(#43) / `FDropFormula::RollItemSet` 패턴 참고.

**목표:** 범용 6슬롯에 같은 룬 세트 2/4/6개 → tiered 보너스.

**아키텍처:** `FRuneInstance.RuneSet`(룬 전용 `ERuneSet`). `GetEquippedCoreMultipliers`/`GetEquippedUtilValues`가 범용 6슬롯(0~5)의 세트 개수 집계 → `FRuneSetFormula`가 2/4/6 tiered 보너스 산출 → 코어/유틸 가산 합류. 저장 `SaveVersion` 5→6.

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 RuneService/SetBonusFormula 패턴.

---

## 파일 구조

### 생성 (client)
- `RuneSystem/RuneSetFormula.h` / `.cpp` — `FRuneSetFormula`
- `Tests/RuneSetFormulaTests.cpp` / `Tests/RuneSetServiceTests.cpp`

### 수정 (client)
- `RuneSystem/RuneTypes.h` — `ERuneSet` enum + `FRuneInstance.RuneSet` + `FRuneSaveEntry.RuneSet`
- `RuneSystem/RuneFormula.cpp` — `RollShopRune`에 세트 롤(`FRuneSetFormula::RollRuneSet`, ClassMastery 제외)
- `RuneSystem/RuneService.cpp` — `GetEquippedCoreMultipliers`/`GetEquippedUtilValues`에 범용 6슬롯 세트 집계+보너스 합산
- `GameCore/IdleSaveGame.h` — `SaveVersion` 5→6
- `UI/IdleHUD.h/.cpp` — 세트 현황 ViewModel
- `Content/Localization/Game/{ko,en}/Rune.csv` — 세트 키

### 생성/수정 (server)
- `server/src/core/formulas/runeSet.ts` + `runeSet.test.ts` + `index.ts` export

---

## 인터페이스 계약

### 1. RuneTypes.h 수정
```cpp
UENUM(BlueprintType)
enum class ERuneSet : uint8
{
    None = 0 UMETA(Hidden),
    Offense = 1 UMETA(DisplayName = "Offense"),   // PhysAtk + MagicAtk (코어)
    Bastion = 2 UMETA(DisplayName = "Bastion"),   // PhysDef + MagicDef (코어)
    Vitality = 3 UMETA(DisplayName = "Vitality"), // Hp(코어) + OfflineEff(유틸)
    Fortune = 4 UMETA(DisplayName = "Fortune")    // GoldFind + ExpBoost + CritDamage (유틸)
};
// FRuneInstance에 추가: UPROPERTY(...) ERuneSet RuneSet = ERuneSet::None;
// FRuneSaveEntry에 추가: UPROPERTY() ERuneSet RuneSet = ERuneSet::None;
```

### 2. RuneSetFormula
```cpp
struct IDLEPROJECT_API FRuneSetFormula
{
    // 세트 티어 보너스 단위 (장착 개수 → 누적). 2/4/6 임계.
    static constexpr int32 Tier1Count = 2;
    static constexpr int32 Tier2Count = 4;
    static constexpr int32 Tier3Count = 6;
    static constexpr float Tier1Bonus = 0.05f;
    static constexpr float Tier2Bonus = 0.12f;
    static constexpr float Tier3Bonus = 0.25f;

    // 장착 개수 → 누적 보너스값(0/0.05/0.12/0.25)
    static float GetSetTierBonus(int32 EquippedCount);

    // 집계된 세트별 개수 맵 → 코어/유틸 보너스 산출
    //  Offense: core.PhysAtk += b; core.MagicAtk += b
    //  Bastion: core.PhysDef += b; core.MagicDef += b
    //  Vitality: core.Hp += b; util.OfflineEff += b
    //  Fortune: util.GoldFind += b; util.ExpBoost += b; util.CritDamage += b
    static void ComputeSetBonus(const TMap<ERuneSet, int32>& SetCounts, FRuneCoreMultipliers& OutCore, FRuneUtilValues& OutUtil);

    // 드롭 세트 롤 (레어도 비례 None~세트). 장비 RollItemSet 패턴.
    static ERuneSet RollRuneSet(EItemRarity Rarity, FRandomStream& Rng);
};
```
- `ComputeSetBonus`의 `OutCore`/`OutUtil`은 **순수 보너스값**(0 base 가산). 호출부에서 기존 합산에 `+=`.
- `GetSetTierBonus`: count>=6 → 0.25, >=4 → 0.12, >=2 → 0.05, else 0 (최고 티어만, 누적 아님 — 단순화).

### 3. RuneService 수정
```cpp
// GetEquippedCoreMultipliers / GetEquippedUtilValues 내부:
//  1) 범용 슬롯 0~5(인덱스 < ClassRuneSlotIndex)의 장착 룬 RuneSet 개수 집계 → TMap<ERuneSet,int32>
//     (ClassMastery 룬/직업 슬롯 제외, RuneSet==None 제외)
//  2) FRuneSetFormula::ComputeSetBonus(SetCounts, CoreBonus, UtilBonus)
//  3) 코어: Result.<stat> += CoreBonus.<stat>
//     유틸: 캡 적용 후 별도 가산 Result.<stat> += UtilBonus.<stat> (세트는 캡 외)
```
⚠️ 유틸 세트 보너스는 **캡 적용 후 별도 가산**(세트는 유틸 캡에 막히지 않음). 즉 `Result.GoldFind = min(룬합, cap+codexExt) + setBonus.GoldFind`.

### 4. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 6; // 5 → 6
// FRuneSaveEntry.RuneSet 추가로 Runes 직렬화 자동 포함
```
RestoreState/CaptureState: 기존 FRuneSaveEntry ↔ FRuneInstance 변환에 RuneSet 추가. v<6 세이브는 RuneSet None.

### 5. RuneFormula.cpp 드롭
```cpp
// RollShopRune 끝에:
Rune.RuneSet = FRuneSetFormula::RollRuneSet(Rune.Rarity, Rng); // 코어/유틸 룬만
// ClassMastery(MakeClassRune)는 RuneSet None 유지
```
⚠️ 시드 순서: 기존 RollRuneType/RollRuneRarity 후 RollRuneSet 추가 → 결정성 테스트 갱신.

### 6. server runeSet.ts
```ts
export const RUNE_SET_TIER1_COUNT = 2;
export const RUNE_SET_TIER2_COUNT = 4;
export const RUNE_SET_TIER3_COUNT = 6;
export const RUNE_SET_TIER1_BONUS = 0.05;
export const RUNE_SET_TIER2_BONUS = 0.12;
export const RUNE_SET_TIER3_BONUS = 0.25;
export function getSetTierBonus(equippedCount: number): number; // Math.fround
export interface RuneSetBonus { core: {physAtk;magicAtk;physDef;magicDef;hp}; util: {critDamage;goldFind;expBoost;offlineEff}; }
export function computeRuneSetBonus(setCounts: Record<number, number>): RuneSetBonus; // 매핑 클라 동일, Math.fround
```

---

## 테스트 케이스

### 클라 Automation
- 세트 None 룬 → 세트 보너스 0(회귀안전)
- Offense 룬 2개 장착(범용 슬롯) → 물공·마공 +0.05(GetSetTierBonus 2)
- Offense 4개 → +0.12, 6개 → +0.25
- 범용 슬롯 한정: 직업 전용 슬롯6의 ClassMastery는 세트 집계 제외(직업 룬 RuneSet None)
- Vitality: 체력(코어) + OfflineEff(유틸) 가산
- Fortune: GoldFind/ExpBoost/CritDamage 유틸 가산 — **캡 외 별도**(룬 캡 도달 상태에서도 세트 보너스 추가)
- 드롭 세트 부여: RollShopRune RuneSet 결정적(동일 시드 동일)
- 저장 v5→v6 마이그레이션: v5 세이브(RuneSet 없음) → None, 보너스 0
- Capture-Restore 라운드트립(RuneSet 포함)

### 서버 vitest
- `getSetTierBonus(1)===0`, `(2)===0.05`, `(4)===0.12`, `(6)===0.25`, `(7)===0.25`
- `computeRuneSetBonus({1:6})` Offense physAtk·magicAtk 0.25
- 4세트 매핑 parity + Math.fround

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | ERuneSet/RuneSet 필드/FRuneSetFormula + 세트 집계·합성(범용 6슬롯) + 드롭 부여 + 저장 v6 + 서버 runeSet.ts + Tests |
| designer | 세트 현황 HUD(개수 N/6 + 티어) + Rune.csv ko/en 세트 키 |
| balance | 세트 2/4/6 수치 시뮬 + 파워크리프 가드 + 페이싱 영향 |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 점검 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(세트 매핑/임계 명시, 배율 balance 위임 + 시작값)
- 타입 일관성: `ERuneSet`/`FRuneInstance.RuneSet`/`FRuneSetFormula::ComputeSetBonus`/`GetSetTierBonus`/`runeSet.ts` 전 섹션 일치
- 주의: 세트 보너스 코어/유틸 모두 **순수 보너스값 += 합산**(곱 아님, PR #61 교훈) / 유틸은 캡 외 별도 가산 / 집계는 범용 6슬롯 한정(직업 슬롯 제외)
