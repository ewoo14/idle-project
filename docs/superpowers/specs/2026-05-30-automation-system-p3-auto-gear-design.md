# 통합 자동화 시스템 P3 — 자동 장비 관리 + 최소 판매 설계 스펙

- 작성일: 2026-05-30
- 상태: 설계 승인됨(단일 MaxRarity) → plan 단계화
- 부모 스펙: `docs/superpowers/specs/2026-05-30-automation-system-design.md` (§5③)
- 선행: P1(PR #99 `5f1fa38`) + P2(PR #100 `5994d98`). 브랜치 `feat/automation-system-p3`.

## 1. 배경

장비 인벤토리는 손이 많이 간다. 드랍이 쌓이고, 더 좋은 장비로 갈아끼우고, 잡템을 정리하는 노가다가 자동화되어 있지 않다. 게다가 **장비를 파는 기능 자체가 없다**(인벤 정리 = 방치 불가).

현재(`UInventoryComponent`): `AddItem`/`EquipItem`/`UnequipSlot`/`GetEquippedItem`/`ComputeEquipmentBonus`/`SetItemLocked(bLocked)`/`CaptureState`. 아이템 `FItemInstance`: Slot/Rarity/EnhanceLevel/PotentialGrade/UniqueTrait1·2/bLocked/보너스. 몬스터 드랍은 `EquipmentDrop.cpp`가 캐릭터 접촉 시 `Inventory->AddItem(Payload)`. **판매/매각가 공식 없음.**

P3는 (1) 최소 판매 기본 기능을 신설하고, (2) 자동 장착, (3) 자동 매각(드랍 필터)을 정책으로 붙인다.

## 2. 목표 / 비목표

### 목표
1. **최소 판매 기본 기능(신규)**: parity 매각가 공식 + `InventoryComponent::SellItem` + GameInstance 진입점. 수동·자동 공용.
2. **자동 장착**: parity `computeItemPower` 스칼라 비교로 슬롯별 더 강한 장비 자동 교체. 정책 토글.
3. **자동 매각**: 정책 `AutoSellMaxRarity`(단일) + 토글. 몬스터 드랍이 필터 이하면 자동 매각(골드). 안전 제외 가드.
4. `AutomationPolicy` 확장 + SaveVer 27→28. `AutoGear` 해금 = 챕터 5(P1 게이트 재사용).
5. 자동화 패널 "장비" 탭(데이터 구동).

### 비목표 (P3 제외)
- **자동 강화**: 골드 폭주 위험(부모 §2). 후속.
- 자동 매각 **부위별/다중 필터 슬롯** + 효율 업그레이드 sink → P4.
- 자동 소비/버프 → P4.
- 샵 구매 장비 자동 매각(의도적 구매 보호 — 자동 매각은 **몬스터 드랍만**).

## 3. 데이터 모델

```
// FAutomationPolicySave (P1/P2) 확장 → SaveVer 27 → 28
bool  bAutoEquipByPower = false;   // 자동 장착
bool  bAutoSell = false;           // 자동 매각
EItemRarity AutoSellMaxRarity = EItemRarity::Common; // 이 등급 이하 드랍 자동 매각
```

EItemRarity(7단계, #65): None0/Common1/Rare2/Epic3/Unique4/Legendary5/Transcendent6/Mythic7.

## 4. parity 공식 (서버 신규 `itemSell.ts` ↔ 클라 `ItemSellFormula`)

### 매각가
```
// 등급별 base × 강화 배수. 무한/캡 없음(고강화 고가).
RARITY_SELL_BASE: { Common:100, Rare:400, Epic:1500, Unique:6000,
                    Legendary:25000, Transcendent:100000, Mythic:400000 }  // None=0
computeItemSellValue(rarity, enhanceLevel):
  base = RARITY_SELL_BASE[rarity] (None→0)
  return round(base * (1 + 0.2 * max(0, enhanceLevel)))   // 강화당 +20%
```

### 아이템 파워(자동 장착 비교용 스칼라)
```
// 장비 보너스 가중합 × 강화 배수. 슬롯 간 비교 아님(같은 슬롯 내 비교).
computeItemPower(item):
  raw = item.bonusAtk + item.bonusMagicAtk
      + item.bonusDef + item.bonusPhysDef + item.bonusMagicDef
      + (item.bonusHp + item.bonusAffixHp) / 10
      + item.bonusCritRate * 1000 + item.bonusCritDmg * 100
  return round(raw * (1 + 0.1 * max(0, enhanceLevel)))   // 강화당 +10%
```
- 음수/NaN 가드. 클라 `FItemSellFormula::ComputeSellValue`/`ComputeItemPower` 1:1(Math.fround/round 정합).
- 주: 아이템 파워는 **자동 장착의 같은 슬롯 내 우열 판정용** 스칼라일 뿐, CombatPower(#49)와 별개. 잠재/유니크 trait/룬은 비교에서 제외(MVP — 베이스+강화 기준). 후속 정교화 가능.

## 5. 동작 / 통합

### ① 최소 판매
- `InventoryComponent::SellItem(int32 Index) -> int64 GoldGained`: 인덱스 유효 + **미장착 + 미잠금**일 때만 제거하고 매각가 반환(아니면 0). 장착 슬롯 인덱스 보정(EquippedIndex 갱신).
- `GameInstance::SellInventoryItem(int32 Index)`: `SellItem` → `AddGold(value)`. 수동 판매 BP 진입점.

### ② 자동 장착
- `InventoryComponent::AutoEquipBestPerSlot() -> int32 EquipCount`: 슬롯별 보유 아이템 중 `ComputeItemPower` 최고가 현재 장착품보다 크면 `EquipItem`. 잠긴 장착품도 더 강하면 교체(잠금은 매각 보호용이지 장착 고정 아님 — 단순화). 교체된 구장비는 인벤 유지.
- 정책 `bAutoEquipByPower` ON일 때, 드랍 처리/로그인 시 호출.

### ③ 자동 매각 + 드랍 핸들러 (통합점)
- `GameInstance::HandleDroppedEquipment(UInventoryComponent* Inv, const FItemInstance& Item) -> bool bKept`:
  - 자동 매각 ON && `Item.Rarity <= AutoSellMaxRarity` && **제외 아님** → `AddGold(ComputeSellValue)` 후 false(인벤 미추가).
  - 아니면 `Inv->AddItem(Item)`; 성공 && 자동 장착 ON → `AutoEquipBestPerSlot()`; true 반환.
  - **제외 가드**(자동 매각 안 함): `bLocked || EnhanceLevel>0 || PotentialGrade != None || UniqueTrait1 != None || UniqueTrait2 != None`.
- `EquipmentDrop.cpp`의 몬스터 드랍 픽업이 `Inventory->AddItem(Payload)` 대신 `GameInstance->HandleDroppedEquipment(Inventory, Payload)` 경유. 반환 false면 매각된 것(획득 연출/업적은 추가된 경우만).
- 샵 구매(`IdleGameInstance.cpp` GearRoll)는 **변경 없음**(자동 매각 미적용).

## 6. 세이브 / 해금 / HUD
- `AutomationPolicy` 3필드 추가, SaveVer 27→28(<28=기본 OFF/Common 마이그). `RestoreState` 확장 + 저장 직렬화.
- 해금 `AutoGear`(P1 `IsAutomationFeatureUnlocked`, 챕터 5). 미해금 시 토글 잠금(드랍 핸들러는 정책 OFF로 기존 동작).
- 자동화 패널 "장비" 탭: 자동 장착 토글 / 자동 매각 토글 + 등급 선택 / 수동 판매. 데이터 구동(신규 위젯 코드 0).

## 7. 오류 처리 / 안전 가드
- 매각: 장착/잠금 아이템 거부(0 반환). 인덱스 범위 가드.
- 자동 매각 제외 화이트리스트(잠금/강화/잠재/유니크) — 사고 매각 방지(부모 §8).
- 자동 장착: 더 약한 장비로 교체 금지(파워 비교).
- `Rarity`/`enhanceLevel` 음수/범위 가드. 구버전 세이브(<28)=정책 OFF=기존 동작(드랍 그대로 인벤 추가).
- 실존재화(골드)만. 신규 재화 없음.

## 8. 테스트 / 게이트
- 서버 `itemSell.test.ts`: `computeItemSellValue`/`computeItemPower` 등급·강화·가드.
- 클라 parity `FItemSellFormula` 회귀 + InventoryComponent: SellItem(장착/잠금 거부), AutoEquipBestPerSlot(우열/교체), HandleDroppedEquipment(자동 매각/제외 가드).
- SaveVer 28 round-trip + **26→27→28 stale 일괄 갱신**(P2 교훈: 헤더 기본값 + `CaptureToSave` writer + 테스트 단언 3곳).
- 표준 jumbo + 전체 Automation(`IdleProject.UI.HUD`/Inventory/Item 포함). 서버 biome clean.

## 9. 구현 단계화 (plan)
단일 충실 슬라이스(P1/P2 골격 재사용).
- 서버 `itemSell.ts`(2공식) + 테스트 + index export.
- 클라 `ItemSellFormula` static + 회귀.
- InventoryComponent: SellItem/AutoEquipBestPerSlot + 회귀.
- 정책/세이브 SaveVer 28 + GameInstance(SellInventoryItem/HandleDroppedEquipment/BP 토글) + EquipmentDrop 경유.
- SaveVer 26/27→28 stale 일괄 + HUD 장비 탭 진입점.

## 10. 후속
- P4 자동 소비/버프 + 규칙·필터 슬롯 효율 업그레이드 sink(부위별 필터 등).
- 자동 강화(골드 예산 상한·확률 설계) 별도.
- 아이템 파워 정교화(잠재/유니크/룬 반영), 일괄 매각/분해(에센스 환산).
