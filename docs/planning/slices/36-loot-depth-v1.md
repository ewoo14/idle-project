# PR #36 기획서 — 드롭/아이템 깊이 (심화)

> 현재 드롭(ItemFactory)은 **희귀도가 스탯에 영향을 주지 않는다**(Common 과 Rare 가 같은 레벨이면 BonusAtk 동일, 이름 형용사만 다름). 또 희귀도 확률은 스테이지와 무관하게 고정이고 티어가 Common/Uncommon/Rare 3종뿐이다. #32(스테이지 비례 드롭 레벨)·#33(강화)가 먹일 **장비 다양성**을 위해 희귀도 스탯 스케일·스테이지 연동 확률·상위 티어(Epic/Legendary)를 추가한다. 디자인 토큰(UIThemeTokens)에 Epic/Legendary 색이 이미 준비돼 있다.

## 1. 목표 / DoD
드롭 장비의 희귀도가 높을수록 스탯이 강하고, 스테이지가 높을수록 상위 희귀도가 잘 나오며, Epic/Legendary 티어가 추가된다.

### DoD 검증
1. 희귀도 스탯 배수: 같은 레벨이라도 희귀도↑ → BonusAtk/Def/Hp↑(예 Common 1.0 / Uncommon 1.3 / Rare 1.7 / Epic 2.3 / Legendary 3.2).
2. 스테이지 연동 확률: 몬스터 레벨(=스테이지 비례, #32)↑ → 상위 희귀도 확률↑(저레벨은 Common 위주, 고레벨에서 Rare+ 비중↑).
3. 티어 확장: EItemRarity 에 Epic/Legendary 추가, 전 참조 사이트(HUD 색/이름, factory, PowerScore, 드롭) 정합.
4. 희귀도별 색/이름 HUD 표기(Theme RarityXxx 토큰) + ko/en 이름.
5. 서버 DropFormula 미러(배수/확률) + parity. 서버 Vitest + UE 빌드/Automation GREEN. 기존 인벤토리/장착/강화 회귀 없음.

## 2. 범위 (In Scope)
### 2.1 희귀도 확장 (메인, C++)
- `EItemRarity`(ItemTypes.h): None=0/Common=1/Uncommon=2/Rare=3 + **Epic=4/Legendary=5** 추가. 전 switch/참조(IdleHUD RarityToString, ItemFactory GetRarityAdjective, FItemPowerScore, EquipmentDrop, 테스트) 새 티어 처리.
### 2.2 드롭 공식 (메인, C++)
- `DropFormula.h/.cpp`(ItemSystem/, 순수 static, rng 주입):
  - `GetRarityStatMultiplier(EItemRarity)` → 위 배수.
  - `RollRarityForLevel(int32 Level, FRandomStream& Rng)` → 레벨 비례로 상위 희귀도 확률 상향(저레벨 Common 편중 → 고레벨 Rare/Epic/Legendary 비중↑). 결정적(rng).
  - `ComputeItemBonus(EItemSlot, int32 Level, EItemRarity)` → 기존 슬롯 분배 × 레벨 × 희귀도 배수로 BonusAtk/Def/Hp 산출(기존 ItemFactory 슬롯 분배 보존 + 희귀도 배수 추가).
- `FItemFactory::RandomDropFromMonster` 가 DropFormula 사용(희귀도 스탯 반영 + 레벨 연동 확률). 시그니처/호출부(IdleMonster) 유지.
### 2.3 UI (디자이너)
- 장착 요약/드롭 표시에 희귀도 색(Theme RarityCommon/Uncommon/Rare/Epic/Legendary) + 희귀도 이름. ko/en 희귀도 라벨 키.
### 2.4 서버 (백엔드)
- `server/src/core/data/itemRarity.ts` 또는 `formulas/drop.ts`: getRarityStatMultiplier + rollRarityForLevel(odds, rng 주입) + computeItemBonus 미러 + parity 테스트. 기존 equipment.ts(computeInventoryBonus)와 일관.
### 2.5 데이터/밸런스
- 희귀도 배수/레벨별 확률 테이블 + 강화(#33)·스탯 영향 + 문서.
### 2.6 테스트
- 서버 Vitest(배수/확률 분포/미러/parity) + 클라 Automation(희귀도 배수, 레벨별 확률 경향, 티어 확장, 스탯 산출).

## 3. 범위 외
- 아이템 접두/접미 옵션(affix)·잠재능력·세트 효과(후속), 슬롯별 전용 드롭 풀/유니크 아이템.
- Mythic 티어(토큰만 존재, 후속), 드롭 연출/파티클(외부).
- 서버 권위 드롭 롤(클라 권위 V1, 서버 공식 미러만).

## 4. 작업 분배 — Codex 호출
| 파트 | 작업 | 비중 |
| --- | --- | --- |
| 캐릭터·전투 (메인) | EItemRarity 확장 + DropFormula + ItemFactory 적용 + Automation | ✅ 메인 (`character`) |
| 백엔드 | drop 공식 미러(배수/확률/bonus) + parity | ✅ 보조 (`backend`) |
| 디자이너 | 희귀도 색/이름 HUD + 로컬라이즈 | ✅ 보조 (`designer`) |
| 밸런스 | 희귀도 배수/레벨 확률 테이블 + 문서 | ✅ 보조 (`balance`) |
| QA | 희귀도 스탯/스테이지 확률/티어 시나리오 | ✅ 보조 (`qa`) |

## 5. 워크플로우 v3
[1] 기획+PR → [2] Codex character 메인(+backend/designer/balance/qa) → [3] Claude TM → [4] Codex TM+fix → [5] 검증 → [N] **CI 그린 확정** + 머지. 사용자 PIE 차후([[feedback-autonomous-slices]]). 머지 전 CI 별도 확인([[feedback-ci-before-merge]]). **Codex 커밋/푸시 누락 PM 보완**([[reference-codex-invoke]]).

## 6. 리스크
| 리스크 | 완화 |
| --- | --- |
| EItemRarity 확장 누락 사이트(컴파일/표시 깨짐) | 전 참조 grep 후 switch default 안전 + 신규 티어 처리, 빌드/Automation 검증 |
| 희귀도 배수로 밸런스 폭주 | 보수적 배수 + 레벨 확률 점진 + balance 문서/점검 |
| 레벨별 확률 분포 비결정 테스트 | rng 주입 + 임계/배수 결정적 단위 테스트(분포는 시드 고정) |
| 서버↔클라 공식 parity | DropFormula DefinitionParity 확장 |
| 기존 인벤토리/장착/강화 회귀 | RandomDropFromMonster 시그니처 유지, 슬롯 분배 보존, FItemPowerScore/ComputeEquipmentBonus 기존 로직 유지 |

## 7. 후속
- affix/잠재/세트 효과, 슬롯 전용/유니크 풀, Mythic 티어, 드롭 연출(외부), 서버 권위 드롭.
