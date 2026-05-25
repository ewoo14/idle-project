# PR #9 기획서 — 인벤토리 + 장비 V1 (원 슬라이스 ID: S4, M2)

> **순서 복귀**: PR #6 (자동 전투 V1) + PR #7 / PR #8 (hotfix) 머지 + 사용자 PIE 검증 완료 (캐릭터 + 슬라임 + 바닥 + HUD 정상). M2 마일스톤 첫 슬라이스 진입.

---

## 1. 목표 / DoD

UE5 5.7 에디터에서 PIE 시작 시:
1. 캐릭터가 슬라임 사냥 → 슬라임 사망 시 **장비 드롭 (확률 5%)** + 골드 동시 드롭
2. 드롭된 장비를 캐릭터가 자동 흡수 → 인벤토리 추가
3. 드롭 장비가 현재 장착보다 강하면 **자동 장착** (간단한 점수 비교)
4. 장착 시 캐릭터 능력치 (Atk/Def/HP) 자동 갱신 → 다음 전투에 반영
5. **HUD 좌하단** 에 현재 무기 / 방어구 표시 (등급 색상 + 옵션 줄)
6. (옵션) `I` 키 → 인벤토리 위젯 전체 열기 / 닫기
7. UE Automation 테스트: 장비 드롭 → 자동 흡수 → 자동 장착 → 능력치 변화 시뮬레이션 검증

### DoD 검증 시나리오
- 캐릭터가 슬라임 5마리 사냥 → ~1-2 장비 드롭 (5% × 5마리 → 평균 0.25개, 운에 따라 0~2)
- 드롭된 장비가 PIE 뷰포트에 placeholder mesh 로 보임 (큐브, 색상은 등급별)
- 자동 흡수 후 캐릭터 능력치 변경 — Output Log: `[Inventory] 장착: 강철 검 (Common, ATK+5)`
- HUD: `무기: 강철 검 (Common, ATK+5)`

## 2. 범위 (In Scope)

### 2.1 ItemSystem 모듈 확장
- `client/Source/IdleProject/ItemSystem/ItemTypes.h`:
  - `enum class EItemSlot : uint8` — None=0, Weapon=1, Helmet, Top, Bottom, Shoes, Gloves, Cloak, Accessory (8 슬롯)
  - `enum class EItemRarity : uint8` — None=0, Common, Uncommon, Rare (MVP — Epic/Legendary/Mythic 후속 PR)
  - `USTRUCT FItemInstance` — ItemId, Slot, Rarity, Name (FText), Atk/Def/Hp 보정, EnhanceLevel (0~5)
  - `static int32 ComputePowerScore(const FItemInstance&)` (자동 장착 판정용)

- `client/Source/IdleProject/ItemSystem/InventoryComponent.h/cpp`:
  - `UInventoryComponent : UActorComponent`
  - TArray<FItemInstance> Items (최대 100칸)
  - EquippedItems[8] (슬롯별 1개)
  - `void AddItem(const FItemInstance&)` — 자동 장착 비교 + 인벤토리 추가
  - `void EquipItem(int32 ItemIndex)`, `void UnequipSlot(EItemSlot)`
  - `FDerivedStats ComputeEquipmentBonus()` — 장착 아이템 합계 (StatFormulas::DeriveStats 의 EquipmentBonus 인자로 전달)
  - DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedChanged, EItemSlot, Slot);

- `client/Source/IdleProject/ItemSystem/EquipmentDrop.h/cpp`:
  - `AEquipmentDrop : AActor` (AGoldDrop 와 비슷 패턴)
  - FItemInstance Payload + Mesh (Cube, 등급 색상)
  - Tick: 가장 가까운 캐릭터로 VInterpTo
  - 거리 50 이내: InventoryComponent.AddItem + Destroy

- `client/Source/IdleProject/ItemSystem/ItemFactory.h/cpp`:
  - `static FItemInstance RandomDropFromMonster(int32 MonsterLevel)`:
    - 70% Common / 20% Uncommon / 8% Rare (Epic/Leg/Myth 0% PR #9, 후속 PR)
    - 랜덤 슬롯 (Weapon 50% 가중, 방어구 균등)
    - ATK/DEF 보정값 = MonsterLevel × (1.0 ~ 2.0 random)
    - 이름: `{Rarity Adj} {Slot Noun}` (예: "정련된 검", "거친 신발") — 한글 placeholder

### 2.2 CharacterSystem 확장
- `AIdleCharacter`:
  - `UInventoryComponent* Inventory` 추가 (CreateDefaultSubobject)
  - BeginPlay 끝에 `RefreshDerivedStats()` 호출 — Combat 능력치를 StatFormulas + InventoryBonus 합계로 갱신
  - InventoryComponent.OnEquippedChanged 구독 → RefreshDerivedStats 자동 호출
- `AIdleMonster`:
  - Combat->OnDeath 콜백 갱신 — 5% 확률로 EquipmentDrop spawn + 기존 GoldDrop 도 유지

### 2.3 UI 확장
- `SIdleHUDWidget` 갱신:
  - 좌하단 VBox 추가: "무기", "방어구 (등급)", "강화" — 현재 장비 요약
  - InventoryComponent.OnEquippedChanged 구독 → 자동 갱신
- (옵션) `SInventoryWidget` 신규 — `I` 키로 토글, 8슬롯 그리드, 각 슬롯에 ItemInstance 표시. 본 PR 단계에서는 placeholder OK.
- `UIThemeTokens` — Rarity 색상 (Common, Uncommon, Rare) 이미 정의됨 (PR #1 ui-tokens.json), C++ 미러 활용

### 2.4 데이터 / 밸런스
- `client/Content/Data/ItemDB.csv` 신규:
  - 8슬롯 × 3등급 = 24행 (기본 템플릿) + 옵션 추가
  - 컬럼: ItemId, NameKr, Slot, Rarity, BaseAtk, BaseDef, BaseHp, MaxEnhance
- `docs/planning/05-balance-philosophy.md` 부록에 "M2 인벤토리 V1 수치" 추가:
  - 등급별 ATK/DEF 평균 (Common 5~10, Uncommon 10~20, Rare 20~40)
  - 강화 +1 당 +10% (PR #1 §2.3 미러)
  - 자동 장착 점수: PowerScore = (Atk + Def + Hp/10) × (1 + EnhanceLevel × 0.1)

### 2.5 서버 미러 (최소)
- `server/src/core/formulas/equipment.ts` 신규:
  - `computeItemPowerScore(item: ItemInstance): number` — UE5 미러
  - `computeInventoryBonus(items: ItemInstance[]): EquipmentBonus` (PR #10 서버 검증 시 사용 예정)
  - vitest 단위 테스트 5~8건
- DB 스키마 변경 없음 (characters.inventory JSONB 가 이미 PR #2 에 존재)

### 2.6 테스트
- `client/Source/IdleProject/Tests/InventoryTests.cpp`:
  - 자동 장착 (Common 무기 → Uncommon 무기 교체)
  - 점수 비교 결정성
  - 강화 보너스 계산
- AutoBattle 시뮬레이션: 캐릭터 + 몬스터 + EquipmentDrop 흐름 (단순 단위)

## 3. 범위 외 (Out of Scope)

| 항목 | 시점 |
| --- | --- |
| Epic / Legendary / Mythic 등급 | PR #11 (M2 후반) 또는 PR #12 (M3 메타) |
| 강화 +6 ~ +15 (확률 강화) | PR #11 |
| 잠재 옵션 (RNG 옵션 줄) | PR #11 |
| 세트 효과 | PR #12 (다직업 추가 시) |
| 장비 판매 / 교환소 | PR #13 (M4 상점) |
| 장비 비교 UI | PR #11 |
| 인벤토리 전체 위젯 (I 키) | 본 PR 옵션 — placeholder OK |
| 서버 측 장비 검증 (재계산) | PR #14 (M3 백엔드 V2) |
| BP/.uasset 본격 (Game.umap, W_MainMenu) | 별도 chore PR |
| 사운드 (장비 흡수, 장착) | M5 |
| Niagara 이펙트 (장비 드롭) | M5 |

## 4. 7파트 작업 분배 — Codex 호출 계획

| 파트 | 작업 | Codex 호출 |
| --- | --- | --- |
| **캐릭터·아이템·능력치 (메인)** | ItemSystem 모듈 전체 + AIdleCharacter 통합 + AEquipmentDrop + ItemFactory + Tests | ✅ 메인 |
| 백엔드·DB | server `core/formulas/equipment.ts` + vitest | ✅ 보조 |
| 밸런스 | ItemDB.csv (24행) + balance-philosophy.md 부록 | ✅ 보조 |
| 디자이너 | SIdleHUDWidget 좌하단 장비 표시 + (옵션) SInventoryWidget | ✅ 보조 |
| QA | docs/qa/scenarios/M2-inventory-v1.md + regression-checklist §1/§4 | ✅ 보조 |
| 스토리 | N/A (PR 위임) | ❌ |
| 퀘스트 | N/A (PR 위임) | ❌ |

→ 총 Codex 실호출 **2회** (캐릭터 메인 + 보조 4 합동)

## 5. 호출 순서

1. **캐릭터 메인** — ItemSystem 전체 + Character/Monster 통합 + EquipmentDrop + Tests (~30~50분)
2. **보조 4 합동** — server equipment.ts / ItemDB.csv / SIdleHUDWidget 갱신 / QA 시나리오 (~15분)

## 6. 워크플로우 v3 (PR #6 학습 반영)

- 단계 [2] 후 PM 이 Codex 산출 PR 코멘트 게시
- 단계 [3] Claude 리뷰 + fix → 단계 [4] Codex 리뷰 + fix → 단계 [5] Claude 검증 → [N] 머지
- Codex TM 프롬프트 단순화 유지

## 7. 일정 (잠정)

- [1] 본 문서 (완료)
- [2] Codex 2회 호출 → 45~65분
- [3] Claude 리뷰 + fix → 10분
- [4] Codex 리뷰 + fix → 10~15분
- [5] Claude 검증 → 5분
- [N] PM 종합 + 머지 → 5분
- **총: 약 1.5~2시간**

## 8. 리스크

| 항목 | 위험 | 대응 |
| --- | --- | --- |
| InventoryComponent ↔ Combat ↔ StatFormulas 결합 복잡 | 중 | RefreshDerivedStats() 단일 진입점, OnEquippedChanged 델리게이트로 단방향 흐름 |
| 자동 장착 알고리즘 단순화 | 낮음 | 본 PR PowerScore 단순 비교 (PR #11 비교 UI 시 정밀화) |
| ItemDB CSV 와 ItemFactory 결합 | 중 | ItemFactory 가 CSV 로드 대신 inline 생성 (PR #10 백엔드와 함께 CSV→DataTable 전환) |
| 장비 드롭이 너무 빈번 (인벤토리 폭주) | 낮음 | 5% 드롭률 + 인벤토리 최대 100칸 + 자동 장착으로 회전 |
| HUD 좌하단 추가가 좌상단 HP/EXP 와 겹침 | 낮음 | 별도 VBox 좌하단 위치 명시 |

## 9. 후속 PR 예고

- **PR #10 (M2-S5)** — 스킬 트리 V1 (전사 액티브 4 + 패시브 2 + 궁극기 1)
- **PR #11 (M3-S6)** — 인벤토리 / 장비 V2 (Epic/Legendary/Mythic + 강화 +6~+15 + 잠재 + 비교 UI)
- **PR #12 (M3-S7)** — 오프라인 보상

본 PR 머지 후 메모리 / 마일스톤 문서 PR 번호 컬럼 갱신.
