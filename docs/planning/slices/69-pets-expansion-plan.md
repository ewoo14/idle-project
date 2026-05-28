# 펫 시스템 대확장 구현 계획 — PR #69

> **실행 방식:** 워크플로우 v3 [2] Codex 명세. character가 클라+서버 미러+테스트 통합, [3] Claude TM 검증, [5] **UE Automation 직접**(#68 교훈). PM 자율. 기존 펫(#22 PetService/#42 FeedPet 레벨업) 패턴 재사용.

**목표:** 펫 2→10종, 보너스 타입 다양화(Gold/Drop/Exp/스탯류), 스탯류 % 곱.

**아키텍처:** `EPetBonusType` 확장 + 10종 카탈로그(`BuildDefaultDefinitions`). 보너스 적용: Gold/Drop 기존 + Exp(reward 훅) + 스탯류(`RefreshDerivedStats` % 곱). 펫 레벨(#42) 배수 정합.

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 PetService/Save 패턴.

---

## 인터페이스 계약

### 1. PetService.h — EPetBonusType 확장
```cpp
enum class EPetBonusType : uint8
{
    None = 0 UMETA(Hidden),
    Gold = 1, Drop = 2,
    Exp = 3,        // 경험치 획득 +%
    PhysAtk = 4,    // 물공 +% (스탯류)
    MagicAtk = 5,   // 마공 +%
    Hp = 6,         // 체력 +%
    Def = 7,        // 물방·마방 +%
    AllStat = 8     // 물공·마공·물방·마방 +%
};
```
`FPetDefinition` 기존 유지(PetId/Name/BonusType/BonusPercent).

### 2. 펫 카탈로그 (PetService.cpp BuildDefaultDefinitions)
```cpp
AddDefinition("dog",    "Dog",    EPetBonusType::Gold,    20.0f); // 기존
AddDefinition("bird",   "Bird",   EPetBonusType::Drop,    15.0f); // 기존
AddDefinition("cat",    "Cat",    EPetBonusType::Exp,     15.0f);
AddDefinition("wolf",   "Wolf",   EPetBonusType::PhysAtk, 10.0f);
AddDefinition("owl",    "Owl",    EPetBonusType::MagicAtk,10.0f);
AddDefinition("bear",   "Bear",   EPetBonusType::Hp,      12.0f);
AddDefinition("turtle", "Turtle", EPetBonusType::Def,     12.0f);
AddDefinition("fox",    "Fox",    EPetBonusType::Gold,    30.0f);
AddDefinition("rabbit", "Rabbit", EPetBonusType::Drop,    25.0f);
AddDefinition("dragon", "Dragon", EPetBonusType::AllStat, 8.0f);
```
값은 balance 튜닝. (해금 정책: 기존 InitializeDefaultPets가 전부 Owned면 유지, 또는 dog/bird만 기본+나머지 골드 해금 — character 결정, 회귀안전.)

### 3. 보너스 조회 (PetService)
```cpp
// 기존 유지: GetEquippedPetGoldBonusPercent / GetEquippedPetDropBonusPercent / ApplyGoldBonus / ApplyDropBonusChance
// 신규: 장착 펫의 보너스 타입별 유효 % (레벨 #42 배수 포함)
float GetEquippedPetExpBonusPercent() const;        // Exp 펫일 때 유효 %, 아니면 0
FPetStatBonus GetEquippedPetStatBonus() const;      // 스탯류 펫 → 스탯별 % (PhysAtk/MagicAtk/Hp/Def), 아니면 0
// FPetStatBonus { float PhysAtkPct; float MagicAtkPct; float PhysDefPct; float MagicDefPct; float HpPct; }
```
유효 % = `BonusPercent/100 * (1 + level*levelMultiplier)`(기존 #42 GetEffectiveBonusPercent 패턴).

### 4. 보너스 적용
- **Exp**: `IdleMonster` 경험치 지급(룬 #61 GetRuneExpBoostBonus 패턴 옆) `×(1 + GameInstance->GetEquippedPetExpBonusPercent()/100 또는 비율)`.
- **스탯류**: `RefreshDerivedStats`에서 펫 스탯 보너스 % 곱 — 룬 코어(#61)/유니크 trait(#67) 합산 곱 경로에 합류. `Derived.PhysAtk *= (1 + petStat.PhysAtkPct)` 등. **% 곱(flat 아님, PR #67 교훈)**. AllStat → 4스탯, Hp → Hp.
- Gold/Drop: 기존 경로.

### 5. 서버 petBonus.ts
```ts
export interface PetDefinition { id: string; bonusType: number; bonusPercent: number; }
export function getPetCatalog(): PetDefinition[]; // 클라 10종 동일
export function getEffectiveBonusPercent(bonusPercent: number, level: number): number; // #42 패턴, Math.fround
```
카탈로그/보너스 클라 동일. (기존 서버 pet 모듈 있으면 확장.)

### 6. 저장
- OwnedPetIds(TSet<FString>)/PetLevels(TMap) 기존 직렬화. 신규 펫 ID는 기존 세이브에 없음(미보유/레벨0, 회귀안전). SaveVersion bump는 해금 정책에 따라(전부 기본 보유면 불필요).

---

## 테스트 케이스

### 클라 Automation
- 펫 카탈로그 10종 정의(BonusType/Percent)
- Gold 펫 장착 → ApplyGoldBonus, Drop 펫 → DropBonusChance(기존)
- Exp 펫 장착 → GetEquippedPetExpBonusPercent > 0, 경험치 지급 반영
- **스탯류 펫(wolf PhysAtk) 장착 → RefreshDerivedStats PhysAtk % 증가(곱), 타 스탯 불변. AllStat(dragon) → 4스탯 % 증가. flat 아님 검증(베이스 대비 비율)**
- 펫 레벨(#42) 배수가 신규 보너스 타입에 적용
- 해금/장착(신규 펫) + 저장 라운드트립 + 기존 세이브 회귀안전

### 서버 vitest
- getPetCatalog 10종 클라 동일(id/bonusType/percent)
- getEffectiveBonusPercent 레벨 배수 parity(Math.fround)

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | EPetBonusType 확장 + 10종 카탈로그 + 보너스 적용(Exp 훅/스탯류 % 곱/Gold·Drop) + 해금/장착 + 저장 + 서버 petBonus.ts + Tests |
| designer | 펫 패널 10종 HUD + 로컬라이즈 ko/en(펫명/보너스) |
| balance | 펫 보너스 값/스탯류 % + 파워크리프 가드 |
| (backend/qa) | character 흡수, [3] Claude TM parity·커버리지 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+designer/balance) → [3] Claude TM → [4] fix(필요시) → [5] 검증(UE Automation 직접) → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(카탈로그/보너스 가이드 명시, 정밀 수치 balance 위임)
- 타입 일관성: `EPetBonusType`(8)/10종 카탈로그/`FPetStatBonus`/`petBonus.ts` 전 섹션 일치
- 주의: **스탯류 펫 % 곱(flat 아님, PR #67 교훈)** / 레벨 배수(#42) 정합 / 클라↔서버 parity / [5] UE Automation 직접(#68 교훈)
