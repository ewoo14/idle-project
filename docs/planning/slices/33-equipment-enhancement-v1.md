# PR #33 기획서 — 장비 강화 행위 (심화)

> PR #32 로 골드가 스테이지 비례로 쏟아지지만 **쓸 곳(골드 싱크)이 없다**. `FItemInstance.EnhanceLevel`(0~5)과 스탯 반영(×(1+lvl×0.1))·서버 미러·`RecordGearEnhanced()` 퀘스트 훅은 이미 있으나, **강화하는 행위 자체**(골드 소모·성공/실패 확률·UI)가 없다. 강화 행위를 추가해 *kill→골드→강화→강해짐→고스테이지* 경제 순환을 완성한다.

## 1. 목표 / DoD
플레이어가 골드를 소모해 장착 장비를 강화(+1)할 수 있고, 강화 레벨이 오를수록 비용이 커지고 성공률이 낮아지며, 성공 시 장비 스탯 보너스가 상승한다.

### DoD 검증
1. 강화 시도: 장착 슬롯 선택 → 비용(현재 레벨 비례) 골드 보유 시 차감 → 성공률 판정 → 성공 시 EnhanceLevel +1(최대 MaxEnhanceLevel), 실패 시 레벨 유지(V1 파괴/하락 없음).
2. 골드 부족 또는 이미 최대 레벨이면 시도 불가(차감 없음).
3. 강화 성공 시 ComputeEquipmentBonus 가 상승(기존 ×(1+lvl×0.1) 재사용) → 캐릭터 스탯 반영.
4. 강화 시도 시 `RecordGearEnhanced()`(퀘스트) 기록.
5. HUD 에서 장착 장비별 레벨/비용/성공률 + 강화 버튼. 서버 EnhanceFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 인벤토리/스탯 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 강화 공식 (메인, C++)
- `EnhanceFormula.h/.cpp`(ItemSystem 또는 GameCore, 순수 static):
  - `MaxEnhanceLevel`(기존 ClampMax=5 일치)
  - `GetEnhanceCost(int32 CurrentLevel)` — 비용 곡선(레벨↑ 비용↑, 예 base×(level+1) 또는 지수 보수적)
  - `GetEnhanceSuccessRate(int32 CurrentLevel)` — 0~1, 레벨↑ 성공률↓(예 0레벨 0.95 … 고레벨 0.4)
### 2.2 강화 행위 (메인, C++)
- `UInventoryComponent`: 장착 장비의 EnhanceLevel 을 증가시키는 변이 API(예 `bool EnhanceEquippedItem(EItemSlot Slot)` — 성공 적용만; 최대치 가드). 장착 아이템 레벨 조회 게터.
- `UIdleGameInstance::TryEnhanceEquipped(EItemSlot Slot)` → 장착 확인 + 현재 레벨 + 비용 계산 → `GetGold() >= 비용` & 레벨<Max 확인 → 골드 차감(AddGold(-비용)) → 성공률 RNG 판정 → 성공 시 Inventory 레벨 증가 → `RecordGearEnhanced()` → 결과 구조체 `FEnhanceAttemptResult{bAttempted,bSuccess,GoldSpent,NewLevel}` 반환 + 델리게이트(OnEnhanceResult) 브로드캐스트(HUD/연출용).
- RNG 는 테스트 가능하게(시드 주입 가능 경로 또는 결정적 판정 함수 분리).
### 2.3 UI (디자이너)
- HUD 강화 패널: 장착 슬롯별 현재 레벨/다음 비용/성공률 + 강화 버튼(HitBox). 골드 부족/최대치 시 비활성 표시. 결과(성공/실패) 피드백(색/텍스트). 로컬라이즈 ko/en.
### 2.4 서버 (백엔드)
- `server/src/core/formulas/enhance.ts`: getEnhanceCost/getEnhanceSuccessRate/MAX_ENHANCE_LEVEL 클라 미러 + parity 테스트. (기존 equipment.ts computeInventoryBonus 와 일관.)
### 2.5 데이터/밸런스
- 비용/성공률 곡선 + MaxLevel 근거 + 문서(05-balance-philosophy). 골드 유입(#32)과 강화 비용 싱크 균형 점검(balance-sim 참고, 선택).
### 2.6 테스트
- 서버 Vitest(공식/미러/parity) + 클라 Automation(비용/성공률 곡선, 골드 부족/최대치 가드, 성공 시 레벨·스탯 상승, RecordGearEnhanced 호출, 결정적 판정).

## 3. 범위 외
- 강화 실패 시 파괴/레벨 하락/보호권(후속), 강화 재료 아이템(골드만 V1), 잠재능력/추가옵션, 강화 연출 파티클/사운드(외부).
- 서버 권위 강화 정산(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EnhanceFormula + Inventory/GameInstance 강화 행위 + RNG + Automation | ✅ 메인 (`character`) |
| 백엔드 | enhance.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 강화 HUD 패널 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 비용/성공률 곡선 + 골드 싱크 균형 + 문서 | ✅ 보조 (`balance`) |
| QA | 강화 성공/실패/골드부족/최대치 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 골드 음수/이중 차감 | TryEnhanceEquipped 에서 보유≥비용 선확인 후 1회 차감, 시도/성공 분리 테스트 |
| RNG 비결정성 → 테스트 불가 | 판정 함수 분리(성공률 입력) + 시드/주입 경로, 결정적 단위 테스트 |
| 비용-유입 불균형(싱크 과/소) | #32 골드 유입과 비용 곡선 함께 점검(balance), 보수적 곡선 |
| 서버↔클라 공식 parity | EnhanceFormula DefinitionParity 확장 |
| 기존 인벤토리/스탯 회귀 | EnhanceLevel/ComputeEquipmentBonus 기존 로직 보존, 변이 API만 추가 |

## 7. 후속
- 강화 실패 파괴/하락/보호권, 강화 재료, 잠재능력/추가옵션, 강화 연출(외부), 서버 권위 정산.
