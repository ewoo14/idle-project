# 레어도 7단계 재정의 구현 계획 — PR #65

> **실행 방식:** 워크플로우 v3 [2] Codex 작업 명세. character가 대규모 횡단 변경(클라+서버+도감+저장+테스트), [3] Claude TM이 전 공식 parity·마이그레이션 집중 검증. **`grep EItemRarity`/`ItemRarity`/`Uncommon` 전수**로 누락 방지.

**목표:** 레어도 6단계 → 7단계(Common/Rare/Epic/Unique/Legendary/Transcendent/Mythic), 모든 공식·저장·서버·UI 정합 + 기존 세이브 마이그레이션.

**아키텍처:** `EItemRarity` enum 재정의 + `FRarityMigration` 이름 기준 변환 함수. 전 레어도 비례 공식에 Unique/Transcendent 보간 + Uncommon 제거. 룬 도감 54→63셀. 저장 `SaveVersion` 6→7.

**기술 스택:** UE5 C++ + TS/vitest. `Math.fround` parity. 기존 모든 레어도 공식 패턴.

---

## 인터페이스 계약

### 1. EItemRarity (ItemTypes.h)
```cpp
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    None = 0 UMETA(Hidden),
    Common = 1 UMETA(DisplayName = "Common"),
    Rare = 2 UMETA(DisplayName = "Rare"),
    Epic = 3 UMETA(DisplayName = "Epic"),
    Unique = 4 UMETA(DisplayName = "Unique"),
    Legendary = 5 UMETA(DisplayName = "Legendary"),
    Transcendent = 6 UMETA(DisplayName = "Transcendent"),
    Mythic = 7 UMETA(DisplayName = "Mythic")
};
```
⚠️ **Uncommon 완전 제거** — `grep Uncommon` 전수 제거(컴파일 에러로 잔존 검출). 모든 switch/case에 Unique/Transcendent 추가(누락 시 -Wswitch 경고).

### 2. FRarityMigration (신규 RarityMigration.h/.cpp 또는 ItemTypes)
```cpp
struct IDLEPROJECT_API FRarityMigration
{
    // 기존 6단계 정수 값 → 신규 7단계 EItemRarity. 이름 기준(Uncommon→Rare 승급).
    static EItemRarity MigrateLegacy(int32 LegacyValue);
    //  1→Common(1) / 2(Uncommon)→Rare(2) / 3(Rare)→Rare(2) / 4(Epic)→Epic(3)
    //  / 5(Legendary)→Legendary(5) / 6(Mythic)→Mythic(7) / 그 외→None
};
```

### 3. 마이그레이션 적용 (IdleGameInstance::ApplyFromSave)
```cpp
if (SaveGame->SaveVersion < 7)
{
    for (FItemInstance& Item : SaveGame->InventoryItems) Item.Rarity = FRarityMigration::MigrateLegacy((int32)Item.Rarity);
    for (FRuneSaveEntry& Rune : SaveGame->Runes) Rune.Rarity = FRarityMigration::MigrateLegacy((int32)Rune.Rarity);
    // RuneCodex 셀 키: 기존 Uncommon(2) 셀 → Rare 병합, 기존 Rare(3)~Mythic(6) → 신규 값 재매핑 (도감 §6)
}
// 적용 후 SaveVersion = 7
```
※ enum 재정의로 **저장된 정수 값의 의미가 바뀌므로**, 반드시 SaveVersion<7 게이트에서 한 번만 변환(이미 7이면 변환 금지 — 이중 변환 방지).

### 4. 전 공식 7단계 처리 (각 파일 grep 후 일괄)
각 레어도 비례 공식: **Uncommon case 제거 + Unique(4)·Transcendent(6) case 추가**(기존 곡선 보간, 단조 증가 유지). 클라↔서버 동일 값.
- `DropFormula.cpp`: `GetRarityStatMultiplier`(Common~Mythic 배수, Unique=Epic·Legendary 사이/Transcendent=Legendary·Mythic 사이 보간), `RollRarityForLevel`(Uncommon 제거, Unique/Transcendent 확률 추가, **합=1.0 유지**), 색
- `EnhanceFormula.cpp`: `RarityCostMultiplier`(Common1/Rare2/Epic4/Unique?/Legendary16/Transcendent?/Mythic? — 기존 ×2 곡선 연장: Unique 8, Legendary 16, Transcendent 32, Mythic 64)
- `RuneFormula.cpp`: `GetRarityTuning`(CoreBase/CoreStep/UtilBase/DisenchantBase에 Unique/Transcendent 보간), `RollRuneRarity`(7단계)
- `ClassRuneFormula.cpp`: `GetClassRuneCraftCost`(Unique/Transcendent)
- `SetBonusFormula`/`RuneSetFormula`: 레어도 게이트 7단계(RollItemSet/RollRuneSet None/Common 제외 유지)
- `RuneCodexFormula`: `GetRowCompletionBonus`(7레어도) + 도감 셀 수(§6)

### 5. 서버 미러 (server/src/core/formulas/*.ts)
전 공식 `.ts` ItemRarity 1~6 → 1~7: `drop.ts`/`enhance.ts`/`setBonus.ts`/`rune.ts`/`runeCodex.ts`/`runeSet.ts`/`classRune.ts`. 상수 배열 7요소(인덱스 0=None..7=Mythic). Uncommon 제거. `Math.fround` parity. 클라우드 `save.schema`/`validateSavePayload` grade 0~7.

### 6. 룬 도감 63셀 (RuneCodexFormula + RuneService)
```cpp
static constexpr int32 TotalCells = 63;       // 9 * 7 (54→63)
static constexpr int32 CoreCategoryCells = 35; // 5 * 7
static constexpr int32 UtilCategoryCells = 28; // 4 * 7
```
- `UnlockCodexCell`/도감 사전 채우기 63셀(9타입 × 7레어도). 행 보너스 7레어도(Unique/Transcendent 행 추가).
- 도감 마이그레이션: 기존 54셀(6레어도) → 63셀(7레어도). 기존 Uncommon 행 unlock → Rare 행 병합, 기존 Rare~Mythic unlock → 신규 레어도 재매핑. 신규 Unique/Transcendent 행 미unlock.

### 7. IdleSaveGame.h
```cpp
UPROPERTY() int32 SaveVersion = 7; // 6 → 7
```

### 8. 서버 runeCodex.ts 도감 상수
`RUNE_CODEX_TOTAL_CELLS = 63`, `RUNE_CODEX_CORE_CELLS = 35`, `RUNE_CODEX_UTIL_CELLS = 28`. 클라 동일.

---

## 신규 등급 보간 가이드 (balance 조정)
- 스탯 배수: Common 1.0 < Rare < Epic < **Unique** < Legendary < **Transcendent** < Mythic(기존 Mythic 4.5) 단조 증가. 기존 Epic/Legendary/Mythic 사이 보간.
- 강화 비용 배수: Common 1 / Rare 2 / Epic 4 / **Unique 8** / Legendary 16 / **Transcendent 32** / Mythic 64 (×2 곡선).
- 드롭 확률: 7단계 합=1.0, 고레벨일수록 상위 비중↑. Unique<Epic, Transcendent<Legendary 희소.
- 룬 코어 base/step: 기존 6단계 곡선에 Unique/Transcendent 보간.
- 초기값 character 보간 → balance 시뮬 조정.

---

## 테스트 케이스

### 클라 Automation
- `FRarityMigration::MigrateLegacy`: 1→Common, 2→Rare, 3→Rare, 4→Epic, 5→Legendary, 6→Mythic, 0/8+→None
- ApplyFromSave v6 세이브(Uncommon/Rare/Mythic 아이템) → 마이그레이션 후 신규 등급. SaveVersion 7.
- 이중 변환 방지: SaveVersion 7 세이브 재적용 시 변환 안 함
- 신규 등급 드롭/스탯/강화: Unique/Transcendent 단조 증가
- 도감 63셀: 9×7 사전 채우기, 카테고리 35/28
- Uncommon 제거 회귀: 기존 등급 동작 보존(Common/Rare/Epic/Legendary/Mythic)
- 라운드트립: 신규 등급 아이템/룬 Capture-Restore

### 서버 vitest
- 전 공식 7단계 parity(drop/enhance/rune/runeCodex/runeSet/classRune/setBonus) 클라 앵커 일치
- 마이그레이션 매핑(서버에도 있으면) parity
- 드롭 확률 합=1.0
- grade 0~7 검증

---

## Codex 작업 분배
| 파트 | 작업 |
| --- | --- |
| character (메인) | EItemRarity 7단계 + FRarityMigration + ApplyFromSave + 전 공식 클라 보간(Uncommon 제거) + 도감 63셀 + 저장 v7 + 서버 미러 전 .ts + Tests |
| backend | 클라우드 grade 0~7 + 서버 공식 parity 검증 보강 |
| designer | 레어도 색 7(Unique/Transcendent) + 표시명 7 ko/en + HUD 정합 |
| balance | 7단계 드롭/스탯/강화/도감 행 보간 시뮬 + 파워크리프 가드 |
| qa | 마이그레이션 매핑·라운드트립·도감 63셀·Uncommon 제거 회귀 |

## 워크플로우 v3
[1] ✅ 기획·계획 + PR → [2] character(+보조) → [3] Claude TM(parity 집중) → [4] Codex fix(필요시) → [5] 검증 → [N] CI 그린 + 머지

## Self-Review
- DoD 1~8 매핑 ✅
- placeholder 없음(매핑/보간 가이드 명시, 정밀 배율 balance 위임)
- 타입 일관성: `EItemRarity`(7) / `FRarityMigration::MigrateLegacy` / 도감 63/35/28 / SaveVersion 7 전 섹션 일치
- 주의: **enum 정수 의미 변동 → SaveVersion<7 게이트 1회 변환(이중 변환 금지)** / Uncommon grep 전수 제거 / 전 공식 클라↔서버 동시(parity)
