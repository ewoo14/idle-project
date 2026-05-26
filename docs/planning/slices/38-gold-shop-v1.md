# PR #38 기획서 — 골드 상점 (장비 뽑기) (심화)

> #33 밸런스에서 *엔드게임 골드가 강화로 다 소모되지 않음(baseline pressure 0.017h)* 이 지적됐다. #36 으로 드롭/장비가 깊어진 지금, **골드로 장비를 뽑는 상점**을 추가해 남는 골드를 gear 루프(드롭→강화 #33→스탯)로 환원하는 골드 싱크를 만든다. 상점 장비는 현재 스테이지 레벨로 생성되고 항상 아이템이 나온다(보장).

## 1. 목표 / DoD
플레이어가 골드를 소모해 현재 스테이지 수준의 장비를 1개 뽑을 수 있고(항상 아이템 획득), 비용은 진행도(스테이지)에 비례한다.

### DoD 검증
1. 장비 뽑기: 골드 ≥ 비용(현재 GlobalStageIndex 비례) 보유 시 차감 → 현재 스테이지 몬스터 레벨(GetMonsterLevelForStage) 장비 1개 생성(희귀도는 DropFormula 확률, **None 없이 보장**) → 인벤토리 추가(AddItem, 기존 자동 장착 정책 적용).
2. 골드 부족 시 구매 불가(차감 없음).
3. 뽑기 결과(희귀도/이름/슬롯/골드 소모) 반환 + 델리게이트(OnShopPurchase) 브로드캐스트.
4. HUD 상점 패널(비용/뽑기 버튼/최근 결과). 서버 ShopFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 인벤토리/경제 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 상점 공식 (메인, C++)
- `ShopFormula.h/.cpp`(ItemSystem 또는 GameCore, 순수 static):
  - `GetGearRollCost(int32 GlobalStageIndex)` → int64 비용. 진행도 비례(예 BaseCost 300 × StageRewardMultiplier(idx) 또는 300×(1+idx×0.15)). 보수적(밸런스 점검).
### 2.2 상점 행위 (메인, C++)
- `FItemFactory` 보장 드롭: 상점용으로 None 없이 아이템을 보장하는 경로(예 `GuaranteedDropForLevel(int32 Level)` — 기존 RollRarity 가 None 이면 Common 으로, 또는 reroll). 기존 RandomDropFromMonster(드롭 실패 None 가능)는 유지.
- `UIdleGameInstance::TryBuyGearRoll()` → `FShopPurchaseResult{bool bPurchased; int64 GoldSpent; EItemRarity Rarity; EItemSlot Slot; FText ItemName;}`:
  - 현재 스테이지(GetStageService()->GetGlobalStageIndex()) → 비용 = GetGearRollCost. GetGold() < 비용 → bPurchased=false(차감 없음).
  - AddGold(-비용) 1회 → 레벨 = FRewardFormula::GetMonsterLevelForStage(GlobalStageIndex) → FItemFactory 보장 드롭 생성 → FindPlayerInventory()->AddItem(Item). 결과 채움. OnShopPurchase 브로드캐스트.
  - 인벤토리 부재 시 bPurchased=false(차감 전 확인) — 차감 후 인벤토리 없음 방지 위해 인벤토리 확인 후 차감.
  - 테스트 가능하게 인벤토리 주입 오버로드(TryEnhanceEquipped 패턴) + 결정적 rng 경로(기존 드롭 rng 또는 주입).
### 2.3 UI (디자이너)
- HUD 상점 패널: 현재 뽑기 비용 + 뽑기 버튼(HitBox, 예 "ShopGearRoll") + 최근 결과(희귀도 색/이름). 골드 부족 시 버튼 비활성. 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/shop.ts`: getGearRollCost(globalStageIndex) 클라 미러(Math.fround로 float parity) + parity 테스트.
### 2.5 데이터/밸런스
- 비용 곡선 vs 골드 유입(#32)·강화 싱크(#33) 균형 점검(상점이 주 골드 싱크가 되도록) + 문서. balance-sim 반영 선택.
### 2.6 테스트
- 서버 Vitest(비용/미러/parity) + 클라 Automation(비용 곡선, 골드 부족 가드, 구매 시 차감+인벤토리 추가, 보장 드롭 None 없음, 결과 구조체).

## 3. 범위 외
- 상점 새로고침/재고 슬롯, 고정 상품/특별 상품, 보장 희귀도 프리미엄 뽑기(후속), 유료/캐시 상점·재화 다양화.
- 강화 재료/소비 아이템 판매(소비 아이템 시스템 부재 — 후속).
- 서버 권위 상점 정산(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | ShopFormula + 보장 드롭 + GameInstance.TryBuyGearRoll + Automation | ✅ 메인 (`character`) |
| 백엔드 | shop.ts 비용 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 상점 HUD 패널 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 비용 곡선 vs 유입/싱크 균형 + 문서 | ✅ 보조 (`balance`) |
| QA | 뽑기 성공/골드부족/보장 드롭/인벤토리 추가 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 골드 음수/이중 차감/인벤토리 부재 차감 | 인벤토리·보유 골드 선확인 후 1회 차감, 시도/성공 분리 테스트 |
| 보장 드롭이 None 반환(빈 아이템) | GuaranteedDropForLevel None→Common 보정 + 테스트 |
| 비용-유입 불균형(싱크 과/소) | #32 유입·#33 강화와 함께 점검(balance), 진행도 비례 보수 곡선 |
| 서버↔클라 비용 parity | ShopFormula DefinitionParity 확장(Math.fround) |
| 기존 인벤토리/자동장착 회귀 | AddItem 기존 정책 재사용, 시그니처 불변 |

## 7. 후속
- 상점 새로고침/재고, 보장 희귀도 프리미엄, 강화 재료/소비 아이템, 재화 다양화, 서버 권위 정산.
