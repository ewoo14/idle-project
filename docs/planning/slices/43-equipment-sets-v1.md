# PR #43 기획서 — 장비 세트 효과 (심화)

> gear 루프(드롭#36/affix#40/강화#33/상점#38)의 캡스톤. 장비에 **세트** 소속을 부여하고, 같은 세트를 여러 개 장착하면 **세트 보너스**(2세트/4세트)를 준다. 상점 뽑기(#38)에 "세트 완성"이라는 구체적 골드 사용 목표가 생기고, 수집/빌드 깊이가 늘어난다. 보너스는 기존 ComputeEquipmentBonus→DeriveStats 파이프라인에 **flat 추가**로 통합(경제 중립, 보상/골드 무변경). client+server+UI+balance+qa 5-team([[feedback-substantial-slices]]).

## 1. 목표 / DoD
장착 장비 중 같은 세트가 2개/4개 이상이면 세트 보너스가 적용돼 파생 스탯이 오른다.

### DoD 검증
1. 세트: `EItemSet{None, Warrior(공격), Guardian(수호), Arcane(비전)}`. 드롭/상점 장비가 세트 소속을 가짐(ItemFactory 부여, 일부는 None).
2. 세트 보너스: 장착 장비의 세트별 개수 집계 → 2세트/4세트 임계 시 tiered 보너스(예 Warrior 2pc +PhysAtk, 4pc +PhysAtk+CritRate; Guardian 2pc +Def/Hp, 4pc 강화; Arcane 2pc +MagicAtk, 4pc 강화). flat FDerivedStats 추가로 ComputeEquipmentBonus→DeriveStats 반영.
3. 세트 미충족(2개 미만)이면 보너스 0(회귀: 기존 장비 보너스 불변).
4. HUD 가 활성 세트/단계(2/4)와 보너스 표시. 서버 SetBonusFormula 미러 + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 장비/스탯/경제 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 세트 데이터/부여 (메인, C++)
- `EItemSet`(ItemTypes.h): None=0/Warrior=1/Guardian=2/Arcane=3. `FItemInstance` 에 `ItemSet`(EItemSet, 기본 None) 필드.
- `FDropFormula`(또는 ItemFactory): 드롭/상점 아이템에 세트 부여 — rng 로 None/3세트 중 선택(희귀도↑일수록 세트 부여 확률↑ 권장; Common 은 None 위주). RandomDropFromMonster/GuaranteedDropForLevel 통합. 결정적(rng 주입).
### 2.2 세트 보너스 공식/통합 (메인, C++)
- `SetBonusFormula.h/.cpp`(ItemSystem/, 순수 static): `ComputeSetBonus(const TArray<FItemInstance>& EquippedItems)` → FDerivedStats(flat 합). 세트별 개수 집계 → 2pc/4pc tiered 추가. 세트별 보너스 종류(Warrior=PhysAtk/Crit, Guardian=PhysDef/Hp, Arcane=MagicAtk). `GetSetThreshold`(2,4), 세트별 단계 보너스 상수.
- `UInventoryComponent::ComputeEquipmentBonus`: 기존 per-item 합 + SetBonusFormula::ComputeSetBonus(장착 목록) 추가. (세트 보너스는 강화 배수 미적용 — flat 고정.)
### 2.3 UI (디자이너)
- HUD(장착 요약/정보 패널 또는 신규 세트 패널)에 활성 세트별 장착 수/단계(2/4 충족)와 세트 보너스 표시. 로컬라이즈 ko/en(세트명/보너스).
### 2.4 서버 (백엔드)
- `server/src/core/formulas/setBonus.ts`: ItemSet 타입 + computeSetBonus(equippedItems) 미러(세트 집계/임계/보너스) + parity 테스트. equipment.ts ItemInstance 에 itemSet 필드.
### 2.5 데이터/밸런스
- 세트별 2/4세트 보너스 값 + 세트 부여 확률 + 빌드 영향 문서. flat 값이 진행 단계에서 의미있는 범위인지(또는 후속 % 검토 기록).
### 2.6 테스트
- 서버 Vitest(세트 집계/임계/보너스/미러/parity) + 클라 Automation(세트 부여, 2/4세트 보너스, 미충족 0 회귀, ComputeEquipmentBonus→DeriveStats 반영).

## 3. 범위 외
- 수동 장착/슬롯 잠금(자동 장착은 PowerScore 기준 유지 — 세트 보너스는 장착된 것 기준 V1; 수동 세트 맞춤은 후속), % 세트 보너스, 세트 전용 외형/룩, 신규 세트 추가.
- 보상/골드 변경(경제 중립). 서버 권위 세트 정산(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EItemSet + 세트 부여 + SetBonusFormula + ComputeEquipmentBonus 통합 + Automation | ✅ 메인 (`character`) |
| 백엔드 | setBonus.ts 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | 세트 활성/단계/보너스 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 세트 보너스 값/부여 확률 + 빌드 영향 + 문서 | ✅ 보조 (`balance`) |
| QA | 세트 부여/2·4세트/미충족/스탯 반영 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]], [[reference-ci-retrigger]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| 자동 장착(PowerScore)이 세트 무시 | V1 세트 보너스는 장착된 것 기준(부분 세트 자연 발생) + 상점 reroll 로 완성 추구. 수동 장착/세트 가중은 후속(문서 명시) |
| 세트 부여로 기존 드롭/PowerScore 회귀 | ItemSet 기본 None + 부여는 추가 필드만, PowerScore 무변경(세트는 장착 보너스로만). 회귀 테스트 |
| 세트 보너스 flat 이 후반 무의미 | 진행 단계별 값 점검(balance), 후속 % 검토 기록 |
| 서버↔클라 parity(집계/임계/보너스) | SetBonusFormula DefinitionParity 확장(rng 주입/정수) |
| ComputeEquipmentBonus 회귀 | 세트 미충족 0 → 기존 per-item 합 동일, 추가만 |

## 7. 후속
- 수동 장착/세트 가중 자동장착, % 세트 보너스, 세트 전용 외형, 신규 세트, 서버 권위 정산.
