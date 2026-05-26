# PR #40 기획서 — 아이템 추가 옵션 affix (심화)

> 드롭/강화/상점(#36/#33/#38)으로 gear 루프는 깊어졌지만, 장비 스탯은 슬롯 고정형(공/방/체)뿐이라 **빌드 다양성**이 없다. 희귀도가 높을수록 **추가 옵션(affix: 크리율/공속/마법공격)** 이 붙게 해 크리·속도·마법 빌드 등 선택지를 더한다. 골드/보상을 늘리지 않아 **경제 중립**이며, `FDerivedStats` 가 이미 CritRate/AtkSpeed/MagicAtk 를 EquipmentBonus 로 전파하므로 통합이 깔끔하다.

## 1. 목표 / DoD
드롭/상점 장비에 희귀도에 따라 추가 옵션(크리율·공속·마법공격)이 붙고, 장착 시 해당 파생 스탯이 상승한다.

### DoD 검증
1. affix 롤: 희귀도↑ → 추가 옵션 개수/크기↑(예 Common 0 / Uncommon 1 / Rare 1 / Epic 2 / Legendary 2~3). 옵션 종류: 크리율(+%)/공속(+)/마법공격(+flat). 결정적(rng 주입).
2. 장착 시 ComputeEquipmentBonus 가 affix 를 CritRate/AtkSpeed/MagicAtk EquipmentBonus 에 반영 → DeriveStats 통해 최종 스탯 상승(기존 클램프 유지).
3. FItemPowerScore 가 affix 가치를 반영(자동 장착이 옵션 좋은 장비 선호).
4. HUD 가 장비 affix 표시. 서버 미러(affix 필드/롤/computeInventoryBonus 확장) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 장비/스탯 회귀 없음(affix 0 = 기존 동일).

## 2. 범위 (In Scope)
### 2.1 affix 데이터/롤 (메인, C++)
- `FItemInstance`(ItemTypes.h): affix 필드 추가 — `BonusCritRate`(float, 0~), `BonusAtkSpeed`(float), `BonusMagicAtk`(float). 기본 0(회귀 안전).
- `FDropFormula::RollAffixes(EItemRarity Rarity, int32 Level, FRandomStream& Rng, FItemInstance& OutItem)` (또는 affix 값 반환): 희귀도별 개수/크기 — 크리율 예 +0.01~0.05, 공속 +0.05~0.15, 마공 +Level×(0.5~1.5). 결정적. ComputeItemBonus/RandomDropFromMonster/GuaranteedDropForLevel 에 통합.
### 2.2 장비 보너스/PowerScore (메인, C++)
- `UInventoryComponent::ComputeEquipmentBonus`: 기존 PhysAtk/PhysDef/Hp 에 더해 `Bonus.CritRate += Item.BonusCritRate × EnhanceMul`(또는 affix 는 강화 미적용 — 결정), `Bonus.AtkSpeed += Item.BonusAtkSpeed`, `Bonus.MagicAtk += Item.BonusMagicAtk` 누적. (강화 배수 적용 여부 명확히: V1 affix 도 ×(1+lvl×0.1) 적용 권장 — 일관.)
- `FItemPowerScore::Compute`: affix 가치 가중(예 CritRate×1000 + AtkSpeed×100 + MagicAtk) 추가해 자동 장착이 affix 반영.
### 2.3 UI (디자이너)
- 장착 요약/드롭 표시에 affix 라벨(예 "크리 +3% / 공속 +0.1 / 마공 +12"). ko/en 라벨 키.
### 2.4 서버 (백엔드)
- `equipment.ts` ItemInstance 에 affix 필드 + computeInventoryBonus 가 crit/atkSpeed/magicAtk 누적(EquipmentBonus 확장) + affix 롤 공식 미러 + parity. (drop.ts 와 일관.)
### 2.5 데이터/밸런스
- 희귀도별 affix 개수/크기 + 빌드 영향(크리/속도/마법) + 문서. 클램프(AtkSpeed/CritRate) 내 합리적 범위.
### 2.6 테스트
- 서버 Vitest(affix 롤/보너스/미러/parity) + 클라 Automation(희귀도별 affix, ComputeEquipmentBonus crit/공속/마공 반영, DeriveStats 최종 상승, PowerScore affix 반영, affix 0 회귀).

## 3. 범위 외
- 접두/접미 네이밍, affix 재롤/잠재, 세트 효과(후속), 신규 affix 종류(흡혈/관통 등).
- 골드/보상 변경(경제 중립).
- 서버 권위 affix 롤(클라 권위 V1).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | FItemInstance affix + RollAffixes + ComputeEquipmentBonus/PowerScore + Automation | ✅ 메인 (`character`) |
| 백엔드 | equipment.ts affix 필드/보너스/롤 미러 + parity | ✅ 보조 (`backend`) |
| 디자이너 | affix HUD 표시 + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 희귀도별 affix 개수/크기 + 빌드 영향 + 문서 | ✅ 보조 (`balance`) |
| QA | 희귀도별 affix/스탯 반영/회귀 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| affix 가 파생 스탯 과인플레 | DeriveStats 기존 클램프(AtkSpeed 0.5~3, CritRate 0~1) + 보수적 magnitude + balance 점검 |
| 강화 배수와 affix 상호작용 모호 | ComputeEquipmentBonus 에서 적용 규칙 명확화(affix 도 ×(1+lvl×0.1)) + 테스트 |
| PowerScore 변경으로 자동 장착 회귀 | affix 0 시 기존 PowerScore 동일, affix 가중 추가만 |
| 서버↔클라 parity(롤/보너스) | DropFormula/equipment DefinitionParity 확장(rng 주입 결정적) |
| 기존 장비/스탯 회귀 | affix 기본 0 → 기존 드롭/장착/스탯 불변 |

## 7. 후속
- affix 네이밍/재롤/잠재, 세트 효과, 신규 affix 종류, 서버 권위 롤.
